#include "mrender/systems/my-system2/my_system2.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace mrender {

MySystem2::MySystem2()
    : RenderSystem("My System 2")
{
}

MySystem2::~MySystem2()
{
}

bool MySystem2::init(RenderContext& context)
{
   
    return true;
}

void MySystem2::render(RenderContext& context)
{
    context.clear();
}

std::vector<std::pair<std::string, std::shared_ptr<FrameBuffer>>> MySystem2::getBuffers(RenderContext& context)
{
    std::vector<std::pair<std::string, std::shared_ptr<FrameBuffer>>> buffers;
    return buffers;
}

}   // namespace mrender