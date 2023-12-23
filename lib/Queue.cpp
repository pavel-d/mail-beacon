#include "Queue.h"

Queue::Queue() : front(0), rear(-1), itemCount(0) {}

bool Queue::isEmpty() {
    return itemCount == 0;
}

bool Queue::isFull() {
    return itemCount >= MAX_QUEUE_SIZE;
}

bool Queue::enqueue(Message m) {
    if (isFull()) {
        return 0;
    }

    if (rear == MAX_QUEUE_SIZE -1) {
        rear = -1;
    }

    queue[++rear] = m;
    itemCount++;

    return 1;
}

Message Queue::dequeue() {
    Message m = queue[front++];

    if (front == MAX_QUEUE_SIZE) {
        front = 0;
    }

    itemCount--;
    return m;
}