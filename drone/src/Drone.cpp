#include "Drone.h"

// Implementazione dei metodi della classe Drone
Drone::Drone(int id) :  {
    drone_id = id;
    battery_level = 100, 
    is_charging= false,
    x= 0;
    y= 0;
    speed = 0;
    control_radius = 10;
 }

void Drone::update(int ) {
    
}

void Drone::chargeDrone() {
    // Implementazione della ricarica del drone
    // ...
}

void Drone::move() {
    // Implementazione del movimento del drone
    // ...
}

// Altri metodi della classe Drone
// ...
