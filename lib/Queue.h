#ifndef QUEUE_H
#define QUEUE_H

#include <Arduino.h>

#define MAX_QUEUE_SIZE 20

struct Message {
    uint8_t payload[20];
};

class Queue {
private:
    Message queue[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int itemCount;

public:
    Queue();
    bool isEmpty();
    bool isFull();
    bool enqueue(Message m);
    Message dequeue();
};

#endif