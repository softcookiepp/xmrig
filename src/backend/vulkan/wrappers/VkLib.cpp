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

#include <thread>
#include <stdexcept>
#include <uv.h>


#include "backend/vulkan/wrappers/VkLib.h"
#include "backend/common/Tags.h"
#include "backend/vulkan/wrappers/VkError.h"
#include "base/io/Env.h"
#include "base/io/log/Log.h"


#if defined(OCL_DEBUG_REFERENCE_COUNT)
#   define LOG_REFS(x, ...) xmrig::Log::print(xmrig::Log::WARNING, x, ##__VA_ARGS__)
#endif


static uv_lib_t oclLib;

static const char *kErrorTemplate                    = MAGENTA_BG_BOLD(WHITE_BOLD_S " opencl  ") RED(" error ") RED_BOLD("%s") RED(" when calling ") RED_BOLD("%s");

static const char *kBuildProgram                     = "clBuildProgram";
static const char *kCreateBuffer                     = "clCreateBuffer";
static const char *kCreateCommandQueue               = "clCreateCommandQueue";
static const char *kCreateCommandQueueWithProperties = "clCreateCommandQueueWithProperties";
static const char *kCreateContext                    = "clCreateContext";
static const char *kCreateKernel                     = "clCreateKernel";
static const char *kCreateProgramWithBinary          = "clCreateProgramWithBinary";
static const char *kCreateProgramWithSource          = "clCreateProgramWithSource";
static const char *kCreateSubBuffer                  = "clCreateSubBuffer";
static const char *kEnqueueNDRangeKernel             = "clEnqueueNDRangeKernel";
static const char *kEnqueueReadBuffer                = "clEnqueueReadBuffer";
static const char *kEnqueueWriteBuffer               = "clEnqueueWriteBuffer";
static const char *kFinish                           = "clFinish";
static const char *kGetCommandQueueInfo              = "clGetCommandQueueInfo";
static const char *kGetContextInfo                   = "clGetContextInfo";
static const char *kGetDeviceIDs                     = "clGetDeviceIDs";
static const char *kGetDeviceInfo                    = "clGetDeviceInfo";
static const char *kGetKernelInfo                    = "clGetKernelInfo";
static const char *kGetMemObjectInfo                 = "clGetMemObjectInfo";
static const char *kGetPlatformIDs                   = "clGetPlatformIDs";
static const char *kGetPlatformInfo                  = "clGetPlatformInfo";
static const char *kGetProgramBuildInfo              = "clGetProgramBuildInfo";
static const char *kGetProgramInfo                   = "clGetProgramInfo";
static const char *kReleaseCommandQueue              = "clReleaseCommandQueue";
static const char *kReleaseContext                   = "clReleaseContext";
static const char *kReleaseDevice                    = "clReleaseDevice";
static const char *kReleaseKernel                    = "clReleaseKernel";
static const char *kReleaseMemObject                 = "clReleaseMemObject";
static const char *kReleaseProgram                   = "clReleaseProgram";
static const char *kRetainMemObject                  = "clRetainMemObject";
static const char *kRetainProgram                    = "clRetainProgram";
static const char *kSetKernelArg                     = "clSetKernelArg";
static const char *kSetMemObjectDestructorCallback   = "clSetMemObjectDestructorCallback";
static const char *kSymbolNotFound                   = "symbol not found";
static const char *kUnloadPlatformCompiler           = "clUnloadPlatformCompiler";


#if defined(CL_VERSION_2_0)
typedef tart::device_ptr (CL_API_CALL *createCommandQueueWithProperties_t)(tart::device_ptr, tart::device_ptr, const cl_queue_properties *, int32_t *);
#endif

typedef tart::device_ptr (CL_API_CALL *createCommandQueue_t)(tart::device_ptr, tart::device_ptr, tart::device_ptr_properties, int32_t *);
typedef tart::device_ptr (CL_API_CALL *createContext_t)(const tart::device_ptr_properties *, uint32_t, const tart::device_ptr *, void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *), void *, int32_t *);
typedef int32_t (CL_API_CALL *buildProgram_t)(tart::cl_program_ptr, uint32_t, const tart::device_ptr *, const char *, void (CL_CALLBACK *pfn_notify)(tart::cl_program_ptr, void *), void *);
typedef int32_t (CL_API_CALL *enqueueNDRangeKernel_t)(tart::device_ptr, kernel_pair, uint32_t, const size_t *, const size_t *, const size_t *, uint32_t, const cl_event *, cl_event *);
typedef int32_t (CL_API_CALL *enqueueReadBuffer_t)(tart::device_ptr, tart::buffer_ptr, cl_bool, size_t, size_t, void *, uint32_t, const cl_event *, cl_event *);
typedef int32_t (CL_API_CALL *enqueueWriteBuffer_t)(tart::device_ptr, tart::buffer_ptr, cl_bool, size_t, size_t, const void *, uint32_t, const cl_event *, cl_event *);
typedef int32_t (CL_API_CALL *finish_t)(tart::device_ptr);
typedef int32_t (CL_API_CALL *getCommandQueueInfo_t)(tart::device_ptr, tart::device_ptr_info, size_t, void *, size_t *);
typedef int32_t (CL_API_CALL *getContextInfo_t)(tart::device_ptr, tart::device_ptr_info, size_t, void *, size_t *);
typedef int32_t (CL_API_CALL *getDeviceIDs_t)(size_t, cl_device_type, uint32_t, tart::device_ptr *, uint32_t *);
typedef int32_t (CL_API_CALL *getDeviceInfo_t)(tart::device_ptr, cl_device_info, size_t, void *, size_t *);
typedef int32_t (CL_API_CALL *getKernelInfo_t)(kernel_pair, kernel_pair_info, size_t, void *, size_t *);
typedef int32_t (CL_API_CALL *getMemObjectInfo_t)(tart::buffer_ptr, tart::buffer_ptr_info, size_t, void *, size_t *);
typedef int32_t (CL_API_CALL *getPlatformIDs_t)(uint32_t, size_t *, uint32_t *);
typedef int32_t (CL_API_CALL *getPlatformInfo_t)(size_t, cl_platform_info, size_t, void *, size_t *);
typedef int32_t (CL_API_CALL *getProgramBuildInfo_t)(tart::cl_program_ptr, tart::device_ptr, tart::cl_program_ptr_build_info, size_t, void *, size_t *);
typedef int32_t (CL_API_CALL *getProgramInfo_t)(tart::cl_program_ptr, tart::cl_program_ptr_info, size_t, void *, size_t *);
typedef int32_t (CL_API_CALL *releaseCommandQueue_t)(tart::device_ptr);
typedef int32_t (CL_API_CALL *releaseContext_t)(tart::device_ptr);
typedef int32_t (CL_API_CALL *releaseDevice_t)(tart::device_ptr device);
typedef int32_t (CL_API_CALL *releaseKernel_t)(kernel_pair);
typedef int32_t (CL_API_CALL *releaseMemObject_t)(tart::buffer_ptr);
typedef int32_t (CL_API_CALL *releaseProgram_t)(tart::cl_program_ptr);
typedef int32_t (CL_API_CALL *retainMemObject_t)(tart::buffer_ptr);
typedef int32_t (CL_API_CALL *retainProgram_t)(tart::cl_program_ptr);
typedef int32_t (CL_API_CALL *setKernelArg_t)(kernel_pair, uint32_t, size_t, const void *);
typedef int32_t (CL_API_CALL *setMemObjectDestructorCallback_t)(tart::buffer_ptr, void (CL_CALLBACK *)(tart::buffer_ptr, void *), void *);
typedef int32_t (CL_API_CALL *unloadPlatformCompiler_t)(size_t);
typedef kernel_pair (CL_API_CALL *createKernel_t)(tart::cl_program_ptr, const char *, int32_t *);
typedef tart::buffer_ptr (CL_API_CALL *createBuffer_t)(tart::device_ptr, tart::buffer_ptr_flags, size_t, void *, int32_t *);
typedef tart::buffer_ptr (CL_API_CALL *createSubBuffer_t)(tart::buffer_ptr, tart::buffer_ptr_flags, cl_buffer_create_type, const void *, int32_t *);
typedef tart::cl_program_ptr (CL_API_CALL *createProgramWithBinary_t)(tart::device_ptr, uint32_t, const tart::device_ptr *, const size_t *, const unsigned char **, int32_t *, int32_t *);
typedef tart::cl_program_ptr (CL_API_CALL *createProgramWithSource_t)(tart::device_ptr, uint32_t, const char **, const size_t *, int32_t *);

#if defined(CL_VERSION_2_0)
static createCommandQueueWithProperties_t pCreateCommandQueueWithProperties = nullptr;
#endif

static buildProgram_t  pBuildProgram                                        = nullptr;
static createBuffer_t pCreateBuffer                                         = nullptr;
static createCommandQueue_t pCreateCommandQueue                             = nullptr;
static createContext_t pCreateContext                                       = nullptr;
static createKernel_t pCreateKernel                                         = nullptr;
static createProgramWithBinary_t pCreateProgramWithBinary                   = nullptr;
static createProgramWithSource_t pCreateProgramWithSource                   = nullptr;
static createSubBuffer_t pCreateSubBuffer                                   = nullptr;
static enqueueNDRangeKernel_t pEnqueueNDRangeKernel                         = nullptr;
static enqueueReadBuffer_t pEnqueueReadBuffer                               = nullptr;
static enqueueWriteBuffer_t pEnqueueWriteBuffer                             = nullptr;
static finish_t pFinish                                                     = nullptr;
static getCommandQueueInfo_t pGetCommandQueueInfo                           = nullptr;
static getContextInfo_t pGetContextInfo                                     = nullptr;
static getDeviceIDs_t pGetDeviceIDs                                         = nullptr;
static getDeviceInfo_t pGetDeviceInfo                                       = nullptr;
static getKernelInfo_t pGetKernelInfo                                       = nullptr;
static getMemObjectInfo_t pGetMemObjectInfo                                 = nullptr;
static getPlatformIDs_t pGetPlatformIDs                                     = nullptr;
static getPlatformInfo_t pGetPlatformInfo                                   = nullptr;
static getProgramBuildInfo_t pGetProgramBuildInfo                           = nullptr;
static getProgramInfo_t pGetProgramInfo                                     = nullptr;
static releaseCommandQueue_t pReleaseCommandQueue                           = nullptr;
static releaseContext_t pReleaseContext                                     = nullptr;
static releaseDevice_t pReleaseDevice                                       = nullptr;
static releaseKernel_t pReleaseKernel                                       = nullptr;
static releaseMemObject_t pReleaseMemObject                                 = nullptr;
static releaseProgram_t pReleaseProgram                                     = nullptr;
static retainMemObject_t pRetainMemObject                                   = nullptr;
static retainProgram_t pRetainProgram                                       = nullptr;
static setKernelArg_t pSetKernelArg                                         = nullptr;
static setMemObjectDestructorCallback_t pSetMemObjectDestructorCallback     = nullptr;
static unloadPlatformCompiler_t pUnloadPlatformCompiler                     = nullptr;

#define DLSYM(x) if (uv_dlsym(&oclLib, k##x, reinterpret_cast<void**>(&p##x)) == -1) { throw std::runtime_error(kSymbolNotFound); }


namespace xmrig {
	
tart::Instance gTartInstance;

bool VkLib::m_initialized = false;
bool VkLib::m_ready       = false;
String VkLib::m_loader;


template<typename FUNC, typename OBJ, typename PARAM>
static String getVkString(FUNC fn, OBJ obj, PARAM param)
{
    size_t size = 0;
    if (fn(obj, param, 0, nullptr, &size) != CL_SUCCESS) {
        return String();
    }

    char *buf = new char[size]();
    fn(obj, param, size, buf, nullptr);

    return String(buf);
}


} // namespace xmrig


bool xmrig::VkLib::init(const char *fileName)
{
    if (!m_initialized) {
        m_loader      = fileName == nullptr ? defaultLoader() : Env::expand(fileName);
        m_ready       = uv_dlopen(m_loader, &oclLib) == 0 && load();
        m_initialized = true;
    }

    return m_ready;
}


const char *xmrig::VkLib::lastError()
{
    return uv_dlerror(&oclLib);
}


void xmrig::VkLib::close()
{
    uv_dlclose(&oclLib);
}


bool xmrig::VkLib::load()
{
    try {
        DLSYM(CreateCommandQueue);
        DLSYM(CreateContext);
        DLSYM(BuildProgram);
        DLSYM(EnqueueNDRangeKernel);
        DLSYM(EnqueueReadBuffer);
        DLSYM(EnqueueWriteBuffer);
        DLSYM(Finish);
        DLSYM(GetDeviceIDs);
        DLSYM(GetDeviceInfo);
        DLSYM(GetPlatformInfo);
        DLSYM(GetPlatformIDs);
        DLSYM(GetProgramBuildInfo);
        DLSYM(GetProgramInfo);
        DLSYM(SetKernelArg);
        DLSYM(CreateKernel);
        DLSYM(CreateBuffer);
        DLSYM(CreateProgramWithBinary);
        DLSYM(CreateProgramWithSource);
        DLSYM(ReleaseMemObject);
        DLSYM(ReleaseProgram);
        DLSYM(ReleaseKernel);
        DLSYM(ReleaseCommandQueue);
        DLSYM(ReleaseContext);
        DLSYM(GetKernelInfo);
        DLSYM(GetCommandQueueInfo);
        DLSYM(GetMemObjectInfo);
        DLSYM(GetContextInfo);
        DLSYM(ReleaseDevice);
        DLSYM(UnloadPlatformCompiler);
        DLSYM(SetMemObjectDestructorCallback);
        DLSYM(CreateSubBuffer);
        DLSYM(RetainProgram);
        DLSYM(RetainMemObject);
    } catch (std::exception &ex) {
        return false;
    }

#   if defined(CL_VERSION_2_0)
    uv_dlsym(&oclLib, kCreateCommandQueueWithProperties, reinterpret_cast<void**>(&pCreateCommandQueueWithProperties));
#   endif

    return true;
}


xmrig::String xmrig::VkLib::defaultLoader()
{
#   if defined(__APPLE__)
    return "/System/Library/Frameworks/OpenCL.framework/OpenCL";
#   elif defined(_WIN32)
    return "OpenCL.dll";
#   else
    return "libOpenCL.so";
#   endif
}


tart::device_ptr xmrig::VkLib::createCommandQueue(tart::device_ptr context, tart::device_ptr device, int32_t *errcode_ret) noexcept
{
    tart::device_ptr result = nullptr;

#   if defined(CL_VERSION_2_0)
    if (pCreateCommandQueueWithProperties) {
        const cl_queue_properties commandQueueProperties[] = { 0, 0, 0 };
        result = pCreateCommandQueueWithProperties(context, device, commandQueueProperties, errcode_ret);
    }
    else {
#   endif
        const tart::device_ptr_properties commandQueueProperties = { 0 };
        result = pCreateCommandQueue(context, device, commandQueueProperties, errcode_ret);
#   if defined(CL_VERSION_2_0)
    }
#   endif

    if (*errcode_ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(*errcode_ret), kCreateCommandQueueWithProperties);

        return nullptr;
    }

    return result;
}


tart::device_ptr xmrig::VkLib::createCommandQueue(tart::device_ptr context, tart::device_ptr device)
{
    int32_t ret = 0;
    tart::device_ptr queue = createCommandQueue(context, device, &ret);
    if (ret != CL_SUCCESS) {
        throw std::runtime_error(VkError::toString(ret));
    }

    return queue;
}


tart::device_ptr xmrig::VkLib::createContext(const tart::device_ptr_properties *properties, uint32_t num_devices, const tart::device_ptr *devices, void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *), void *user_data, int32_t *errcode_ret)
{
    assert(pCreateContext != nullptr);

    auto result = pCreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
    if (*errcode_ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(*errcode_ret), kCreateContext);

        return nullptr;
    }

    return result;
}


tart::device_ptr xmrig::VkLib::createContext(const std::vector<tart::device_ptr> &ids)
{
    int32_t ret = 0;
    return createContext(nullptr, static_cast<uint32_t>(ids.size()), ids.data(), nullptr, nullptr, &ret);
}


int32_t xmrig::VkLib::buildProgram(tart::cl_program_ptr program, uint32_t num_devices, const tart::device_ptr *device_list, const char *options, void (CL_CALLBACK *pfn_notify)(tart::cl_program_ptr program, void *user_data), void *user_data) noexcept
{
    assert(pBuildProgram != nullptr);

    const int32_t ret = pBuildProgram(program, num_devices, device_list, options, pfn_notify, user_data);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(ret), kBuildProgram);
    }

    return ret;
}


int32_t xmrig::VkLib::enqueueNDRangeKernel(tart::device_ptr command_queue, kernel_pair kernel, uint32_t work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size, uint32_t num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) noexcept
{
    assert(pEnqueueNDRangeKernel != nullptr);

    return pEnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
}


int32_t xmrig::VkLib::enqueueReadBuffer(tart::device_ptr command_queue, tart::buffer_ptr buffer, cl_bool blocking_read, size_t offset, size_t size, void *ptr, uint32_t num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) noexcept
{
    assert(pEnqueueReadBuffer != nullptr);

    const int32_t ret = pEnqueueReadBuffer(command_queue, buffer, blocking_read, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(ret), kEnqueueReadBuffer);
    }

    return ret;
}


int32_t xmrig::VkLib::enqueueWriteBuffer(tart::device_ptr command_queue, tart::buffer_ptr buffer, cl_bool blocking_write, size_t offset, size_t size, const void *ptr, uint32_t num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) noexcept
{
    assert(pEnqueueWriteBuffer != nullptr);

    const int32_t ret = pEnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(ret), kEnqueueWriteBuffer);
    }

    return ret;
}


int32_t xmrig::VkLib::finish(tart::device_ptr command_queue) noexcept
{
    assert(pFinish != nullptr);

    return pFinish(command_queue);
}


int32_t xmrig::VkLib::getCommandQueueInfo(tart::device_ptr command_queue, tart::device_ptr_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) noexcept
{
    return pGetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, param_value_size_ret);
}


int32_t xmrig::VkLib::getContextInfo(tart::device_ptr context, tart::device_ptr_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) noexcept
{
    return pGetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret);
}


int32_t xmrig::VkLib::getDeviceIDs(size_t platform, cl_device_type device_type, uint32_t num_entries, tart::device_ptr *devices, uint32_t *num_devices) noexcept
{
    assert(pGetDeviceIDs != nullptr);

    return pGetDeviceIDs(platform, device_type, num_entries, devices, num_devices);
}


int32_t xmrig::VkLib::getDeviceInfo(tart::device_ptr device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) noexcept
{
    assert(pGetDeviceInfo != nullptr);

    const int32_t ret = pGetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);
    if (ret != CL_SUCCESS && param_name != CL_DEVICE_BOARD_NAME_AMD) {
        LOG_ERR("Error %s when calling %s, param 0x%04x", VkError::toString(ret), kGetDeviceInfo, param_name);
    }

    return ret;
}


int32_t xmrig::VkLib::getKernelInfo(kernel_pair kernel, kernel_pair_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) noexcept
{
    return pGetKernelInfo(kernel, param_name, param_value_size, param_value, param_value_size_ret);
}


int32_t xmrig::VkLib::getMemObjectInfo(tart::buffer_ptr memobj, tart::buffer_ptr_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) noexcept
{
    return pGetMemObjectInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);
}


int32_t xmrig::VkLib::getPlatformIDs(uint32_t num_entries, size_t *platforms, uint32_t *num_platforms)
{
    assert(pGetPlatformIDs != nullptr);

    return pGetPlatformIDs(num_entries, platforms, num_platforms);
}


int32_t xmrig::VkLib::getPlatformInfo(size_t platform, cl_platform_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) noexcept
{
    assert(pGetPlatformInfo != nullptr);

    return pGetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret);
}


int32_t xmrig::VkLib::getProgramBuildInfo(tart::cl_program_ptr program, tart::device_ptr device, tart::cl_program_ptr_build_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) noexcept
{
    assert(pGetProgramBuildInfo != nullptr);

    const int32_t ret = pGetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(ret), kGetProgramBuildInfo);
    }

    return ret;
}


int32_t xmrig::VkLib::getProgramInfo(tart::cl_program_ptr program, tart::cl_program_ptr_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
    assert(pGetProgramInfo != nullptr);

    const int32_t ret = pGetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(ret), kGetProgramInfo);
    }

    return ret;
}


int32_t xmrig::VkLib::release(tart::device_ptr command_queue) noexcept
{
    assert(pReleaseCommandQueue != nullptr);
    assert(pGetCommandQueueInfo != nullptr);

    if (command_queue == nullptr) {
        return CL_SUCCESS;
    }

#   if defined(OCL_DEBUG_REFERENCE_COUNT)
    LOG_REFS("%p %u ~queue", command_queue, getUint(command_queue, CL_QUEUE_REFERENCE_COUNT));
#   endif

    finish(command_queue);

    int32_t ret = pReleaseCommandQueue(command_queue);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(ret), kReleaseCommandQueue);
    }

    return ret;
}


int32_t xmrig::VkLib::release(tart::device_ptr context) noexcept
{
    assert(pReleaseContext != nullptr);

#   if defined(OCL_DEBUG_REFERENCE_COUNT)
    LOG_REFS("%p %u ~context", context, getUint(context, CL_CONTEXT_REFERENCE_COUNT));
#   endif

    const int32_t ret = pReleaseContext(context);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(ret), kReleaseContext);
    }

    return ret;
}


int32_t xmrig::VkLib::release(tart::device_ptr id) noexcept
{
    assert(pReleaseDevice != nullptr);

#   if defined(OCL_DEBUG_REFERENCE_COUNT)
    LOG_REFS("%p %u ~device", id, getUint(id, CL_DEVICE_REFERENCE_COUNT));
#   endif

    const int32_t ret = pReleaseDevice(id);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(ret), kReleaseDevice);
    }

    return ret;
}


int32_t xmrig::VkLib::release(kernel_pair kernel) noexcept
{
    assert(pReleaseKernel != nullptr);

    if (kernel == nullptr) {
        return CL_SUCCESS;
    }

#   if defined(OCL_DEBUG_REFERENCE_COUNT)
    LOG_REFS("%p %u ~kernel %s", kernel, getUint(kernel, CL_KERNEL_REFERENCE_COUNT), getString(kernel, CL_KERNEL_FUNCTION_NAME).data());
#   endif

    const int32_t ret = pReleaseKernel(kernel);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(ret), kReleaseKernel);
    }

    return ret;
}


int32_t xmrig::VkLib::release(tart::buffer_ptr mem_obj) noexcept
{
    assert(pReleaseMemObject != nullptr);

    if (mem_obj == nullptr) {
        return CL_SUCCESS;
    }

#   if defined(OCL_DEBUG_REFERENCE_COUNT)
    LOG_REFS("%p %u ~mem %zub", mem_obj, getUint(mem_obj, CL_MEM_REFERENCE_COUNT), getUlong(mem_obj, CL_MEM_SIZE));
#   endif

    const int32_t ret = pReleaseMemObject(mem_obj);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(ret), kReleaseMemObject);
    }

    return ret;
}


int32_t xmrig::VkLib::release(tart::cl_program_ptr program) noexcept
{
    assert(pReleaseProgram != nullptr);

    if (program == nullptr) {
        return CL_SUCCESS;
    }

#   if defined(OCL_DEBUG_REFERENCE_COUNT)
    LOG_REFS("%p %u ~program %s", program, getUint(program, CL_PROGRAM_REFERENCE_COUNT), getString(program, CL_PROGRAM_KERNEL_NAMES).data());
#   endif

    const int32_t ret = pReleaseProgram(program);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(ret), kReleaseProgram);
    }

    return ret;
}


int32_t xmrig::VkLib::setKernelArg(kernel_pair kernel, uint32_t arg_index, size_t arg_size, const void *arg_value) noexcept
{
    assert(pSetKernelArg != nullptr);

    return pSetKernelArg(kernel, arg_index, arg_size, arg_value);
}


int32_t xmrig::VkLib::unloadPlatformCompiler(size_t platform) noexcept
{
    return pUnloadPlatformCompiler(platform);
}


kernel_pair xmrig::VkLib::createKernel(tart::cl_program_ptr program, const char *kernel_name, int32_t *errcode_ret) noexcept
{
    assert(pCreateKernel != nullptr);

    auto result = pCreateKernel(program, kernel_name, errcode_ret);
    if (*errcode_ret != CL_SUCCESS) {
        LOG_ERR("%s" RED(" error ") RED_BOLD("%s") RED(" when calling ") RED_BOLD("clCreateKernel") RED(" for kernel ") RED_BOLD("%s"),
                vulkan_tag(), VkError::toString(*errcode_ret), kernel_name);

        return nullptr;
    }

    return result;
}


kernel_pair xmrig::VkLib::createKernel(tart::cl_program_ptr program, const char *kernel_name)
{
    int32_t ret = 0;
    kernel_pair kernel = createKernel(program, kernel_name, &ret);
    if (ret != CL_SUCCESS) {
        throw std::runtime_error(VkError::toString(ret));
    }

    return kernel;
}


tart::buffer_ptr xmrig::VkLib::createBuffer(tart::device_ptr context, tart::buffer_ptr_flags flags, size_t size, void *host_ptr)
{
    int32_t ret = 0;
    tart::buffer_ptr mem = createBuffer(context, flags, size, host_ptr, &ret);
    if (ret != CL_SUCCESS) {
        throw std::runtime_error(VkError::toString(ret));
    }

    return mem;
}


tart::buffer_ptr xmrig::VkLib::createBuffer(tart::device_ptr context, tart::buffer_ptr_flags flags, size_t size, void *host_ptr, int32_t *errcode_ret) noexcept
{
    assert(pCreateBuffer != nullptr);

    auto result = pCreateBuffer(context, flags, size, host_ptr, errcode_ret);
    if (*errcode_ret != CL_SUCCESS) {
        LOG_ERR("%s" RED(" error ") RED_BOLD("%s") RED(" when calling ") RED_BOLD("%s") RED(" with buffer size ") RED_BOLD("%zu"),
                vulkan_tag(), VkError::toString(*errcode_ret), kCreateBuffer, size);

        return nullptr;
    }

    return result;
}


tart::buffer_ptr xmrig::VkLib::createSubBuffer(tart::buffer_ptr buffer, tart::buffer_ptr_flags flags, size_t offset, size_t size, int32_t *errcode_ret) noexcept
{
    const cl_buffer_region region = { offset, size };

    auto result = pCreateSubBuffer(buffer, flags, CL_BUFFER_CREATE_TYPE_REGION, &region, errcode_ret);
    if (*errcode_ret != CL_SUCCESS) {
        LOG_ERR("%s" RED(" error ") RED_BOLD("%s") RED(" when calling ") RED_BOLD("%s") RED(" with offset ") RED_BOLD("%zu") RED(" and size ") RED_BOLD("%zu"),
                vulkan_tag(), VkError::toString(*errcode_ret), kCreateSubBuffer, offset, size);

        return nullptr;
    }

    return result;
}


tart::buffer_ptr xmrig::VkLib::createSubBuffer(tart::buffer_ptr buffer, tart::buffer_ptr_flags flags, size_t offset, size_t size)
{
    int32_t ret = 0;
    tart::buffer_ptr mem = createSubBuffer(buffer, flags, offset, size, &ret);
    if (ret != CL_SUCCESS) {
        throw std::runtime_error(VkError::toString(ret));
    }

    return mem;
}


tart::buffer_ptr xmrig::VkLib::retain(tart::buffer_ptr memobj) noexcept
{
    assert(pRetainMemObject != nullptr);

    if (memobj != nullptr) {
        pRetainMemObject(memobj);
    }

    return memobj;
}


tart::cl_program_ptr xmrig::VkLib::createProgramWithBinary(tart::device_ptr context, uint32_t num_devices, const tart::device_ptr *device_list, const size_t *lengths, const unsigned char **binaries, int32_t *binary_status, int32_t *errcode_ret) noexcept
{
    assert(pCreateProgramWithBinary != nullptr);

    auto result = pCreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);
    if (*errcode_ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(*errcode_ret), kCreateProgramWithBinary);

        return nullptr;
    }

    return result;
}


tart::cl_program_ptr xmrig::VkLib::createProgramWithSource(tart::device_ptr context, uint32_t count, const char **strings, const size_t *lengths, int32_t *errcode_ret) noexcept
{
    assert(pCreateProgramWithSource != nullptr);

    auto result = pCreateProgramWithSource(context, count, strings, lengths, errcode_ret);
    if (*errcode_ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, VkError::toString(*errcode_ret), kCreateProgramWithSource);

        return nullptr;
    }

    return result;
}


tart::cl_program_ptr xmrig::VkLib::retain(tart::cl_program_ptr program) noexcept
{
    assert(pRetainProgram != nullptr);

    if (program != nullptr) {
        pRetainProgram(program);
    }

    return program;
}


uint32_t xmrig::VkLib::getNumPlatforms() noexcept
{
    uint32_t count   = 0;
    int32_t ret      = 0;

    if ((ret = VkLib::getPlatformIDs(0, nullptr, &count)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clGetPlatformIDs for number of platforms.", VkError::toString(ret));
    }

    if (count == 0) {
        LOG_ERR("No OpenCL platform found.");
    }

    return count;
}


uint32_t xmrig::VkLib::getUint(tart::device_ptr command_queue, tart::device_ptr_info param_name, uint32_t defaultValue) noexcept
{
    getCommandQueueInfo(command_queue, param_name, sizeof(uint32_t), &defaultValue);

    return defaultValue;
}


uint32_t xmrig::VkLib::getUint(tart::device_ptr context, tart::device_ptr_info param_name, uint32_t defaultValue) noexcept
{
    getContextInfo(context, param_name, sizeof(uint32_t), &defaultValue);

    return defaultValue;
}


uint32_t xmrig::VkLib::getUint(tart::device_ptr id, cl_device_info param, uint32_t defaultValue) noexcept
{
    getDeviceInfo(id, param, sizeof(uint32_t), &defaultValue);

    return defaultValue;
}


uint32_t xmrig::VkLib::getUint(kernel_pair kernel, kernel_pair_info  param_name, uint32_t defaultValue) noexcept
{
    getKernelInfo(kernel, param_name, sizeof(uint32_t), &defaultValue);

    return defaultValue;
}


uint32_t xmrig::VkLib::getUint(tart::buffer_ptr memobj, tart::buffer_ptr_info param_name, uint32_t defaultValue) noexcept
{
    getMemObjectInfo(memobj, param_name, sizeof(uint32_t), &defaultValue);

    return defaultValue;
}


uint32_t xmrig::VkLib::getUint(tart::cl_program_ptr program, tart::cl_program_ptr_info param, uint32_t defaultValue) noexcept
{
    getProgramInfo(program, param, sizeof(uint32_t), &defaultValue);

    return defaultValue;
}


cl_ulong xmrig::VkLib::getUlong(tart::device_ptr id, cl_device_info param, cl_ulong defaultValue) noexcept
{
    getDeviceInfo(id, param, sizeof(cl_ulong), &defaultValue);

    return defaultValue;
}


cl_ulong xmrig::VkLib::getUlong(tart::buffer_ptr memobj, tart::buffer_ptr_info param_name, cl_ulong defaultValue) noexcept
{
    getMemObjectInfo(memobj, param_name, sizeof(cl_ulong), &defaultValue);

    return defaultValue;
}


std::vector<size_t> xmrig::VkLib::getPlatformIDs() noexcept
{
    const uint32_t count = getNumPlatforms();
    std::vector<size_t> platforms(count);

    if (count) {
        getPlatformIDs(count, platforms.data(), nullptr);
    }

    return platforms;
}


xmrig::String xmrig::VkLib::getProgramBuildLog(tart::cl_program_ptr program, tart::device_ptr device) noexcept
{
    size_t size = 0;
    if (getProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size) != CL_SUCCESS) {
        return String();
    }

    char* log = nullptr;
    try {
        log = new char[size + 1]();
    }
    catch (...) {
        return String();
    }

    if (getProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, size, log, nullptr) != CL_SUCCESS) {
        delete [] log;
        return String();
    }

    return log;
}


xmrig::String xmrig::VkLib::getString(tart::device_ptr id, cl_device_info param) noexcept
{
    return getVkString(VkLib::getDeviceInfo, id, param);
}


xmrig::String xmrig::VkLib::getString(kernel_pair kernel, kernel_pair_info param_name) noexcept
{
    return getVkString(VkLib::getKernelInfo, kernel, param_name);
}


xmrig::String xmrig::VkLib::getString(size_t platform, cl_platform_info param_name) noexcept
{
    return getVkString(VkLib::getPlatformInfo, platform, param_name);
}


xmrig::String xmrig::VkLib::getString(tart::cl_program_ptr program, tart::cl_program_ptr_info param_name) noexcept
{
    return getVkString(VkLib::getProgramInfo, program, param_name);
}
