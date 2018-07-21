#ifndef SF_QUEUE
#define SF_QUEUE

#define MIN_QUEUE_SIZE 5
#define MAX_QUEUE_SIZE 200

typedef struct queue {
    void **data_queue;
    int size;
    int current;
    int write_p;
    pthread_mutex_t queue_lock;
} queue;

int sf_queue_set(queue *q, int size);
int sf_queue_add(queue *q, void *value);
void *sf_queue_get(queue *q);

#endif
