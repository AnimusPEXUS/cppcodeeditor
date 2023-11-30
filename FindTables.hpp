#ifndef FIND_TABLES_HPP
#define FIND_TABLES_HPP

#include <filesystem>

#include "FindTypes.hpp"

namespace wayround_i2p
{
namespace codeeditor
{

    class FindFileResultTreeItem;
    class FindFileResultTreeItemItem;

    using FindFileResultTreeItemP     = Glib::RefPtr<FindFileResultTreeItem>;
    using FindFileResultTreeItemItemP = Glib::RefPtr<FindFileResultTreeItemItem>;

    class FindFileResultTreeItem : public Glib::Object
    {
      public:
        static Glib::RefPtr<FindFileResultTreeItem> create(
            std::filesystem::path subpath
        );

        ~FindFileResultTreeItem();

        const std::filesystem::path subpath;

      protected:
        FindFileResultTreeItem(
            std::filesystem::path subpath
        );

      private:
        Glib::RefPtr<Gio::ListStore<FindFileResultTreeItemItem>> items;

      public: // methods
        FindFileResultTreeItemItemP create_item(
            unsigned int line,
            std::string  text,
            unsigned int start_offset,
            unsigned int end_offset
        );

        Glib::RefPtr<Gio::ListStore<FindFileResultTreeItemItem>>
            get_list_store();
    };

    class FindFileResultTreeItemItem : public Glib::Object
    {
      public:
        static FindFileResultTreeItemItemP create(
            unsigned int line,
            std::string  text,
            unsigned int start_offset,
            unsigned int end_offset
        );

        ~FindFileResultTreeItemItem();

        const unsigned int line;
        const std::string  text;
        unsigned int       start_offset;
        unsigned int       end_offset;

      protected:
        FindFileResultTreeItemItem(
            unsigned int line,
            std::string  text,
            unsigned int start_offset,
            unsigned int end_offset
        );
    };

    class FindFileResultTreeItemWidget : public Gtk::Box
    {
      public:
        FindFileResultTreeItemWidget(
            const Glib::RefPtr<Gtk::ListItem> &list_item
        );
        ~FindFileResultTreeItemWidget();

        void bind(const Glib::RefPtr<Gtk::ListItem> &list_item);
        void unbind(const Glib::RefPtr<Gtk::ListItem> &list_item);

      private:
        Gtk::Label subpath;
        Gtk::Label found_count;
    };

    class FindFileResultTreeItemItemWidget : public Gtk::Box
    {
      public:
        FindFileResultTreeItemItemWidget(
            const Glib::RefPtr<Gtk::ListItem> &list_item
        );
        ~FindFileResultTreeItemItemWidget();

        void bind(const Glib::RefPtr<Gtk::ListItem> &list_item);
        void unbind(const Glib::RefPtr<Gtk::ListItem> &list_item);

      private:
        Gtk::Label line;
        Gtk::Label text;
    };

} // namespace codeeditor
} // namespace wayround_i2p
#endif