#include "ControlCenter.h"
#include <vector>
// Implementazione dei metodi della classe ControlCenter
ControlCenter::ControlCenter() {/* Costruttore */}

void ControlCenter::addDrone(Drone drone) {
    // Aggiunge un drone al centro di controllo
    drones.push_back(drone);
}

void ControlCenter::sendInstructions() {
    // Implementazione dell'invio di istruzioni ai droni
    // ...
}

void ControlCenter::updateMonitoringData() {
    // Implementazione dell'aggiornamento dei dati di monitoraggio
    // ...
}

std::vector<Drone>& ControlCenter::getDrones() {
    return drones;
}

// Altri metodi della classe ControlCenter
// ...
