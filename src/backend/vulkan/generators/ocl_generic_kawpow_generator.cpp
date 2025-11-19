/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018      Lee Clagett <https://github.com/vtnerd>
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


#include "backend/vulkan/VkThreads.h"
#include "backend/vulkan/wrappers/VkDevice.h"
#include "base/crypto/Algorithm.h"
#include "crypto/randomx/randomx.h"
#include "crypto/rx/RxAlgo.h"


namespace xmrig {


bool ocl_generic_kawpow_generator(const VkDevice &device, const Algorithm &algorithm, VkThreads &threads)
{
    if (algorithm.family() != Algorithm::KAWPOW) {
        return false;
    }

    bool isNavi = false;

    switch (device.type()) {
    case VkDevice::Navi_10:
    case VkDevice::Navi_12:
    case VkDevice::Navi_14:
    case VkDevice::Navi_21:
        isNavi = true;
        break;

    default:
        break;
    }

    const uint32_t cu_intensity = isNavi ? 524288 : 262144;
    const uint32_t worksize = isNavi ? 128 : 256;
    threads.add(VkThread(device.index(), device.computeUnits() * cu_intensity, worksize, 1));

    return true;
}


} // namespace xmrig
