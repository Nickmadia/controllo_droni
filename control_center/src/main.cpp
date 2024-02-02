#include "ControlCenter.h"
#include "config.h"

int main(int argc, char *argv[]) {
    // defining the cc
    ControlCenter cc();
    cc.init();

    for ( int t = 0; t <= HORIZON; t++){
        cc.sync(); // sync bloccante con redis

        cc.op(); 
        /* while on start up status waits for drones
        when ready starts assigning jobs
        when regime works normally
        */
        cc.log();
    }
}