/* XMRig
 * Copyright (c) 2018-2024 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2024 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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

#ifndef XMRIG_VKDEVICE_H
#define XMRIG_VKDEVICE_H


#include "backend/common/misc/PciTopology.h"
#include "backend/vulkan/wrappers/VkVendor.h"
#include "base/tools/String.h"
#include "tart.hpp"

#include <algorithm>
#include <vector>






namespace xmrig {


class Algorithm;
class VkThreads;


class VkDevice
{
public:
    enum Type {
        Unknown,
        Baffin,
        Ellesmere,
        Polaris,
        Lexa,
        Vega_10,
        Vega_20,
        Raven,
        Navi_10,
        Navi_12,
        Navi_14,
        Navi_21
    };

    VkDevice() = delete;
    VkDevice(uint32_t index, tart::device_ptr id, size_t platform);

    String printableName() const;
    uint32_t clock() const;
    void generate(const Algorithm &algorithm, VkThreads &threads) const;

    inline bool isValid() const                 { return m_id != nullptr; }
#if 1
	inline tart::device_ptr id() const { return m_device; }
#else
    inline tart::device_ptr id() const              { return m_id; }
#endif
    inline const String &platformVendor() const { return m_platformVendor; }
    inline VkVendor platformVendorId() const   { return m_vendorId; }
    inline const PciTopology &topology() const  { return m_topology; }
    inline const String &board() const          { return m_board.isNull() ? m_name : m_board; }
    inline const String &name() const           { return m_name; }
    inline const String &vendor() const         { return m_vendor; }
    inline VkVendor vendorId() const           { return m_vendorId; }
    inline const String &extensions() const     { return m_extensions; }
    inline Type type() const                    { return m_type; }
    inline uint32_t computeUnits() const        { return m_computeUnits; }
    inline size_t freeMemSize() const           { return std::min(maxMemAllocSize(), globalMemSize()); }
    inline size_t globalMemSize() const         { return m_globalMemory; }
    inline size_t maxMemAllocSize() const       { return m_maxMemoryAlloc; }
    inline uint32_t index() const               { return m_index; }

#   ifdef XMRIG_FEATURE_API
    void toJSON(rapidjson::Value &out, rapidjson::Document &doc) const;
#   endif

private:
    static VkDevice::Type getType(const String &name);

    tart::device_ptr m_id               = nullptr;
    size_t m_platform       = 0;
    const String m_platformVendor;
    String m_board;
    const String m_name;
    const String m_vendor;
    String m_extensions;
    const size_t m_maxMemoryAlloc   = 0;
    const size_t m_globalMemory     = 0;
    const uint32_t m_computeUnits   = 1;
    const uint32_t m_index          = 0;
    VkVendor m_platformVendorId    = OCL_VENDOR_UNKNOWN;
    VkVendor m_vendorId            = OCL_VENDOR_UNKNOWN;
    PciTopology m_topology;
    Type m_type                     = Unknown;
    
    // here we goes
    tart::device_ptr m_device;
};


} // namespace xmrig


#endif /* XMRIG_VKDEVICE_H */
