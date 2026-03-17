#include "addons/addon_context.hpp"
#include "dune3d_appwindow.hpp"
#include <gtkmm/alertdialog.h>

namespace {

void show_status(dune3d::AddonContext &ctx)
{
    if (!ctx.window)
        return;

    auto dialog = Gtk::AlertDialog::create("Constraint Delete Status");
    dialog->set_detail("Constraint-list right-click delete is not implemented by this add-on yet. The converted add-on "
                       "now loads in-process under the new shared-library runtime and can be extended later when a "
                       "documented constraints-box integration point is available.");
    dialog->show(*ctx.window);
}

} // namespace

extern "C" bool dune_register_addon(dune3d::AddonContext &ctx)
{
    if (!ctx.commands)
        return false;

    return ctx.commands->register_command("show-status", [&ctx] { show_status(ctx); }, "Constraint Delete Status", "app");
}
