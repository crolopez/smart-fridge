#include <pthread.h>
#include <stdlib.h>
#include "queue.h"

int sf_queue_set(queue *q, int size) {
    int i;

    if (size < MIN_QUEUE_SIZE) {
        return 1;
    } else if (size > MAX_QUEUE_SIZE) {
        return 2;
    }

    q->data_queue = (void **) malloc(size * sizeof(char *));
    for (i = 0; i < size; i++) {
        q->data_queue[i] = NULL;
    }

    q->size = size;
    q->write_p = q->current = 0;

    return 0;
}

int sf_queue_add(queue *q, void *value) {
    int i;
    int retval;
    int size;
    int write_p;

    retval = 1;
    pthread_mutex_lock(&q->queue_lock);
    size = q->size;
    write_p = q->write_p;

    if (q->current < size) {
        for (i = 0; i < size; i++) {
            int p = (write_p + i) % size;
            if (!q->data_queue[p]) {
                retval = 0;
                q->data_queue[p] = value;
                q->write_p = (write_p + i + 1) % size;
                q->current++;
                break;
            }
        }
    }
    pthread_mutex_unlock(&q->queue_lock);
    return retval;
}

void *sf_queue_get(queue *q) {
    int i;
    int write_p;
    int size;
    void *value = NULL;

    pthread_mutex_lock(&q->queue_lock);
    write_p = q->write_p;
    size = q->size;

    if (q->current) {
        for (i = 0; i < size; i++) {
            int p = (write_p - i + size) % size;
            if (q->data_queue[p]) {
                value = q->data_queue[p];
                q->data_queue[p] = NULL;
                q->write_p = p;
                q->current--;
                break;
            }
        }
    }
    pthread_mutex_unlock(&q->queue_lock);
    return value;
}
