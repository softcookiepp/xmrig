/* XMRig
 * Copyright (c) 2018-2020 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2020 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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

#ifndef XMRIG_VKSHAREDDATA_H
#define XMRIG_VKSHAREDDATA_H


#include <memory>
#include <mutex>
#include "tart.hpp"





namespace xmrig {


class Job;


class VkSharedData
{
public:
    VkSharedData() = default;

    tart::buffer_ptr createBuffer(tart::device_ptr context, size_t size, size_t &offset, size_t limit);
    uint64_t adjustDelay(size_t id);
    uint64_t resumeDelay(size_t id);
    void release();
    void setResumeCounter(uint32_t value);
    void setRunTime(uint64_t time);

    inline size_t threads() const       { return m_threads; }

    inline VkSharedData &operator++()  { ++m_threads; return *this; }

#   ifdef XMRIG_ALGO_RANDOMX
    tart::buffer_ptr dataset() const;
    void createDataset(tart::device_ptr ctx, const Job &job, bool host);
#   endif

private:
    tart::buffer_ptr m_buffer           = nullptr;
    double m_averageRunTime   = 0.0;
    double m_threshold        = 0.95;
    size_t m_offset           = 0;
    size_t m_threads          = 0;
    std::mutex m_mutex;
    uint32_t m_resumeCounter  = 0;
    uint64_t m_timestamp      = 0;

#   ifdef XMRIG_ALGO_RANDOMX
    tart::buffer_ptr m_dataset          = nullptr;
#   endif
};


} /* namespace xmrig */


#endif /* XMRIG_VKSHAREDDATA_H */
