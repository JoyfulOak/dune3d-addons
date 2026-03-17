#include "addons/addon_context.hpp"

extern "C" bool dune_register_addon(dune3d::AddonContext &ctx)
{
    (void)ctx;
    return true;
}
