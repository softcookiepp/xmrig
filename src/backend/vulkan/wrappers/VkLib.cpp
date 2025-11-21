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

namespace xmrig {
	
tart::Instance gTartInstance;

bool VkLib::m_initialized = false;
bool VkLib::m_ready       = false;
String VkLib::m_loader;

} // namespace xmrig


bool xmrig::VkLib::init(const char *fileName)
{
    return true;
}


const char *xmrig::VkLib::lastError()
{
    return uv_dlerror(&oclLib);
}


void xmrig::VkLib::close()
{
    uv_dlclose(&oclLib);
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
