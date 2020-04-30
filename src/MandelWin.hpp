#if !defined(MANDELWIN_HPP_)
#define MANDELWIN_HPP

#include "utils.hpp"

#include <gtkmm/applicationwindow.h>

#include <string>

class MandelWin : public Gtk::ApplicationWindow {
  public:
    MandelWin(CLIOptions clopts);
    virtual ~MandelWin();

  protected:
    // Signal handler
    void on_button_quit_clicked();

    CLIOptions clopts_;

    std::string _get_sample_value();


};
#endif
