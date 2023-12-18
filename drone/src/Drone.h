#ifndef DRONE_H
#define DRONE_H

class Drone {
private:
    // Attributi privati della classe Drone
    int droneID; // Identificativo unico del drone
    float batteryLevel; // Livello della batteria
    bool isCharging; 
public:
    // Costruttore e metodi pubblici della classe Drone
    Drone(int id);
    void update();
    void chargeDrone();
    void move();
    // ...
};

#endif // DRONE_H
