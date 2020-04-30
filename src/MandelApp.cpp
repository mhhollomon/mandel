#include "MandelWin.hpp"

#include "MandelApp.hpp"
#include <gtkmm.h>

MandelApp::MandelApp(int& argc, char**& argv,
        const Glib::ustring& application_id,
        Gio::ApplicationFlags flags)

    : Gtk::Application(application_id, flags),
    clopts_{parse_commandline(argc, argv)}
{
  Glib::set_application_name("Mandel");
}

Glib::RefPtr<MandelApp> MandelApp::create( int& argc, char**& argv,
        const Glib::ustring& application_id,
        Gio::ApplicationFlags flags)
{
    return Glib::RefPtr<MandelApp>(new MandelApp(argc, argv, application_id, flags));
}

void MandelApp::on_startup() {

    //Call the base class's implementation:
    Gtk::Application::on_startup();

    _create_menubar();
}

void MandelApp::on_activate() {
    _create_window();
}

void MandelApp::_create_window() {

    auto win = new MandelWin(clopts_);

    //Make sure that the application runs for as long this window is still open:
    add_window(*win);

    //Delete the window when it is hidden.
    //That's enough for this simple example.
    win->signal_hide().connect(sigc::bind<Gtk::Window*>(
    sigc::mem_fun(*this, &MandelApp::on_window_hide), win));

    win->show_all();
}

void MandelApp::on_menu_file_quit() {
  std::vector<Gtk::Window*> windows = get_windows();
  if (windows.size() > 0)
    windows[0]->hide(); // In this simple case, we know there is only one window.
}
void MandelApp::on_window_hide(Gtk::Window* window) {
    delete window;
}
//
// ###############################################
// Menu Bar
// ##############################################

const char* ui_info =
    "<interface>"
    "  <menu id='menubar'>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_File</attribute>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>_Quit</attribute>"
    "          <attribute name='action'>app.quit</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;q</attribute>"
    "        </item>"
    "      </section>"
    "    </submenu>"
    "  </menu>"
    "</interface>";


void MandelApp::_create_menubar() {
    add_action("quit", sigc::mem_fun(*this, &MandelApp::on_menu_file_quit));

    builder_ = Gtk::Builder::create();
    builder_->add_from_string(ui_info);

    auto object = builder_->get_object("menubar");
    auto gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);

    set_menubar(gmenu);
}
