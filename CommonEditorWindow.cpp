#include <format>

#include "CommonEditorWindow.hpp"

using namespace wayround_i2p::codeeditor;

CommonEditorWindow::CommonEditorWindow(
    std::shared_ptr<ProjectCtl>  project_ctl,
    std::shared_ptr<WorkSubject> subject
) :
    main_box(Gtk::Orientation::VERTICAL, 0),
    outline_box(Gtk::Orientation::VERTICAL, 5),
    text_view_box(Gtk::Orientation::HORIZONTAL, 5)
{
    this->project_ctl = project_ctl;
    this->subject     = subject;

    outline_list_store = Gio::ListStore<OutlineTableRow>::create();

    outline_view_selection = Gtk::SingleSelection::create(
        outline_list_store
    );

    outline_view.set_model(outline_view_selection);

    // maximize();

    set_child(main_box);

    outline_view_refresh_btn.set_label("Refresh");

    main_box.append(menu_bar);
    main_box.append(paned);

    text_view_box.append(linum_area);
    text_view_box.append(text_view_sw);

    paned.set_start_child(text_view_box);
    paned.set_end_child(outline_box);

    outline_box.append(outline_view_refresh_btn);
    outline_box.append(outline_view_sw);

    text_view_sw.set_policy(
        Gtk::PolicyType::ALWAYS,
        Gtk::PolicyType::ALWAYS
    );
    outline_view_sw.set_policy(
        Gtk::PolicyType::ALWAYS,
        Gtk::PolicyType::ALWAYS
    );

    linum_area.set_content_width(50);

    text_view_box.set_vexpand(true);
    text_view_box.set_hexpand(true);
    text_view_box.set_valign(Gtk::Align::FILL);
    text_view_box.set_halign(Gtk::Align::FILL);

    text_view_sw.set_vexpand(true);
    text_view_sw.set_hexpand(true);
    text_view_sw.set_valign(Gtk::Align::FILL);
    text_view_sw.set_halign(Gtk::Align::FILL);

    outline_view_sw.set_vexpand(true);
    outline_view_sw.set_valign(Gtk::Align::FILL);

    text_view_sw.set_overlay_scrolling(false);
    outline_view_sw.set_overlay_scrolling(false);

    text_view_sw.set_kinetic_scrolling(false);
    outline_view_sw.set_kinetic_scrolling(false);

    paned.set_vexpand(true);
    paned.set_resize_end_child(false);

    text_view_sw.set_child(text_view);
    outline_view_sw.set_child(outline_view);

    text_view.set_monospace(true);
    text_view.set_buffer(subject->getTextBuffer());

    make_menubar();
    make_actions();
    make_hotkeys();

    setup_outline_columns();

    linum_area.set_draw_func(
        sigc::mem_fun(*this, &CommonEditorWindow::redraw_linum)
    );

    subject->signal_modified_changed()->connect(
        sigc::mem_fun(*this, &CommonEditorWindow::updateTitle)
    );

    subject->signal_editors_save_state()->connect(
        sigc::mem_fun(*this, &CommonEditorWindow::saveState)
    );

    subject->signal_editors_restore_state()->connect(
        sigc::mem_fun(*this, &CommonEditorWindow::restoreState)
    );

    project_ctl->signal_updated_name()->connect(
        sigc::mem_fun(*this, &CommonEditorWindow::updateTitle)
    );

    outline_view_refresh_btn.signal_clicked().connect(
        sigc::mem_fun(*this, &CommonEditorWindow::on_outline_refresh_btn)
    );

    outline_view.signal_activate().connect(
        sigc::mem_fun(*this, &CommonEditorWindow::on_outline_activate)
    );

    signal_destroy().connect(
        sigc::mem_fun(*this, &CommonEditorWindow::on_destroy_sig)
    );

    text_view_sw.get_vscrollbar()->get_adjustment()->signal_changed().connect(
        sigc::mem_fun(*this, &CommonEditorWindow::force_redraw_linum)
    );

    text_view_sw.get_vscrollbar()->get_adjustment()->signal_value_changed().connect(
        sigc::mem_fun(*this, &CommonEditorWindow::force_redraw_linum)
    );

    updateTitle();
}

CommonEditorWindow::~CommonEditorWindow()
{
    std::cout << "~CommonEditorWindow()" << std::endl;
}

void CommonEditorWindow::setup_outline_columns()
{
    auto factory = Gtk::SignalListItemFactory::create();
    factory->signal_setup().connect(
        sigc::bind(
            [](
                const Glib::RefPtr<Gtk::ListItem> &list_item
            )
            {
                list_item->set_child(*Gtk::make_managed<Gtk::Label>(
                    "",
                    Gtk::Align::START
                )
                );
            }
        )
    );
    // sigc::mem_fun(*this, &CommonEditorWindow::table_cell_setup),
    // Gtk::Align::START
    factory->signal_bind().connect(
        sigc::bind(
            [](const Glib::RefPtr<Gtk::ListItem> &list_item)
            {
                auto col = std::dynamic_pointer_cast<OutlineTableRow>(
                    list_item->get_item()
                );
                if (!col)
                    return;
                auto label = dynamic_cast<Gtk::Label *>(
                    list_item->get_child()
                );
                if (!label)
                    return;
                label->set_text(std::format("{}", col->line));
            }
        )
    );

    auto column = Gtk::ColumnViewColumn::create("Line", factory);
    column->set_fixed_width(50);
    column->set_resizable(true);
    outline_view.append_column(column);

    // -------------
    factory = Gtk::SignalListItemFactory::create();
    factory->signal_setup().connect(
        sigc::bind(
            [](
                const Glib::RefPtr<Gtk::ListItem> &list_item
            )
            {
                list_item->set_child(*Gtk::make_managed<Gtk::Label>(
                    "",
                    Gtk::Align::START
                )
                );
            }
        )
    );
    factory->signal_bind().connect(
        sigc::bind(
            [](const Glib::RefPtr<Gtk::ListItem> &list_item)
            {
                auto col = std::dynamic_pointer_cast<OutlineTableRow>(
                    list_item->get_item()
                );
                if (!col)
                    return;
                auto label = dynamic_cast<Gtk::Label *>(
                    list_item->get_child()
                );
                if (!label)
                    return;
                label->set_text(col->text);
            }
        )
    );

    column = Gtk::ColumnViewColumn::create("Text", factory);
    column->set_expand(true);
    outline_view.append_column(column);
}

void CommonEditorWindow::make_menubar()
{
    menu_model = Gio::Menu::create();
    menu_bar.set_menu_model(menu_model);

    mm_buffer = Gio::Menu::create();
    menu_model->append_submenu("Buffer", mm_buffer);

    mm_buffer_reload = Gio::MenuItem::create(
        "(Re)load",
        "editor_window.buffer_reload"
    );
    mm_buffer_save = Gio::MenuItem::create(
        "Save",
        "editor_window.buffer_save"
    );
    mm_buffer_save_as = Gio::MenuItem::create(
        "Save as..",
        "editor_window.buffer_save_as"
    );

    mm_buffer->append_item(mm_buffer_reload);
    mm_buffer->append_item(mm_buffer_save);
    mm_buffer->append_item(mm_buffer_save_as);
};

void CommonEditorWindow::make_special_menu()
{
    // note: this is to be overriden by inheriting class
    return;
}

void CommonEditorWindow::make_actions()
{
    auto action_group = Gio::SimpleActionGroup::create();
    action_group->add_action(
        "buffer_reload",
        sigc::mem_fun(*this, &CommonEditorWindow::action_buffer_reload)
    );
    action_group->add_action(
        "buffer_save",
        sigc::mem_fun(*this, &CommonEditorWindow::action_buffer_save)
    );
    action_group->add_action(
        "buffer_save_as",
        sigc::mem_fun(*this, &CommonEditorWindow::action_buffer_save_as)
    );
    insert_action_group("editor_window", action_group);
}

void CommonEditorWindow::make_special_actions()
{
    // note: this is to be overriden by inheriting class
    return;
}

void CommonEditorWindow::make_hotkeys()
{
    auto controller = Gtk::ShortcutController::create();
    controller->set_scope(Gtk::ShortcutScope::LOCAL);

    controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::KeyvalTrigger::create(
            GDK_KEY_r,
            Gdk::ModifierType::CONTROL_MASK
        ),
        Gtk::NamedAction::create("editor_window.buffer_reload")
    ));
    controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::KeyvalTrigger::create(
            GDK_KEY_s,
            Gdk::ModifierType::CONTROL_MASK
        ),
        Gtk::NamedAction::create("editor_window.buffer_save")
    ));
    controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::KeyvalTrigger::create(
            GDK_KEY_s,
            Gdk::ModifierType::CONTROL_MASK
                | Gdk::ModifierType::SHIFT_MASK
        ),
        Gtk::NamedAction::create("editor_window.buffer_save_as")
    ));
    add_controller(controller);
}

void CommonEditorWindow::make_special_hotkeys()
{
    // note: this is to be overriden by inheriting class
    return;
}

Glib::RefPtr<Gio::Menu> CommonEditorWindow::getMenuModel()
{
    return menu_model;
}

void CommonEditorWindow::updateTitle()
{
    std::string new_title;

    auto pth = subject->getPath();

    std::string mod_bullet("");
    if (subject->modified())
    {
        mod_bullet = "*";
    }

    std::string proj_name("(global)");
    if (!project_ctl->isGlobalProject())
    {
        int err                  = 0;
        std::tie(proj_name, err) = project_ctl->getProjectName();
        if (err != 0)
        {
            proj_name = "error getting project name :-(";
        }
        proj_name = std::format(
            "(proj: {})",
            proj_name
        );
    }

    new_title = std::format(
        "{}{} {} - Code Editor",
        mod_bullet,
        (pth.filename()).string(),
        proj_name
    );

    set_title(new_title);
}

void CommonEditorWindow::saveState()
{
    auto tb = subject->getTextBuffer();

    auto cur_pos             = tb->get_insert();
    auto cur_pos_iter        = tb->get_iter_at_mark(cur_pos);
    auto cur_pos_iter_offset = cur_pos_iter.get_offset();
    auto v_scrollbar         = text_view_sw.get_vscrollbar();
    auto adj                 = v_scrollbar->get_adjustment()->get_value();

    saved_editor_state.cur_pos_iter_offset = cur_pos_iter_offset;
    saved_editor_state.scroll_adj          = adj;
}

void CommonEditorWindow::restoreState()
{

    auto context = Glib::MainContext::get_default();

    context->signal_idle().connect_once(
        [this]()
        {
            auto tb = subject->getTextBuffer();

            auto new_iter = tb->get_iter_at_offset(saved_editor_state.cur_pos_iter_offset);
            tb->place_cursor(new_iter);

            auto v_scrollbar = this->text_view_sw.get_vscrollbar();
            v_scrollbar->get_adjustment()->set_value(saved_editor_state.scroll_adj);
        }
    );
}

void CommonEditorWindow::setOutlineContents(
    std::vector<std::tuple<unsigned int, std::string>> val
)
{
    // todo: save and restore current view
    outline_list_store->remove_all();

    auto i = val.begin();
    while (i != val.end())
    {
        auto x = OutlineTableRow::create();

        std::tie(x->line, x->text) = *i;
        outline_list_store->append(x);
        ++i;
    }

    outline_list_store->sort(
        sigc::slot<
            int(
                const Glib::RefPtr<const OutlineTableRow> &,
                const Glib::RefPtr<const OutlineTableRow> &
            )>(
            [](const Glib::RefPtr<const OutlineTableRow> &p1,
               const Glib::RefPtr<const OutlineTableRow> &p2)
            {
                if (p1->line == p2->line)
                {
                    return 0;
                }
                if (p1->line < p2->line)
                {
                    return -1;
                }
                if (p1->line > p2->line)
                {
                    return 1;
                }
                return 0;
            }
        )
    );
}

std::vector<std::tuple<unsigned int, std::string>>
    CommonEditorWindow::genOutlineContents()
{
    return std::vector<std::tuple<unsigned int, std::string>>();
}

void CommonEditorWindow::redraw_linum(
    const Cairo::RefPtr<Cairo::Context> &cont,
    int                                  width,
    int                                  height
)
{
    Gdk::Rectangle r;
    text_view.get_visible_rect(r);

    auto modifier = r.get_y();
    auto size     = r.get_height();

    unsigned int new_line     = 0;
    unsigned int current_line = 0;

    auto text_buff = text_view.get_buffer();

    for (unsigned int i = 0; i != size; i++)
    {
        Gtk::TextIter new_iter;
        text_view.get_iter_at_location(new_iter, 0, i + modifier);
        new_line = new_iter.get_line();
        if (new_line != current_line)
        {
            cont->move_to(0, i);
            cont->show_text(std::format("{}", new_line));
            current_line = new_line;
        }
    }
}

void CommonEditorWindow::force_redraw_linum()
{
    linum_area.queue_draw();
}

void CommonEditorWindow::saveOwnPtr(std::shared_ptr<CodeEditorAbstract> val)
{
    own_ptr = val;
}

void CommonEditorWindow::show()
{
    auto x = (Gtk::Window *)this;
    x->show();
}

void CommonEditorWindow::close()
{
    auto x = (Gtk::Window *)this;
    x->close();
}

void CommonEditorWindow::action_buffer_reload()
{
    std::cout << "reload" << std::endl;
    subject->reload();
}

void CommonEditorWindow::action_buffer_save()
{
    std::cout << "save" << std::endl;
    subject->save();
}

void CommonEditorWindow::action_buffer_save_as()
{
    std::cout << "save as" << std::endl;
}

void CommonEditorWindow::on_outline_refresh_btn()
{
    auto oc = genOutlineContents();
    setOutlineContents(oc);
}

void CommonEditorWindow::on_outline_activate(guint val)
{
    auto x = outline_list_store->get_item(val);
    // std::cout << "x == " << x->line << std::endl;

    auto tb = subject->getTextBuffer();

    auto iter_at_line = tb->get_iter_at_line(x->line);
    text_view.scroll_to(iter_at_line, 0.2);
    tb->place_cursor(iter_at_line);
}

void CommonEditorWindow::on_destroy_sig()
{
    project_ctl->unregisterEditor(own_ptr);
    own_ptr.reset();
}

/*
std::shared_ptr<ProjectCtl> CommonEditorWindow::getProjectCtl()
{
    return project_ctl;
}
*/
