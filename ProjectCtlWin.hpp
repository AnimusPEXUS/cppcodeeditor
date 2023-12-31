#ifndef PROJECT_CTL_WIN_HPP
#define PROJECT_CTL_WIN_HPP

#include <filesystem>
#include <iostream>
#include <tuple>

#include <gtkmm.h>
#include <sigc++/sigc++.h>

#include "CodeEditorAbstract.hpp"
#include "Controller.hpp"
#include "FileExplorer.hpp"
#include "WorkSubject.hpp"
#include "tables.hpp"

namespace wayround_i2p
{
namespace codeeditor
{
    class CodeEditorTableRow;
    class Controller;
    class FileExplorer;
    class WorkSubjectTableRow;

    class ProjectCtlWin : public Gtk::Window
    {
      public:
        ProjectCtlWin(std::shared_ptr<ProjectCtl> proj_ctl);

        ~ProjectCtlWin();

        std::shared_ptr<ProjectCtlWin> own_ptr;

        void updateTitle();

      private:
        std::shared_ptr<ProjectCtl> proj_ctl;
        std::shared_ptr<Controller> controller;

        Gtk::Box main_box;

        Gtk::Button show_file_explorer_btn;
        Gtk::Button show_new_worksubject_list_btn;
        Gtk::Button show_new_editor_list_btn;

        void on_show_file_explorer_btn();
        void on_show_new_worksubject_list_btn();
        void on_show_new_editor_list_btn();

        void on_hide_sig();
        void on_destroy_sig();
    };

} // namespace codeeditor
} // namespace wayround_i2p

#endif
