#ifndef CONTROL_CENTER_H
#define CONTROL_CENTER_H
#include <vector>
#include "Drone.h" // Includi il file dell'header della classe Drone

class ControlCenter {
private:
    std::vector<Drone> drones;
    time * grid;// griglia 2d ultima ora di verifica


public:
    // Metodi pubblici della classe ControlCenter
    ControlCenter();
    void wait();//aspetta il messaggio di "ready" dai droni
    void addDrone(Drone drone);
    void sendInstruction();
    void updateMonitoringData();
    // ...
};

#endif // CONTROL_CENTER_H
