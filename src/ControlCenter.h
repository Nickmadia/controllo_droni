#ifndef CONTROL_CENTER_H
#define CONTROL_CENTER_H

#include "Drone.h" // Includi il file dell'header della classe Drone

class ControlCenter {
private:
    vector<Drone> drones;
    // Altri attributi privati della classe ControlCenter
    // ...

public:
    // Metodi pubblici della classe ControlCenter
    ControlCenter();
    void addDrone(Drone drone);
    void sendInstructions();
    void updateMonitoringData();
    vector<Drone>& getDrones(); // Metodo per ottenere i droni
    // ...
};

#endif // CONTROL_CENTER_H
