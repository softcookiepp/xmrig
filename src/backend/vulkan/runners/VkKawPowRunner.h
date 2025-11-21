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

#ifndef XMRIG_VKKAWPOWRUNNER_H
#define XMRIG_VKKAWPOWRUNNER_H

#include "backend/vulkan/wrappers/VkLib.h"
#include "backend/vulkan/runners/VkBaseRunner.h"
#include "crypto/kawpow/KPCache.h"

#include <mutex>

namespace xmrig {


class KawPow_CalculateDAGKernel;


class VkKawPowRunner : public VkBaseRunner
{
public:
    XMRIG_DISABLE_COPY_MOVE_DEFAULT(VkKawPowRunner)

    VkKawPowRunner(size_t index, const VkLaunchData &data);
    ~VkKawPowRunner() override;

protected:
    void run(uint32_t nonce, uint32_t nonce_offset, uint32_t *hashOutput) override;
    void set(const Job &job, uint8_t *blob) override;
    void build() override;
    void init() override;
    void jobEarlyNotification(const Job& job) override;
    uint32_t processedHashes() const override { return m_intensity - m_skippedHashes; }

private:
    uint8_t* m_blob = nullptr;
    uint32_t m_skippedHashes = 0;

    uint32_t m_blockHeight = 0;
    uint32_t m_epoch = 0xFFFFFFFFUL;

    tart::buffer_ptr m_lightCache = nullptr;
    size_t m_lightCacheSize = 0;
    size_t m_lightCacheCapacity = 0;

    tart::buffer_ptr m_dag = nullptr;
    size_t m_dagCapacity = 0;

    KawPow_CalculateDAGKernel* m_calculateDagKernel = nullptr;

    tart::cl_kernel_ptr m_searchKernel;// = nullptr;

    size_t m_workGroupSize = 256;
    size_t m_dagWorkGroupSize = 64;

#if 1
	tart::device_ptr m_device = nullptr;
#else
    tart::device_ptr m_controlQueue = nullptr;
#endif
    tart::buffer_ptr m_stop = nullptr;
};


} /* namespace xmrig */


#endif // XMRIG_VKKAWPOWRUNNER_H
