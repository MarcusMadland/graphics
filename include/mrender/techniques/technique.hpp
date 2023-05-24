#pragma once

#include "mrender/handler/render_context.hpp"
#include "mrender/core/timeable.hpp"

namespace Capsaicin
{

class RenderTechnique : public Timeable
{
    RenderTechnique(RenderTechnique const&) = delete;
    RenderTechnique& operator=(RenderTechnique const&) = delete;

public:
    RenderTechnique(std::string_view const& name) noexcept;
    virtual ~RenderTechnique() noexcept = default;

    
    using Timeable::getName;

    
    virtual bool init(mrender::RenderContext& capsaicin) noexcept = 0;

    
    virtual void render(mrender::RenderContext& capsaicin) noexcept = 0;

protected:
};
} // namespace Capsaicin