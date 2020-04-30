#if !defined(MANDELAPP_HPP_)
#define MANDELAPP_HPP

#include "utils.hpp"

#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/builder.h>

class MandelApp : public Gtk::Application {

  protected:
    MandelApp(int& argc, char**& argv,
            const Glib::ustring& application_id=Glib::ustring(), 
            Gio::ApplicationFlags flags=Gio::APPLICATION_FLAGS_NONE);

  public:
    static Glib::RefPtr<MandelApp> create(int& argc, char**& argv, 
            const Glib::ustring& application_id=Glib::ustring(), 
            Gio::ApplicationFlags flags=Gio::APPLICATION_FLAGS_NONE);

  protected:
    //Overrides of default signal handlers:
    void on_startup() override;
    void on_activate() override;

  private:
    void _create_window();
    void _create_menubar();

    void on_window_hide(Gtk::Window* window);

    void on_menu_file_quit();

    Glib::RefPtr<Gtk::Builder> builder_;
    CLIOptions clopts_;
};


#endif
