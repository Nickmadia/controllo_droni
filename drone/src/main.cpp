#include "ControlCenter.h" // Includi il file dell'header della classe ControlCenter

int main() {
    ControlCenter controlCenter;
    // Aggiungi droni al controllo
    Drone drone1(1);
    Drone drone2(2);
    controlCenter.addDrone(drone1);
    controlCenter.addDrone(drone2);


    return 0;
}
