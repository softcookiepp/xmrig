/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018-2020 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2020 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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


#include "backend/vulkan/runners/VkBaseRunner.h"
#include "backend/vulkan/cl/VkSource.h"
#include "backend/vulkan/VkCache.h"
#include "backend/vulkan/VkLaunchData.h"
#include "backend/vulkan/runners/tools/VkSharedState.h"
#include "backend/vulkan/wrappers/VkError.h"
#include "backend/vulkan/wrappers/VkLib.h"
#include "base/io/log/Log.h"
#include "base/net/stratum/Job.h"
#include "crypto/common/VirtualMemory.h"


constexpr size_t oneGiB = 1024 * 1024 * 1024;


xmrig::VkBaseRunner::VkBaseRunner(size_t id, const VkLaunchData &data) :
    //m_ctx(data.ctx),
    m_algorithm(data.algorithm),
    m_source(VkSource::get(data.algorithm)),
    m_data(data),
	m_align(data.device.id()->getMetadata().physicalDeviceProperties.limits.minStorageBufferOffsetAlignment),
    m_threadId(id),
    m_intensity(data.thread.intensity())
{
    m_deviceKey = data.device.name();

#   ifdef XMRIG_STRICT_OPENCL_CACHE
    m_deviceKey += ":";
    m_deviceKey += data.platform.version();

    m_deviceKey += ":";
    m_deviceKey += "1.2";//VkLib::getString(data.device.id(), CL_DRIVER_VERSION);
#   endif

#   if defined(__x86_64__) || defined(_M_AMD64) || defined (__arm64__) || defined (__aarch64__)
    m_deviceKey += ":64";
#   endif
}


xmrig::VkBaseRunner::~VkBaseRunner()
{
	// none of this stuff is necessary. thank goodness
    //VkLib::release(m_program);
    //VkLib::release(m_input);
    //VkLib::release(m_output);
    //VkLib::release(m_buffer);
    //VkLib::release(m_queue);
}


size_t xmrig::VkBaseRunner::bufferSize() const
{
    return align(Job::kMaxBlobSize) + align(sizeof(uint32_t) * 0x100);
}


uint32_t xmrig::VkBaseRunner::deviceIndex() const
{
    return data().thread.index();
}


void xmrig::VkBaseRunner::build()
{
    m_program = VkCache::build(this);

    if (m_program == nullptr) {
        throw std::runtime_error("building the program didn't work for some reason :c");
    }
}


void xmrig::VkBaseRunner::init()
{
 #if 1
	// with tart, its just the device c:
	m_queue = data().device.id();
	// and this is the same. will clean this up later, but for now I just need it to work at the bare minimum
	m_ctx = m_queue;
	m_device = m_queue;
 #else
    m_queue = VkLib::createCommandQueue(m_ctx, data().device.id());
#endif
    size_t size         = align(bufferSize());
    const size_t limit  = data().device.freeMemSize();

    if (size < oneGiB && data().device.vendorId() == OCL_VENDOR_AMD && limit >= oneGiB) {
        m_buffer = VkSharedState::get(data().device.index()).createBuffer(m_ctx, size, m_offset, limit);
    }

    if (!m_buffer) {
		m_buffer = m_ctx->allocateBuffer(size);
    }

    m_input  = createSubBuffer(0, Job::kMaxBlobSize);
    m_output = createSubBuffer(0, sizeof(uint32_t) * 0x100);
}


tart::buffer_ptr xmrig::VkBaseRunner::createSubBuffer(uint64_t flags, size_t size)
{
	// we can just use view for this c:
	// although I do wonder if this is more efficient than just letting tart handle it via VMA.
	auto mem = m_buffer->view(m_offset);
    m_offset += align(size);
    return mem;
}


size_t xmrig::VkBaseRunner::align(size_t size) const
{
    return VirtualMemory::align(size, m_align);
}


void xmrig::VkBaseRunner::enqueueReadBuffer(tart::buffer_ptr buffer, bool blocking_read, size_t offset, size_t size, void *ptr)
{
	buffer->copyOut(const_cast<void*>(ptr), (uint64_t)size, (uint64_t)offset);
}


void xmrig::VkBaseRunner::enqueueWriteBuffer(tart::buffer_ptr buffer, bool blocking_write, size_t offset, size_t size, const void *ptr)
{
	buffer->copyIn(const_cast<void*>(ptr), (uint64_t)size, (uint64_t)offset);
}


void xmrig::VkBaseRunner::finalize(uint32_t *hashOutput)
{
    enqueueReadBuffer(m_output, true, 0, sizeof(uint32_t) * 0x100, hashOutput);

    uint32_t &results = hashOutput[0xFF];
    if (results > 0xFF) {
        results = 0xFF;
    }
}
