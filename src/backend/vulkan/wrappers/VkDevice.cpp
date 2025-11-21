/* XMRig
 * Copyright (c) 2021      Spudz76     <https://github.com/Spudz76>
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

#include "backend/vulkan/wrappers/VkDevice.h"
#include "3rdparty/fmt/core.h"
#include "3rdparty/rapidjson/document.h"
#include "backend/vulkan/VkGenerator.h"
#include "backend/vulkan/VkThreads.h"
#include "backend/vulkan/wrappers/VkLib.h"
#include "base/io/log/Log.h"


#ifdef XMRIG_FEATURE_ADL
#   include "backend/vulkan/wrappers/AdlLib.h"
#endif


#include <algorithm>
#include <map>


namespace xmrig {


struct topology_amd {
    uint32_t type;
    int8_t unused[17];
    int8_t bus;
    int8_t device;
    int8_t function;
};


#ifdef XMRIG_ALGO_RANDOMX
extern bool ocl_generic_rx_generator(const VkDevice &device, const Algorithm &algorithm, VkThreads &threads);
#endif

#ifdef XMRIG_ALGO_KAWPOW
extern bool ocl_generic_kawpow_generator(const VkDevice& device, const Algorithm& algorithm, VkThreads& threads);
#endif

extern bool ocl_vega_cn_generator(const VkDevice &device, const Algorithm &algorithm, VkThreads &threads);
extern bool ocl_generic_cn_generator(const VkDevice &device, const Algorithm &algorithm, VkThreads &threads);


static ocl_gen_config_fun generators[] = {
#   ifdef XMRIG_ALGO_RANDOMX
    ocl_generic_rx_generator,
#   endif
#   ifdef XMRIG_ALGO_KAWPOW
    ocl_generic_kawpow_generator,
#   endif
    ocl_vega_cn_generator,
    ocl_generic_cn_generator
};


static VkVendor getPlatformVendorId(const String &vendor, const String &extensions)
{
    if (extensions.contains("cl_amd_") || vendor.contains("Advanced Micro Devices") || vendor.contains("AMD")) {
        return OCL_VENDOR_AMD;
    }

    if (extensions.contains("cl_nv_") || vendor.contains("NVIDIA")) {
        return OCL_VENDOR_NVIDIA;
    }

    if (extensions.contains("int32_tel_") || vendor.contains("Intel")) {
        return OCL_VENDOR_INTEL;
    }

#   ifdef XMRIG_OS_APPLE
    if (extensions.contains("cl_APPLE_") || vendor.contains("Apple")) {
        return OCL_VENDOR_APPLE;
    }
#   endif

    return OCL_VENDOR_UNKNOWN;
}


static VkVendor getVendorId(const String &vendor)
{
    if (vendor.contains("Advanced Micro Devices") || vendor.contains("AMD")) {
        return OCL_VENDOR_AMD;
    }

    if (vendor.contains("NVIDIA")) {
        return OCL_VENDOR_NVIDIA;
    }

    if (vendor.contains("Intel")) {
        return OCL_VENDOR_INTEL;
    }

#   ifdef XMRIG_OS_APPLE
    if (vendor.contains("Apple")) {
        return OCL_VENDOR_APPLE;
    }
#   endif

    return OCL_VENDOR_UNKNOWN;
}


} // namespace xmrig


xmrig::VkDevice::VkDevice(uint32_t index, tart::device_ptr id, size_t platform) :
    m_id(id),
    m_platform(platform),
    m_platformVendor("VULKAN"), //m_platformVendor(VkLib::getString(platform, CL_PLATFORM_VENDOR)),
    m_name("generic vulkan device"),//m_name(VkLib::getString(id, CL_DEVICE_NAME)),
    m_vendor("VULKAN"), //m_vendor(VkLib::getString(id, CL_DEVICE_VENDOR)),
    m_extensions(""),//m_extensions(VkLib::getString(id, CL_DEVICE_EXTENSIONS)),
    m_maxMemoryAlloc(id->getMetadata().maxMemoryAllocationSize),//m_maxMemoryAlloc(VkLib::getUlong(id, CL_DEVICE_MAX_MEM_ALLOC_SIZE)),//m_maxMemoryAlloc(VkLib::getUlong(id, CL_DEVICE_MAX_MEM_ALLOC_SIZE)),
    m_globalMemory(0),//m_globalMemory(VkLib::getUlong(id, CL_DEVICE_GLOBAL_MEM_SIZE)),
    m_computeUnits(1),//m_computeUnits(VkLib::getUint(id, CL_DEVICE_MAX_COMPUTE_UNITS, 1)),
    m_index(index)
{
	m_device = gTartInstance.createDevice(index);
	
    m_vendorId  = getVendorId(m_vendor);
    m_platformVendorId = getPlatformVendorId(m_platformVendor, m_extensions);
    m_type      = getType(m_name);
#if 1
#else
    if (m_extensions.contains("cl_amd_device_attribute_query")) {
        topology_amd topology{};
        if (VkLib::getDeviceInfo(id, CL_DEVICE_TOPOLOGY_AMD, sizeof(topology), &topology) == CL_SUCCESS && topology.type == CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD) {
            m_topology = { topology.bus, topology.device, topology.function };
        }

        m_board = VkLib::getString(id, CL_DEVICE_BOARD_NAME_AMD);
    }
    else if (m_extensions.contains("cl_nv_device_attribute_query")) {
        uint32_t bus = 0;
        if (VkLib::getDeviceInfo(id, CL_DEVICE_PCI_BUS_ID_NV, sizeof(bus), &bus) == CL_SUCCESS) {
            uint32_t slot  = VkLib::getUint(id, CL_DEVICE_PCI_SLOT_ID_NV);
            m_topology = { bus, (slot >> 3) & 0xff, slot & 7 };
        }
    }
#endif
}


xmrig::String xmrig::VkDevice::printableName() const
{
    if (m_board.isNull()) {
        return fmt::format(GREEN_BOLD("{}"), m_name).c_str();
    }

    return fmt::format(GREEN_BOLD("{}") " (" CYAN_BOLD("{}") ")", m_board, m_name).c_str();
}


uint32_t xmrig::VkDevice::clock() const
{
    return 9001;//return VkLib::getUint(id(), CL_DEVICE_MAX_CLOCK_FREQUENCY);
}


void xmrig::VkDevice::generate(const Algorithm &algorithm, VkThreads &threads) const
{
    for (auto fn : generators) {
        if (fn(*this, algorithm, threads)) {
            return;
        }
    }
}


#ifdef XMRIG_FEATURE_API
void xmrig::VkDevice::toJSON(rapidjson::Value &out, rapidjson::Document &doc) const
{
    using namespace rapidjson;
    auto &allocator = doc.GetAllocator();

    out.AddMember("board",       board().toJSON(doc), allocator);
    out.AddMember("name",        name().toJSON(doc), allocator);
    out.AddMember("bus_id",      topology().toString().toJSON(doc), allocator);
    out.AddMember("cu",          computeUnits(), allocator);
    out.AddMember("global_mem",  static_cast<uint64_t>(globalMemSize()), allocator);

#   ifdef XMRIG_FEATURE_ADL
    if (AdlLib::isReady()) {
        auto data = AdlLib::health(*this);

        Value health(kObjectType);
        health.AddMember("temperature", data.temperature, allocator);
        health.AddMember("power",       data.power, allocator);
        health.AddMember("clock",       data.clock, allocator);
        health.AddMember("mem_clock",   data.memClock, allocator);
        health.AddMember("rpm",         data.rpm, allocator);

        out.AddMember("health", health, allocator);
    }
#   endif
}
#endif


#ifndef XMRIG_OS_APPLE
xmrig::VkDevice::Type xmrig::VkDevice::getType(const String &name)
{
    static std::map<const char *, VkDevice::Type> types = {
        { "gfx900",     Vega_10 },
        { "gfx901",     Vega_10 },
        { "gfx902",     Raven },
        { "gfx903",     Raven },
        { "gfx906",     Vega_20 },
        { "gfx907",     Vega_20 },
        { "gfx1010",    Navi_10 },
        { "gfx1011",    Navi_12 },
        { "gfx1012",    Navi_14 },
        { "gfx1030",    Navi_21 },
        { "gfx804",     Lexa },
        { "Baffin",     Baffin },
        { "Ellesmere",  Ellesmere },
        { "gfx803",     Polaris },
        { "polaris",    Polaris },
    };

    for (auto &kv : types) {
        if (name.contains(kv.first)) {
            return kv.second;
        }
    }

    return VkDevice::Unknown;
}
#endif
