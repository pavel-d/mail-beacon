#ifndef BEACON_H
#define BEACON_H

class Beacon {
public:
    Beacon();  // Constructor
    void setBeaconID(unsigned long);
    bool listen();
    bool transmit(bool triggerA, bool triggerB, bool triggerC, uint16_t voltage);
private:
    unsigned long beaconID;
    uint16_t counter;

    int sendMsg(uint16_t beaconID, bool triggerA, bool triggerB, bool triggerC, uint16_t counter, uint16_t voltage);
};

#endif