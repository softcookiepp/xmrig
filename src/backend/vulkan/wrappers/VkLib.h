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

#ifndef XMRIG_VKLIB_H
#define XMRIG_VKLIB_H


#include <vector>


#include "tart.hpp"
#include "base/tools/String.h"

#ifndef CL_DEVICE_TOPOLOGY_AMD
#define CL_DEVICE_TOPOLOGY_AMD 0x4037
#endif
#ifndef CL_DEVICE_BOARD_NAME_AMD
#define CL_DEVICE_BOARD_NAME_AMD 0x4038
#endif
#ifndef CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD
#define CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD 1
#endif
#ifndef CL_DEVICE_PCI_BUS_ID_NV
#define CL_DEVICE_PCI_BUS_ID_NV 0x4008
#endif
#ifndef CL_DEVICE_PCI_SLOT_ID_NV
#define CL_DEVICE_PCI_SLOT_ID_NV 0x4009
#endif


namespace xmrig {

extern tart::Instance gTartInstance;

class VkLib
{
public:
    static bool init(const char *fileName = nullptr);
    static const char *lastError();
    static void close();

    static inline bool isInitialized()   { return m_initialized; }
    static inline const String &loader() { return m_loader; }
#if 1
#else
    static tart::device_ptr createCommandQueue(tart::device_ptr context, tart::device_ptr device, int32_t *errcode_ret) noexcept;
    static tart::device_ptr createCommandQueue(tart::device_ptr context, tart::device_ptr device);
    static tart::device_ptr createContext(const tart::device_ptr_properties *properties, uint32_t num_devices, const tart::device_ptr *devices, void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *), void *user_data, int32_t *errcode_ret);
    static tart::device_ptr createContext(const std::vector<tart::device_ptr> &ids);
    static int32_t buildProgram(tart::cl_program_ptr program, uint32_t num_devices, const tart::device_ptr *device_list, const char *options = nullptr, void (CL_CALLBACK *pfn_notify)(tart::cl_program_ptr program, void *user_data) = nullptr, void *user_data = nullptr) noexcept;
    static int32_t enqueueNDRangeKernel(tart::device_ptr command_queue, tart::cl_kernel_ptr kernel, uint32_t work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size, uint32_t num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) noexcept;
    static int32_t enqueueReadBuffer(tart::device_ptr command_queue, tart::buffer_ptr buffer, bool blocking_read, size_t offset, size_t size, void *ptr, uint32_t num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) noexcept;
    static int32_t enqueueWriteBuffer(tart::device_ptr command_queue, tart::buffer_ptr buffer, bool blocking_write, size_t offset, size_t size, const void *ptr, uint32_t num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) noexcept;
    static int32_t finish(tart::device_ptr command_queue) noexcept;
    static int32_t getCommandQueueInfo(tart::device_ptr command_queue, tart::device_ptr_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret = nullptr) noexcept;
    static int32_t getContextInfo(tart::device_ptr context, tart::device_ptr_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret = nullptr) noexcept;
    static int32_t getDeviceIDs(size_t platform, cl_device_type device_type, uint32_t num_entries, tart::device_ptr *devices, uint32_t *num_devices) noexcept;
    static int32_t getDeviceInfo(tart::device_ptr device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret = nullptr) noexcept;
    static int32_t getKernelInfo(tart::cl_kernel_ptr kernel, tart::cl_kernel_ptr_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret = nullptr) noexcept;
    static int32_t getMemObjectInfo(tart::buffer_ptr memobj, tart::buffer_ptr_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret = nullptr) noexcept;
    static int32_t getPlatformIDs(uint32_t num_entries, size_t *platforms, uint32_t *num_platforms);
    static int32_t getPlatformInfo(size_t platform, cl_platform_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) noexcept;
    static int32_t getProgramBuildInfo(tart::cl_program_ptr program, tart::device_ptr device, tart::cl_program_ptr_build_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) noexcept;
    static int32_t getProgramInfo(tart::cl_program_ptr program, tart::cl_program_ptr_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret = nullptr);
    static int32_t release(tart::device_ptr command_queue) noexcept;
    static int32_t release(tart::device_ptr context) noexcept;
    static int32_t release(tart::device_ptr id) noexcept;
    static int32_t release(tart::cl_kernel_ptr kernel) noexcept;
    static int32_t release(tart::buffer_ptr mem_obj) noexcept;
    static int32_t release(tart::cl_program_ptr program) noexcept;
    static int32_t setKernelArg(tart::cl_kernel_ptr kernel, uint32_t arg_index, size_t arg_size, const void *arg_value) noexcept;
    static int32_t unloadPlatformCompiler(size_t platform) noexcept;
    static tart::cl_kernel_ptr createKernel(tart::cl_program_ptr program, const char *kernel_name, int32_t *errcode_ret) noexcept;
    static tart::cl_kernel_ptr createKernel(tart::cl_program_ptr program, const char *kernel_name);
    static tart::buffer_ptr createBuffer(tart::device_ptr context, uint64_t flags, size_t size, void *host_ptr = nullptr);
    static tart::buffer_ptr createBuffer(tart::device_ptr context, uint64_t flags, size_t size, void *host_ptr, int32_t *errcode_ret) noexcept;
    static tart::buffer_ptr createSubBuffer(tart::buffer_ptr buffer, uint64_t flags, size_t offset, size_t size, int32_t *errcode_ret) noexcept;
    static tart::buffer_ptr createSubBuffer(tart::buffer_ptr buffer, uint64_t flags, size_t offset, size_t size);
    static tart::buffer_ptr retain(tart::buffer_ptr memobj) noexcept;
    static tart::cl_program_ptr createProgramWithBinary(tart::device_ptr context, uint32_t num_devices, const tart::device_ptr *device_list, const size_t *lengths, const unsigned char **binaries, int32_t *binary_status, int32_t *errcode_ret) noexcept;
    static tart::cl_program_ptr createProgramWithSource(tart::device_ptr context, uint32_t count, const char **strings, const size_t *lengths, int32_t *errcode_ret) noexcept;
    static tart::cl_program_ptr retain(tart::cl_program_ptr program) noexcept;
    static uint32_t getNumPlatforms() noexcept;
    static uint32_t getUint(tart::device_ptr command_queue, tart::device_ptr_info param_name, uint32_t defaultValue = 0) noexcept;
    static uint32_t getUint(tart::device_ptr context, tart::device_ptr_info param_name, uint32_t defaultValue = 0) noexcept;
    static uint32_t getUint(tart::device_ptr id, cl_device_info param, uint32_t defaultValue = 0) noexcept;
    static uint32_t getUint(tart::cl_kernel_ptr kernel, tart::cl_kernel_ptr_info  param_name, uint32_t defaultValue = 0) noexcept;
    static uint32_t getUint(tart::buffer_ptr memobj, tart::buffer_ptr_info param_name, uint32_t defaultValue = 0) noexcept;
    static uint32_t getUint(tart::cl_program_ptr program, tart::cl_program_ptr_info param, uint32_t defaultValue = 0) noexcept;
    static cl_ulong getUlong(tart::device_ptr id, cl_device_info param, cl_ulong defaultValue = 0) noexcept;
    static cl_ulong getUlong(tart::buffer_ptr memobj, tart::buffer_ptr_info param_name, cl_ulong defaultValue = 0) noexcept;
    static std::vector<size_t> getPlatformIDs() noexcept;
    static String getProgramBuildLog(tart::cl_program_ptr program, tart::device_ptr device) noexcept;
    static String getString(tart::device_ptr id, cl_device_info param) noexcept;
    static String getString(tart::cl_kernel_ptr kernel, tart::cl_kernel_ptr_info param_name) noexcept;
    static String getString(size_t platform, cl_platform_info param_name) noexcept;
    static String getString(tart::cl_program_ptr program, tart::cl_program_ptr_info param_name) noexcept;
#endif

private:
    static bool load();
    static String defaultLoader();

    static bool m_initialized;
    static bool m_ready;
    static String m_loader;
};


} // namespace xmrig


#endif /* XMRIG_VKLIB_H */
