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

#include <fstream>
#include <map>
#include <mutex>
#include <sstream>


#include "backend/vulkan/VkCache.h"
#include "3rdparty/base32/base32.h"
#include "backend/common/Tags.h"
#include "backend/vulkan/interfaces/IVkRunner.h"
#include "backend/vulkan/VkLaunchData.h"
#include "backend/vulkan/wrappers/VkLib.h"
#include "base/crypto/keccak.h"
#include "base/io/log/Log.h"
#include "base/tools/Chrono.h"


namespace xmrig {


static std::mutex mutex;


static tart::cl_program_ptr createFromSource(const IVkRunner *runner)
{
    LOG_INFO("%s GPU " WHITE_BOLD("#%zu") " " YELLOW_BOLD("compiling..."), vulkan_tag(), runner->data().device.index());
#if 1
	tart::device_ptr device = runner->data().device.id();
#else
	uint ret = 0;
    tart::device_ptr device = runner->data().device.id();
#endif
    const char *source  = runner->source();
    const uint64_t ts   = Chrono::steadyMSecs();
#if 1
	// TODO: figure out if there is a way to just compile SPIR-V
	std::string options = runner->buildOptions();
	tart::shader_module_ptr mod = device->compileCL(source, options);
	tart::cl_program_ptr program = device->createCLProgram(mod);
#else
    tart::cl_program_ptr program = VkLib::createProgramWithSource(runner->ctx(), 1, &source, nullptr, &ret);
    if (ret != CL_SUCCESS) {
        return nullptr;
    }


    if (VkLib::buildProgram(program, 1, &device, runner->buildOptions()) != CL_SUCCESS) {
        printf("BUILD LOG:\n%s\n", VkLib::getProgramBuildLog(program, device).data());

        VkLib::release(program);
        return nullptr;
    }
#endif

    LOG_INFO("%s GPU " WHITE_BOLD("#%zu") " " GREEN_BOLD("compilation completed") BLACK_BOLD(" (%" PRIu64 " ms)"),
             vulkan_tag(), runner->data().device.index(), Chrono::steadyMSecs() - ts);

    return program;
}


static tart::cl_program_ptr createFromBinary(const IVkRunner *runner, const std::string &fileName)
{
#if 1
	// tart just has a function for this lol
	tart::device_ptr device     = runner->data().device.id();
	tart::shader_module_ptr shaderModule = device->loadShaderFromPath(fileName);
	return device->createCLProgram(shaderModule);
#else
	
    std::ifstream file(fileName, std::ofstream::in | std::ofstream::binary);
    if (!file.good()) {
        return nullptr;
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    const std::string s     = ss.str();
    const size_t bin_size   = s.size();
    auto data_ptr           = s.data();
#if 1
    tart::device_ptr device     = runner->data().device.id();
#else
    tart::device_ptr device     = runner->data().device.id();
#endif
    
#if 1
	// no caching for now, sorries :c
	// gotta figure out how SPIR-V will work with this
	return nullptr;
#else
	int32_t clStatus = 0;
    int32_t ret      = 0;
    tart::cl_program_ptr program = VkLib::createProgramWithBinary(runner->ctx(), 1, &device, &bin_size, reinterpret_cast<const unsigned char **>(&data_ptr), &clStatus, &ret);
    if (ret != CL_SUCCESS) {
        return nullptr;
    }

    if (VkLib::buildProgram(program, 1, &device) != CL_SUCCESS) {
        VkLib::release(program);
        return nullptr;
    }

    return program;
#endif
#endif
}


} // namespace xmrig


tart::cl_program_ptr xmrig::VkCache::build(const IVkRunner *runner)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (Nonce::sequence(Nonce::OPENCL) == 0) {
        return nullptr;
    }

    std::string fileName;
    if (runner->data().cache) {
#       ifdef _WIN32
        fileName = prefix() + "\\xmrig\\.cache\\" + cacheKey(runner) + ".bin";
#       else
        fileName = prefix() + "/.cache/" + cacheKey(runner) + ".bin";
#       endif

        tart::cl_program_ptr program = createFromBinary(runner, fileName);
        if (program) {
            return program;
        }
    }

    tart::cl_program_ptr program = createFromSource(runner);
    if (runner->data().cache && program) {
        save(program, fileName);
    }

    return program;
}


std::string xmrig::VkCache::cacheKey(const char *deviceKey, const char *options, const char *source)
{
    std::string in(source);
    in += options;
    in += deviceKey;

    uint8_t hash[200];
    keccak(in.c_str(), in.size(), hash);

    uint8_t result[32] = { 0 };
    base32_encode(hash, 12, result, sizeof(result));

    return reinterpret_cast<char *>(result);
}


std::string xmrig::VkCache::cacheKey(const IVkRunner *runner)
{
    return cacheKey(runner->deviceKey(), runner->buildOptions(), runner->source());
}


void xmrig::VkCache::save(tart::cl_program_ptr program, const std::string &fileName)
{
#if 1
	std::vector<uint32_t> spv = program->getShaderModule()->getSpv();
	size_t nBytes = sizeof(uint32_t)*spv.size();
	
	createDirectory();

    std::ofstream file_stream;
    file_stream.open(fileName, std::ofstream::out | std::ofstream::binary);
    file_stream.write((char*)spv.data(), static_cast<int64_t>(nBytes));
    file_stream.close();
#else
    size_t size = 0;
    if (VkLib::getProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size), &size) != CL_SUCCESS) {
        return;
    }

    std::vector<char> binary(size);

    char *data = binary.data();
    if (VkLib::getProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(char *), &data) != CL_SUCCESS) {
        return;
    }
    createDirectory();

    std::ofstream file_stream;
    file_stream.open(fileName, std::ofstream::out | std::ofstream::binary);
    file_stream.write(binary.data(), static_cast<int64_t>(binary.size()));
    file_stream.close();
#endif
}
