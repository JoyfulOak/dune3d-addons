#include "addons/addon_context.hpp"

extern "C" bool dune_register_addon(dune3d::AddonContext &ctx)
{
    if (!ctx.ui)
        return false;

    ctx.ui->add_app_shortcut("app.quit", "<Meta>q");
    ctx.ui->add_app_shortcut("app.quit", "<Primary>q");
    ctx.ui->add_app_shortcut("app.quit", "<Super>q");
    return true;
}
