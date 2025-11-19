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

#ifndef XMRIG_VKCACHE_H
#define XMRIG_VKCACHE_H


#include <string>
#include "tart.hpp"




namespace xmrig {


class IVkRunner;


class VkCache
{
public:
#if 1
    static tart::cl_program_ptr build(const IVkRunner *runner);
#else
    static tart::cl_program_ptr build(const IVkRunner *runner);
#endif
    static std::string cacheKey(const char *deviceKey, const char *options, const char *source);
    static std::string cacheKey(const IVkRunner *runner);

private:
    static std::string prefix();
    static void createDirectory();
    static void save(tart::cl_program_ptr program, const std::string &fileName);
};


} // namespace xmrig


#endif /* XMRIG_VKCACHE_H */
