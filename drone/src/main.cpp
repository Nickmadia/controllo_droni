#include "Drone.h"

int main() {

    Swarm swarm = Swarm();
    swarm.init();
    swarm.init_drones(DRONES_COUNT);
    int t = 0;
    
    swarm.await_sync(); 
    while(t<=HORIZON) {
        swarm.tick();
        //swarm.log();
        t++;
    }

    return 0;
}
