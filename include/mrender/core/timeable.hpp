/**********************************************************************
Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/
#pragma once

#include "mrender/handler/render_context.hpp"

#include <string_view>
#include <vector>

namespace Capsaicin
{
class TimestampQuery
{
public:
    std::string_view  name; /**< The name of the time stamp */
    //GfxTimestampQuery query;
};

class Timeable
{
public:
    class TimedSection
    {
        TimedSection(TimedSection const &)            = delete;
        TimedSection &operator=(TimedSection const &) = delete;

    public:
        TimedSection(Timeable &parent, std::string_view const &name) noexcept;
        ~TimedSection() noexcept;

    private:
        Timeable      &parent;
        uint32_t const queryIndex;
    };

    /** Defaulted constructor. */
    Timeable() noexcept = default;

    Timeable(std::string_view const &name) noexcept;

    /** Defaulted destructor. */
    ~Timeable() noexcept;

    /**
     * Gets number of timestamp queries.
     * @returns The timestamp query count.
     */
    virtual uint32_t getTimestampQueryCount() const noexcept;

    /**
     * Gets timestamp queries.
     * @returns The timestamp queries.
     */
    virtual TimestampQuery const *getTimestampQueries() const noexcept;

    /** Resets the timed section queries */
    virtual void resetQueries() noexcept;

    /**
     * Sets internal graphics context
     * @param gfx The gfx context.
     */
    virtual void setGfxContext(mrender::RenderContext const &gfx) noexcept;

    /**
     * Gets the name of the timeable.
     * @returns The name string.
     */
    std::string_view getName() const noexcept;

protected:
    std::vector<TimestampQuery> queries;        /**< The array of timestamp queries. */
    uint32_t                    queryCount = 0; /**< The number of timestamp queries. */
    mrender::RenderContext      gfx_;           /**< The rendering context to be used. */
    std::string_view            name_;          /**< The name of the render technique. */
};
} // namespace Capsaicin
