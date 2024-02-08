#include "Drone.h"

int main() {

    Swarm swarm = Swarm();
    swarm.init();
    swarm.init_drones(DRONES_COUNT);

    int t = 0;
    while(t<=HORIZON) {
        swarm.await_sync(); 
        swarm.tick();
        swarm.log();
        t++;
    }

    return 0;
}
