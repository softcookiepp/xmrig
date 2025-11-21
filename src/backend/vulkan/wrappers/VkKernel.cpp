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


#include "backend/common/Tags.h"
#include "backend/vulkan/wrappers/VkError.h"
#include "backend/vulkan/wrappers/VkKernel.h"
#include "backend/vulkan/wrappers/VkLib.h"
#include "base/io/log/Log.h"


#include <stdexcept>


xmrig::VkKernel::VkKernel(tart::cl_program_ptr program, const char *name) :
    m_name(name),
    m_program(program)
{
    m_kernel = m_program->getKernel(name);
}


xmrig::VkKernel::~VkKernel()
{
    //VkLib::release(m_kernel);
}


void xmrig::VkKernel::enqueueNDRange(tart::device_ptr queue, uint32_t work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size)
{
#if 1
	if (global_work_offset) throw std::runtime_error("global_work_offset not implemented!!!");
	
	std::vector<uint32_t> localSize(work_dim);
	std::vector<uint32_t> globalSize(work_dim);
	for (size_t i = 0; i < work_dim; i += 1)
	{
		localSize[i] = local_work_size[i];
		if (global_work_size[i] % local_work_size[i] > 0)
			throw std::runtime_error("can't have indivisible global sizes, sillybum");
		
		globalSize[i] = global_work_size[i] / localSize[i];
	}
	
	throw std::runtime_error("argument setting has not been configured yet!");
	//m_program->dispatch(m_kernel.first, globalSize, localSize, std::vector<buffer_ptr> buffers, std::vector<uint8_t> pushConstants = {});
#else

#endif
}

#if 0
void xmrig::VkKernel::setArg(uint32_t index, size_t size, const void *value)
{
    const int32_t ret = VkLib::setKernelArg(m_kernel, index, size, value);
    if (ret != CL_SUCCESS) {
        LOG_ERR("%s" RED(" error ") RED_BOLD("%s") RED(" when calling ") RED_BOLD("clSetKernelArg") RED(" for kernel ") RED_BOLD("%s")
                RED(" argument ") RED_BOLD("%u") RED(" size ") RED_BOLD("%zu"),
                vulkan_tag(), VkError::toString(ret), name().data(), index, size);

        throw std::runtime_error(VkError::toString(ret));
    }
}
#endif
