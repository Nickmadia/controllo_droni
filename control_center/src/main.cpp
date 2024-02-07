#include "ControlCenter.h"
#include "config.h"

int main(int argc, char *argv[]) {
    // defining the cc


    ControlCenter cc();
    cc.init();
    print_parameters(); //print parameters to stdoutput for readability

    for ( int t = 0; t <= HORIZON; t++){
        cc.sync(); // sync bloccante con redis

        cc.tick(); // performs the action in a given istant of time t based on current status
        cc.log(); // logs data + monitors
    }

    cc.shutdown(); // cleans up everything
    return 0;
}