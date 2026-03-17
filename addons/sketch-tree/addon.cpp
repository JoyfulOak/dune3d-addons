#include "addons/addon_context.hpp"
#include "canvas/canvas.hpp"
#include "core/core.hpp"
#include "document/constraint/constraint.hpp"
#include "document/constraint/constraint_angle.hpp"
#include "document/constraint/constraint_diameter_radius.hpp"
#include "document/constraint/constraint_hv.hpp"
#include "document/constraint/constraint_point_distance.hpp"
#include "document/document.hpp"
#include "document/entity/entity_arc2d.hpp"
#include "document/entity/entity_circle2d.hpp"
#include "document/entity/entity_line2d.hpp"
#include "document/group/group_sketch.hpp"
#include "dune3d_appwindow.hpp"
#include "editor/editor.hpp"
#include "util/gtk_util.hpp"
#include <gtkmm/alertdialog.h>
#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/eventcontrollerfocus.h>
#include <gtkmm/expander.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/searchentry.h>
#include <gtkmm/separator.h>
#include <gtkmm/spinbutton.h>
#include <algorithm>
#include <cctype>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

using namespace dune3d;

struct SketchRowInfo {
    UUID constraint_uuid;
    UUID owner_entity_uuid;
    std::string label;
    bool editable = false;
    bool toggleable = false;
    DatumUnit unit = DatumUnit::MM;
    double min_value = 0;
    double max_value = 0;
    double value = 0;
};

static std::string to_lower(std::string text)
{
    std::ranges::transform(text, text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

static std::string entity_type_key(Entity::Type type)
{
    switch (type) {
    case Entity::Type::LINE_2D:
        return "Line";
    case Entity::Type::CIRCLE_2D:
        return "Circle";
    case Entity::Type::ARC_2D:
        return "Arc";
    case Entity::Type::POINT_2D:
        return "Point";
    case Entity::Type::BEZIER_2D:
        return "Bezier";
    default:
        return Entity::get_type_name(type, Entity::TypeNameStyle::WITHOUT_WORKPLANE);
    }
}

static std::string format_number(double value, int digits)
{
    std::ostringstream ss;
    ss.setf(std::ios::fixed);
    ss.precision(digits);
    ss << value;
    auto text = ss.str();
    while (text.find('.') != std::string::npos && !text.empty() && text.back() == '0')
        text.pop_back();
    if (!text.empty() && text.back() == '.')
        text.pop_back();
    return text;
}

static std::string unit_suffix(DatumUnit unit)
{
    switch (unit) {
    case DatumUnit::MM:
        return "mm";
    case DatumUnit::DEGREE:
        return "deg";
    case DatumUnit::INTEGER:
        return "";
    }
    return "";
}

static bool references_entity(const Constraint &constraint, const UUID &entity_uuid)
{
    const auto refs = constraint.get_referenced_entities();
    return refs.contains(entity_uuid);
}

static bool is_line_length_constraint(const Constraint &constraint, const UUID &entity_uuid)
{
    const auto *distance = dynamic_cast<const ConstraintPointDistanceBase *>(&constraint);
    if (!distance)
        return false;

    const auto direct = distance->m_entity1.entity == entity_uuid && distance->m_entity1.point == 1
                        && distance->m_entity2.entity == entity_uuid && distance->m_entity2.point == 2;
    const auto flipped = distance->m_entity1.entity == entity_uuid && distance->m_entity1.point == 2
                         && distance->m_entity2.entity == entity_uuid && distance->m_entity2.point == 1;
    return direct || flipped;
}

static std::string related_entities_suffix(const Document &doc, const Constraint &constraint, const UUID &owner_entity_uuid,
                                           const std::map<UUID, std::string> &entity_names)
{
    std::vector<std::string> names;
    for (const auto &entity_uuid : constraint.get_referenced_entities()) {
        if (entity_uuid == owner_entity_uuid)
            continue;
        if (const auto it = entity_names.find(entity_uuid); it != entity_names.end())
            names.push_back(it->second);
        else if (const auto *entity = doc.get_entity_ptr(entity_uuid))
            names.push_back(entity->get_type_name(Entity::TypeNameStyle::WITHOUT_WORKPLANE));
    }
    std::ranges::sort(names);
    names.erase(std::unique(names.begin(), names.end()), names.end());
    if (names.empty())
        return {};
    return " with " + names.front();
}

static std::optional<SketchRowInfo> make_row_info(const Document &doc, const Constraint &constraint,
                                                  const UUID &owner_entity_uuid,
                                                  const std::map<UUID, std::string> &entity_names)
{
    SketchRowInfo row{
            .constraint_uuid = constraint.m_uuid,
            .owner_entity_uuid = owner_entity_uuid,
    };

    if (const auto *datum = dynamic_cast<const IConstraintDatum *>(&constraint)) {
        row.editable = true;
        row.min_value = datum->get_datum_range().first;
        row.max_value = datum->get_datum_range().second;
        row.unit = datum->get_datum_unit();
        row.value = datum->get_display_datum(doc);

        if (is_line_length_constraint(constraint, owner_entity_uuid))
            row.label = "Length";
        else
            row.label = datum->get_datum_name();

        return row;
    }

    row.toggleable = true;
    row.label = constraint.get_type_name() + related_entities_suffix(doc, constraint, owner_entity_uuid, entity_names);
    return row;
}

class SketchTreePanel : public Gtk::Box {
public:
    explicit SketchTreePanel(AddonContext &ctx) : Gtk::Box(Gtk::Orientation::VERTICAL), m_ctx(ctx)
    {
        set_spacing(6);
        set_margin(6);

        m_filter_entry = Gtk::make_managed<Gtk::SearchEntry>();
        m_filter_entry->set_placeholder_text("Filter sketch items");
        append(*m_filter_entry);

        m_scroller = Gtk::make_managed<Gtk::ScrolledWindow>();
        m_scroller->set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        m_scroller->set_vexpand(true);
        append(*m_scroller);

        m_content_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        m_content_box->set_spacing(6);
        m_scroller->set_child(*m_content_box);

        m_status_label = Gtk::make_managed<Gtk::Label>();
        m_status_label->set_xalign(0);
        m_status_label->set_wrap(true);
        append(*m_status_label);

        m_filter_entry->signal_search_changed().connect([this] { reload(); });

        if (m_ctx.core)
            m_rebuilt_connection = m_ctx.core->signal_rebuilt().connect([this] { reload(); });
        if (m_ctx.editor)
            m_group_connection = m_ctx.editor->signal_current_group_changed().connect([this] { reload(); });
        if (m_ctx.window)
            m_selection_connection = m_ctx.window->get_canvas().signal_selection_changed().connect([this] {
                sync_from_selection();
            });

        reload();
    }

    ~SketchTreePanel() override
    {
        m_rebuilt_connection.disconnect();
        m_group_connection.disconnect();
        m_selection_connection.disconnect();
    }

private:
    AddonContext &m_ctx;
    Gtk::SearchEntry *m_filter_entry = nullptr;
    Gtk::ScrolledWindow *m_scroller = nullptr;
    Gtk::Box *m_content_box = nullptr;
    Gtk::Label *m_status_label = nullptr;
    sigc::connection m_rebuilt_connection;
    sigc::connection m_group_connection;
    sigc::connection m_selection_connection;
    std::map<UUID, Gtk::Expander *> m_entity_expanders;
    std::map<UUID, Gtk::Label *> m_entity_labels;
    std::map<UUID, Gtk::Label *> m_constraint_labels;
    UUID m_current_group;
    bool m_reloading = false;

    void reload()
    {
        m_reloading = true;
        clear_rows();
        m_entity_expanders.clear();
        m_entity_labels.clear();
        m_constraint_labels.clear();
        set_status({});

        if (!m_ctx.core || !m_ctx.window || !m_ctx.document) {
            append_message("Sketch Tree is unavailable.");
            m_reloading = false;
            return;
        }

        if (!m_ctx.core->has_documents()) {
            append_message("No document open.");
            m_reloading = false;
            return;
        }

        auto &doc = m_ctx.core->get_current_document();
        const auto group_uuid = m_ctx.core->get_current_group();
        m_current_group = group_uuid;
        if (!doc.get_groups().contains(group_uuid)) {
            append_message("Select a sketch group to inspect its tree.");
            m_reloading = false;
            return;
        }
        const auto &group = doc.get_group(group_uuid);
        if (group.get_type() != Group::Type::SKETCH) {
            append_message("Select a sketch group to inspect its tree.");
            m_reloading = false;
            return;
        }

        auto sketch_expander = Gtk::make_managed<Gtk::Expander>();
        sketch_expander->set_label(group.m_name.empty() ? "Sketch" : group.m_name);
        sketch_expander->set_expanded(true);
        m_content_box->append(*sketch_expander);

        auto sketch_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        sketch_box->set_spacing(4);
        sketch_box->set_margin_start(12);
        sketch_expander->set_child(*sketch_box);

        auto entities = collect_entities(doc, group_uuid);
        auto entity_names = make_entity_names(doc, entities);
        const auto filter = to_lower(m_filter_entry->get_text());

        for (const auto *entity : entities) {
            auto rows = collect_constraint_rows(doc, *entity, entity_names);
            if (!passes_filter(entity_names.at(entity->m_uuid), rows, filter))
                continue;

            auto entity_expander = Gtk::make_managed<Gtk::Expander>();
            entity_expander->set_expanded(false);
            entity_expander->set_margin_start(6);
            sketch_box->append(*entity_expander);
            m_entity_expanders.emplace(entity->m_uuid, entity_expander);

            auto label = Gtk::make_managed<Gtk::Label>();
            label->set_xalign(0);
            label->set_use_markup(true);
            label->set_text(entity_names.at(entity->m_uuid));
            entity_expander->set_label_widget(*label);
            m_entity_labels.emplace(entity->m_uuid, label);

            auto entity_click = Gtk::GestureClick::create();
            entity_click->set_button(GDK_BUTTON_PRIMARY);
            entity_click->signal_pressed().connect([this, entity_uuid = entity->m_uuid](int, double, double) {
                select_entity(entity_uuid);
            });
            entity_expander->add_controller(entity_click);

            auto entity_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
            entity_box->set_spacing(3);
            entity_box->set_margin_start(14);
            entity_expander->set_child(*entity_box);

            for (const auto &row : rows)
                entity_box->append(*create_row_widget(row));
        }

        sync_from_selection();
        m_reloading = false;
    }

    void clear_rows()
    {
        while (auto *child = m_content_box->get_first_child())
            m_content_box->remove(*child);
    }

    void append_message(const std::string &text)
    {
        auto label = Gtk::make_managed<Gtk::Label>(text);
        label->set_xalign(0);
        label->set_wrap(true);
        m_content_box->append(*label);
    }

    void set_status(const std::string &text)
    {
        m_status_label->set_text(text);
        m_status_label->set_visible(!text.empty());
    }

    std::vector<const Entity *> collect_entities(const Document &doc, const UUID &group_uuid) const
    {
        std::vector<const Entity *> entities;
        for (const auto &[uu, entity] : doc.m_entities) {
            if (entity->m_group != group_uuid)
                continue;
            if (!entity->of_type(Entity::Type::LINE_2D, Entity::Type::CIRCLE_2D, Entity::Type::ARC_2D,
                                 Entity::Type::POINT_2D, Entity::Type::BEZIER_2D))
                continue;
            entities.push_back(entity.get());
        }
        std::ranges::sort(entities, [](const Entity *a, const Entity *b) {
            if (a->get_type() != b->get_type())
                return a->get_type() < b->get_type();
            return static_cast<std::string>(a->m_uuid) < static_cast<std::string>(b->m_uuid);
        });
        return entities;
    }

    std::map<UUID, std::string> make_entity_names(const Document &, const std::vector<const Entity *> &entities) const
    {
        std::map<UUID, std::string> names;
        std::map<Entity::Type, unsigned int> counters;
        for (const auto *entity : entities) {
            auto &counter = counters[entity->get_type()];
            counter++;
            if (!entity->m_name.empty())
                names.emplace(entity->m_uuid, entity->m_name);
            else
                names.emplace(entity->m_uuid, entity_type_key(entity->get_type()) + "_" + std::to_string(counter));
        }
        return names;
    }

    std::vector<SketchRowInfo> collect_constraint_rows(const Document &doc, const Entity &entity,
                                                       const std::map<UUID, std::string> &entity_names) const
    {
        std::vector<SketchRowInfo> dimensions;
        std::vector<SketchRowInfo> constraints;

        for (const auto &[uu, constraint_ptr] : doc.m_constraints) {
            const auto &constraint = *constraint_ptr;
            if (constraint.m_group != entity.m_group)
                continue;
            if (!references_entity(constraint, entity.m_uuid))
                continue;

            auto row = make_row_info(doc, constraint, entity.m_uuid, entity_names);
            if (!row)
                continue;
            if (row->editable)
                dimensions.push_back(*row);
            else
                constraints.push_back(*row);
        }

        auto sorter = [](const SketchRowInfo &a, const SketchRowInfo &b) { return a.label < b.label; };
        std::ranges::sort(dimensions, sorter);
        std::ranges::sort(constraints, sorter);
        dimensions.insert(dimensions.end(), constraints.begin(), constraints.end());
        return dimensions;
    }

    bool passes_filter(const std::string &entity_name, const std::vector<SketchRowInfo> &rows,
                       const std::string &filter) const
    {
        if (filter.empty())
            return true;
        if (to_lower(entity_name).find(filter) != std::string::npos)
            return true;
        return std::ranges::any_of(rows, [&filter](const auto &row) {
            return to_lower(row.label).find(filter) != std::string::npos;
        });
    }

    Gtk::Widget *create_row_widget(const SketchRowInfo &row)
    {
        auto box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 6);
        box->set_margin_start(6);

        if (row.toggleable) {
            auto toggle = Gtk::make_managed<Gtk::CheckButton>(row.label);
            toggle->set_active(true);
            toggle->set_hexpand(true);
            toggle->set_halign(Gtk::Align::START);
            toggle->signal_toggled().connect([this, row, toggle] {
                if (m_reloading || toggle->get_active())
                    return;
                std::string error;
                const bool ok = m_ctx.core && m_ctx.core->apply_document_edit(
                                                     "sketch tree remove constraint",
                                                     [row](Document &doc, std::string &) {
                                                         doc.m_constraints.erase(row.constraint_uuid);
                                                         if (auto *entity = doc.get_entity_ptr(row.owner_entity_uuid))
                                                             doc.set_group_solve_pending(entity->m_group);
                                                         return true;
                                                     },
                                                     &error);
                if (!ok) {
                    toggle->set_active(true);
                    show_error(error.empty() ? "Could not remove constraint." : error);
                }
            });
            add_row_click_controller(*toggle, row.owner_entity_uuid, row.constraint_uuid);
            box->append(*toggle);

            return box;
        }

        auto label = Gtk::make_managed<Gtk::Label>(row.label);
        label->set_xalign(0);
        label->set_hexpand(true);
        box->append(*label);
        m_constraint_labels.emplace(row.constraint_uuid, label);

        auto spin = Gtk::make_managed<Gtk::SpinButton>();
        spin->set_numeric(true);
        spin->set_range(row.min_value, row.max_value);
        spin->set_value(row.value);
        spin->set_digits(row.unit == DatumUnit::DEGREE ? 2 : 3);
        spin->set_increments(row.unit == DatumUnit::INTEGER ? 1 : .1, row.unit == DatumUnit::INTEGER ? 10 : 1);
        spin->set_width_chars(8);
        box->append(*spin);

        const auto suffix = unit_suffix(row.unit);
        if (!suffix.empty()) {
            auto suffix_label = Gtk::make_managed<Gtk::Label>(suffix);
            suffix_label->set_xalign(0);
            box->append(*suffix_label);
        }

        auto commit = [this, row, spin] {
            if (m_reloading || !m_ctx.core)
                return;
            std::string error;
            const double value = spin->get_value();
            const bool ok = m_ctx.core->apply_document_edit(
                    "sketch tree edit " + row.label,
                    [row, value](Document &doc, std::string &local_error) {
                        auto *constraint = doc.get_constraint_ptr(row.constraint_uuid);
                        auto *datum = dynamic_cast<IConstraintDatum *>(constraint);
                        if (!datum) {
                            local_error = "Constraint is not editable.";
                            return false;
                        }
                        const auto [min_value, max_value] = datum->get_datum_range();
                        if (value < min_value || value > max_value) {
                            local_error = "Value is outside the allowed range.";
                            return false;
                        }
                        datum->set_datum(value);
                        if (auto *entity = doc.get_entity_ptr(row.owner_entity_uuid))
                            doc.set_group_solve_pending(entity->m_group);
                        return true;
                    },
                    &error);
            if (!ok) {
                show_error(error.empty() ? "Could not update the sketch value." : error);
                reload();
            }
        };

        spin->signal_value_changed().connect(commit);
        spinbutton_connect_activate_immediate(*spin, commit);
        auto focus = Gtk::EventControllerFocus::create();
        focus->signal_leave().connect(commit);
        spin->add_controller(focus);

        add_row_click_controller(*box, row.owner_entity_uuid, row.constraint_uuid);
        return box;
    }

    void add_row_click_controller(Gtk::Widget &widget, const UUID &entity_uuid, const UUID &constraint_uuid)
    {
        auto click = Gtk::GestureClick::create();
        click->set_button(GDK_BUTTON_PRIMARY);
        click->signal_pressed().connect([this, entity_uuid, constraint_uuid](int, double, double) {
            select_constraint(entity_uuid, constraint_uuid);
        });
        widget.add_controller(click);
    }

    void select_entity(const UUID &entity_uuid)
    {
        if (!m_ctx.window)
            return;
        SelectableRef sr{.type = SelectableRef::Type::ENTITY, .item = entity_uuid};
        m_ctx.window->get_canvas().set_selection({sr}, true);
    }

    void select_constraint(const UUID &entity_uuid, const UUID &constraint_uuid)
    {
        if (!m_ctx.window)
            return;
        SelectableRef entity_sr{.type = SelectableRef::Type::ENTITY, .item = entity_uuid};
        SelectableRef constraint_sr{.type = SelectableRef::Type::CONSTRAINT, .item = constraint_uuid};
        m_ctx.window->get_canvas().set_selection({entity_sr, constraint_sr}, true);
    }

    void sync_from_selection()
    {
        if (!m_ctx.window)
            return;

        std::optional<UUID> selected_entity;
        std::optional<UUID> selected_constraint;
        for (const auto &sr : m_ctx.window->get_canvas().get_selection()) {
            if (sr.type == SelectableRef::Type::ENTITY)
                selected_entity = sr.item;
            else if (sr.type == SelectableRef::Type::CONSTRAINT)
                selected_constraint = sr.item;
        }

        for (const auto &[entity_uuid, expander] : m_entity_expanders) {
            expander->set_expanded(selected_entity && *selected_entity == entity_uuid);
        }
        for (const auto &[entity_uuid, label] : m_entity_labels) {
            const auto text = label->get_text();
            if (selected_entity && *selected_entity == entity_uuid)
                label->set_markup("<b>" + Glib::Markup::escape_text(text) + "</b>");
            else
                label->set_text(text);
        }
        for (const auto &[constraint_uuid, label] : m_constraint_labels) {
            if (!label)
                continue;
            const auto text = label->get_text();
            if (selected_constraint && *selected_constraint == constraint_uuid && !text.empty())
                label->set_markup("<b>" + Glib::Markup::escape_text(text) + "</b>");
            else if (!text.empty())
                label->set_text(text);
        }
    }

    void show_error(const std::string &message)
    {
        set_status(message);
        if (!m_ctx.window)
            return;
        auto dialog = Gtk::AlertDialog::create("Sketch Tree");
        dialog->set_detail(message);
        dialog->show(*m_ctx.window);
    }
};

} // namespace

extern "C" bool dune_register_addon(dune3d::AddonContext &ctx)
{
    if (!ctx.docks)
        return false;

    return ctx.docks->register_left_panel("dev.justin.sketch-tree", "Sketch Tree",
                                          [&ctx] { return Gtk::make_managed<SketchTreePanel>(ctx); });
}
