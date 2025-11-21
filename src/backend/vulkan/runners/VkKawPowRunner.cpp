/* XMRig
 * Copyright (c) 2018-2021 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2021 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdexcept>


#include "backend/vulkan/runners/VkKawPowRunner.h"
#include "backend/common/Tags.h"
#include "3rdparty/libethash/ethash_internal.h"
#include "backend/vulkan/kernels/kawpow/KawPow_CalculateDAGKernel.h"
#include "backend/vulkan/VkLaunchData.h"
#include "backend/vulkan/runners/tools/VkKawPow.h"
#include "backend/vulkan/wrappers/VkError.h"
#include "backend/vulkan/wrappers/VkLib.h"
#include "base/io/log/Log.h"
#include "base/io/log/Log.h"
#include "base/io/log/Tags.h"
#include "base/net/stratum/Job.h"
#include "base/tools/Chrono.h"
#include "crypto/common/VirtualMemory.h"
#include "crypto/kawpow/KPHash.h"


namespace xmrig {


constexpr size_t BLOB_SIZE = 40;


VkKawPowRunner::VkKawPowRunner(size_t index, const VkLaunchData &data) : VkBaseRunner(index, data)
{
    switch (data.thread.worksize())
    {
    case 64:
    case 128:
    case 256:
    case 512:
        m_workGroupSize = data.thread.worksize();
        break;
    }

    if (data.device.vendorId() == VkVendor::OCL_VENDOR_NVIDIA) {
        m_options += " -DPLATFORM=OPENCL_PLATFORM_NVIDIA";
        m_dagWorkGroupSize = 32;
    }
}


VkKawPowRunner::~VkKawPowRunner()
{
    m_device->deallocateBuffer(m_lightCache);
    m_device->deallocateBuffer(m_dag);

    //delete m_calculateDagKernel;

    //VkLib::release(m_controlQueue);
    m_device->deallocateBuffer(m_stop);

    VkKawPow::clear();
}


void VkKawPowRunner::run(uint32_t nonce, uint32_t /*nonce_offset*/, uint32_t *hashOutput)
{
    const size_t local_work_size = m_workGroupSize;
    const size_t global_work_offset = nonce;
    const size_t global_work_size = m_intensity - (m_intensity % m_workGroupSize);

    enqueueWriteBuffer(m_input, false, 0, BLOB_SIZE, m_blob);

    const uint32_t zero[2] = {};
    enqueueWriteBuffer(m_output, false, 0, sizeof(uint32_t), zero);
    enqueueWriteBuffer(m_stop, false, 0, sizeof(uint32_t) * 2, zero);

    m_skippedHashes = 0;
	
	if (global_work_offset > 0)
		throw std::runtime_error("global offsets not implemented :c");

	m_searchKernel->enqueue({global_work_size}, {local_work_size});

    uint32_t stop[2] = {};
    enqueueReadBuffer(m_stop, false, 0, sizeof(stop), stop);

    uint32_t output[16] = {};
    enqueueReadBuffer(m_output, true, 0, sizeof(output), output);

    m_skippedHashes = stop[1] * m_workGroupSize;

    if (output[0] > 15) {
        output[0] = 15;
    }

    hashOutput[0xFF] = output[0];
    memcpy(hashOutput, output + 1, output[0] * sizeof(uint32_t));
}


void VkKawPowRunner::set(const Job &job, uint8_t *blob)
{
    m_blockHeight = static_cast<uint32_t>(job.height());
    m_searchKernel = VkKawPow::get(*this, m_blockHeight, m_workGroupSize);

    const uint32_t epoch = m_blockHeight / KPHash::EPOCH_LENGTH;

    const uint64_t dag_size = KPCache::dag_size(epoch);
    if (dag_size > m_dagCapacity) {
        m_device->deallocateBuffer(m_dag); //VkLib::release(m_dag);

        m_dagCapacity = VirtualMemory::align(dag_size, 16 * 1024 * 1024);
        m_dag = m_device->allocateBuffer(m_dagCapacity);//m_dag = VkLib::createBuffer(m_device, 1, m_dagCapacity);
    }

    if (epoch != m_epoch) {
        m_epoch = epoch;

        {
            std::lock_guard<std::mutex> lock(KPCache::s_cacheMutex);

            KPCache::s_cache.init(epoch);

            if (KPCache::s_cache.size() > m_lightCacheCapacity) {
                m_device->deallocateBuffer(m_lightCache);//VkLib::release(m_lightCache);

                m_lightCacheCapacity = VirtualMemory::align(KPCache::s_cache.size());
                m_lightCache = m_device->allocateBuffer(m_lightCacheCapacity);//VkLib::createBuffer(m_device, 1, m_lightCacheCapacity);
            }

            m_lightCacheSize = KPCache::s_cache.size();
            enqueueWriteBuffer(m_lightCache, true, 0, m_lightCacheSize, KPCache::s_cache.data());
        }

        const uint64_t start_ms = Chrono::steadyMSecs();

        const uint32_t dag_words = dag_size / sizeof(node);
        m_calculateDagKernel->setArgs(0, m_lightCache, m_dag, dag_words, m_lightCacheSize / sizeof(node));

        constexpr uint32_t N = 1 << 18;

        for (uint32_t start = 0; start < dag_words; start += N) {
            m_calculateDagKernel->setArg(0, sizeof(start), &start);
            m_calculateDagKernel->enqueue(m_device, N, m_dagWorkGroupSize);
        }

        m_device->sync();//VkLib::finish(m_device);

        LOG_INFO("%s " YELLOW("KawPow") " DAG for epoch " WHITE_BOLD("%u") " calculated " BLACK_BOLD("(%" PRIu64 "ms)"), Tags::vulkan(), epoch, Chrono::steadyMSecs() - start_ms);
    }

    const uint64_t target = job.target();
    const uint32_t hack_false = 0;

	m_searchKernel->setArg(0, m_dag);
	m_searchKernel->setArg(1, m_input);
	m_searchKernel->setArg(2, target);
	m_searchKernel->setArg(3, hack_false);
	m_searchKernel->setArg(4, m_output);
	m_searchKernel->setArg(5, m_stop);

    m_blob = blob;
    enqueueWriteBuffer(m_input, true, 0, BLOB_SIZE, m_blob);
}


void VkKawPowRunner::jobEarlyNotification(const Job&)
{
    const uint32_t one = 1;
    m_stop->copyIn((void*)(&one), sizeof(one), 0);
}


void xmrig::VkKawPowRunner::build()
{
    VkBaseRunner::build();

    m_calculateDagKernel = new KawPow_CalculateDAGKernel(m_program);
}


void xmrig::VkKawPowRunner::init()
{
    VkBaseRunner::init();
    m_device = data().device.id();
	m_stop = m_device->allocateBuffer(sizeof(uint32_t) * 2);
}

} // namespace xmrig
