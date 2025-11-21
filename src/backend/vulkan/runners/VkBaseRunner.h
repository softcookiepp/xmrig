/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018-2020 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2020 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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

#ifndef XMRIG_VKBASERUNNER_H
#define XMRIG_VKBASERUNNER_H


#include <string>


#include "tart.hpp"
#include "backend/vulkan/interfaces/IVkRunner.h"
#include "base/crypto/Algorithm.h"
#include "tart.hpp"

namespace xmrig {


class VkLaunchData;


class VkBaseRunner : public IVkRunner
{
public:
    XMRIG_DISABLE_COPY_MOVE_DEFAULT(VkBaseRunner)

    VkBaseRunner(size_t id, const VkLaunchData &data);
    ~VkBaseRunner() override;

protected:
    inline tart::device_ptr ctx() const override                { return m_device; }
    inline const Algorithm &algorithm() const override    { return m_algorithm; }
    inline const char *buildOptions() const override      { return m_options.c_str(); }
    inline const char *deviceKey() const override         { return m_deviceKey.c_str(); }
    inline const char *source() const override            { return m_source; }
    inline const VkLaunchData &data() const override     { return m_data; }
    inline size_t intensity() const override              { return m_intensity; }
    inline size_t threadId() const override               { return m_threadId; }
    inline uint32_t roundSize() const override            { return m_intensity; }
    inline uint32_t processedHashes() const override      { return m_intensity; }
    inline void jobEarlyNotification(const Job&) override {}

    size_t bufferSize() const override;
    uint32_t deviceIndex() const override;
    void build() override;
    void init() override;

protected:

    tart::buffer_ptr createSubBuffer(uint64_t flags, size_t size);

    size_t align(size_t size) const;

	void enqueueReadBuffer(tart::buffer_ptr buffer, bool blocking_read, size_t offset, size_t size, void *ptr);
    void enqueueWriteBuffer(tart::buffer_ptr buffer, bool blocking_write, size_t offset, size_t size, const void *ptr);

    void finalize(uint32_t *hashOutput);

	//tart::device_ptr m_queue = nullptr;
    //tart::device_ptr m_ctx = nullptr;
	tart::device_ptr m_device = nullptr;
	tart::buffer_ptr m_buffer = nullptr;
	tart::buffer_ptr m_input = nullptr;
	tart::buffer_ptr m_output = nullptr;
	tart::cl_program_ptr m_program = nullptr;

    const Algorithm m_algorithm;
    const char *m_source;
    const VkLaunchData &m_data;
    const size_t m_align;
    const size_t m_threadId;
    const uint32_t m_intensity;
    size_t m_offset             = 0;
    std::string m_deviceKey;
    std::string m_options;
};


} /* namespace xmrig */


#endif // XMRIG_VKBASERUNNER_H
