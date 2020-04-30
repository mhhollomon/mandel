#include "utils.hpp"

#include "MandelWin.hpp"
#include "MandelApp.hpp"

int main(int argc, char *argv[]) {

    auto app = MandelApp::create(argc, argv,
        "com.codevamping.mandel", Gio::APPLICATION_NON_UNIQUE);

    return app->run();
}
