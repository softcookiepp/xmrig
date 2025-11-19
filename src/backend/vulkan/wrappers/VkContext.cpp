/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018-2019 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2019 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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


#include "backend/vulkan/wrappers/VkContext.h"
#include "backend/vulkan/runners/tools/VkSharedState.h"
#include "backend/vulkan/wrappers/VkLib.h"


xmrig::VkContext::VkContext(const VkDevice &device)
{
    std::vector<tart::device_ptr> ids = { device.id() };
    m_ctx = VkLib::createContext(ids);
}


xmrig::VkContext::~VkContext()
{
    if (m_ctx) {
        VkLib::release(m_ctx);
    }
}


bool xmrig::VkContext::init(const std::vector<VkDevice> &devices, std::vector<VkLaunchData> &threads)
{
    if (!m_ctx) {
        std::vector<tart::device_ptr> ids(devices.size());
        for (size_t i = 0; i < devices.size(); ++i) {
            ids[i] = devices[i].id();
        }

        m_ctx = VkLib::createContext(ids);
    }

    if (!m_ctx) {
        return false;
    }

    for (VkLaunchData &data : threads) {
        data.ctx = m_ctx;
    }

    return true;
}
