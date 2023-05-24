#pragma once

#include "mrender/techniques/technique.hpp"

namespace Capsaicin
{
    class Testing : public RenderTechnique
    {
    public:
        Testing();
        ~Testing();

        bool init(mrender::RenderContext& context) noexcept override;
        void render(mrender::RenderContext& context) noexcept override;

    protected:
    };
} // namespace Capsaicin
