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

#ifndef XMRIG_VKKERNEL_H
#define XMRIG_VKKERNEL_H

#include "tart.hpp"
#include "base/tools/Object.h"
#include "base/tools/String.h"
#include <string>
#include <utility>







namespace xmrig {


class VkKernel
{
public:
    XMRIG_DISABLE_COPY_MOVE_DEFAULT(VkKernel)

    VkKernel(tart::cl_program_ptr program, const char *name);
    virtual ~VkKernel();

    //inline bool isValid() const         { return m_kernel != nullptr; }
    inline bool isValid() const         { return true; }
    inline std::pair<std::string, tart::cl_program_ptr> kernel() const     { return m_kernel; }
    inline const String name() const   { return m_name.c_str(); }

    void enqueueNDRange(tart::device_ptr queue, uint32_t work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size);
    void setArg(uint32_t index, size_t size, const void *value);

private:
#if 1
	// there is no tart equivalent for a CL kernel. should there be?
	// for now, just store both
	std::string m_name;
	tart::cl_program_ptr m_program;
	std::pair<std::string, tart::cl_program_ptr> m_kernel;
#else
    kernel_pair m_kernel = nullptr;

    const String m_name;
#endif
};


} // namespace xmrig


#endif /* XMRIG_VKKERNEL_H */
