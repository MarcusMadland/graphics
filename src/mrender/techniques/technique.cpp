#include "mrender/techniques/technique.hpp"

namespace Capsaicin {

RenderTechnique::RenderTechnique(std::string_view const& name) noexcept
    : Timeable(name)
{}

}
