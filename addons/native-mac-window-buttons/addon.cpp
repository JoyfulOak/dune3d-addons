#include "addons/addon_context.hpp"

extern "C" bool dune_register_addon(dune3d::AddonContext &ctx)
{
    if (!ctx.ui)
        return false;

    ctx.ui->set_window_controls(true, "close,minimize,maximize:");
    return true;
}
