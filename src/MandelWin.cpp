#include "MandelWin.hpp"

#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/drawingarea.h>


MandelWin::MandelWin(CLIOptions clopts) :
        clopts_{clopts} {

    set_title("Mandel-GTK");

    auto *hbox = Gtk::make_managed<Gtk::Box>();

    hbox->set_spacing(3);
    
    Gtk::Label *label;
    Gtk::Entry *entry;

    // Sample
    label = Gtk::make_managed<Gtk::Label>("Sample:");
    entry = Gtk::make_managed<Gtk::Entry>();
    entry->set_editable(false);
    entry->set_text(_get_sample_value());

    hbox->pack_start(*label, Gtk::PACK_SHRINK, 0);
    hbox->pack_start(*entry, Gtk::PACK_EXPAND_WIDGET, 2);

    // Quit button
    auto *quit = Gtk::make_managed<Gtk::Button>("Quit");

    quit->signal_clicked().connect( 
            sigc::mem_fun(*this,
              &MandelWin::on_button_quit_clicked) );

    hbox->pack_start(*quit, Gtk::PACK_EXPAND_PADDING);

    auto *vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);

    vbox->pack_start(*hbox, Gtk::PACK_SHRINK, 1);
    
    auto *da = Gtk::make_managed<Gtk::DrawingArea>();
    da->set_size_request(200, 200);

    vbox->pack_start(*da, Gtk::PACK_EXPAND_WIDGET, 2);


    add(*vbox);

    show_all_children();
}

void MandelWin::on_button_quit_clicked() {
    hide();
}

std::string MandelWin::_get_sample_value() {
    return std::to_string(clopts_.samples);
}


MandelWin::~MandelWin() {}
