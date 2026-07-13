
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 10
#define NUM_THREADS 4
#define TIME_QUANTUM 2
#define NUM_PROCESSES 5
#define MAX_ITEMS 15

typedef struct { int items[BUFFER_SIZE], count, in, out; } SharedBuffer;
SharedBuffer buffer = {{0}, 0, 0, 0};
pthread_mutex_t buf_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t empty_slots, filled_slots;
volatile int running = 1;
int total_produced = 0, total_consumed = 0;

typedef struct PCB { int pid, burst, rem; char name[20]; struct PCB* next; } PCB;
PCB *queue_head = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

void* producer(void* arg) {
    int item = 0;
    while(running && total_produced < MAX_ITEMS) {
        usleep(300000);
        sem_wait(&empty_slots);
        pthread_mutex_lock(&buf_mutex);
        buffer.items[buffer.in] = rand() % 1000;
        buffer.in = (buffer.in + 1) % BUFFER_SIZE;
        buffer.count++;
        total_produced++;
        printf("[PRODUCER-0] Produced item #%d: value=%d | Buffer: %d/%d\n", 
               ++item, buffer.items[(buffer.in-1+BUFFER_SIZE)%BUFFER_SIZE], buffer.count, BUFFER_SIZE);
        pthread_mutex_unlock(&buf_mutex);
        sem_post(&filled_slots);
    }
    printf("[PRODUCER-0] Thread terminating. Total produced: %d\n", item);
    return NULL;
}

void* consumer(void* arg) {
    int item = 0, data;
    while(running && total_consumed < MAX_ITEMS) {
        sem_wait(&filled_slots);
        pthread_mutex_lock(&buf_mutex);
        data = buffer.items[buffer.out];
        buffer.out = (buffer.out + 1) % BUFFER_SIZE;
        buffer.count--;
        total_consumed++;
        printf("[CONSUMER-1] Consumed item #%d: value=%d | Buffer: %d/%d\n", 
               ++item, data, buffer.count, BUFFER_SIZE);
        pthread_mutex_unlock(&buf_mutex);
        sem_post(&empty_slots);
        usleep(500000);
    }
    printf("[CONSUMER-1] Thread terminating. Total consumed: %d\n", item);
    return NULL;
}

void init_queue() {
    char* names[] = {"P1-Browser", "P2-Editor", "P3-Compiler", "P4-Music", "P5-Terminal"};
    int bursts[] = {8, 4, 6, 3, 5};
    PCB* procs[NUM_PROCESSES];
    for(int i=0; i<NUM_PROCESSES; i++) {
        procs[i] = malloc(sizeof(PCB));
        procs[i]->pid = i+1; procs[i]->burst = bursts[i]; 
        procs[i]->rem = bursts[i]; strcpy(procs[i]->name, names[i]);
    }
    for(int i=0; i<NUM_PROCESSES; i++) procs[i]->next = procs[(i+1)%NUM_PROCESSES];
    queue_head = procs[0];
}

void* scheduler(void* arg) {
    printf("[SCHEDULER-2] Thread started\n");
    printf("[SCHEDULER-2] Initializing Round-Robin Scheduler...\n\n");
    printf("--- INITIAL PROCESS QUEUE ---\n");
    PCB* p = queue_head;
    for(int i=0; i<NUM_PROCESSES; i++) {
        printf("[%s: rem=%ds] -> ", p->name, p->rem); p = p->next;
    }
    printf("(HEAD)\n\n");

    int cycle = 0;
    while(running && cycle < 2) {
        printf("========== SCHEDULING CYCLE %d ==========\n", cycle+1);
        p = queue_head;
        for(int i=0; i<NUM_PROCESSES && running; i++) {
            if(p->rem > 0) {
                int q = (p->rem < TIME_QUANTUM) ? p->rem : TIME_QUANTUM;
                printf("\n[RR] Dispatching: %s (PID:%d, Remaining:%ds)\n", p->name, p->pid, p->rem);
                printf("[RR] Executing for %d seconds...\n", q);
                sleep(q);
                p->rem -= q;
                if(p->rem <= 0) printf("[RR] Process %s COMPLETED\n", p->name);
                else printf("[RR] Process %s preempted (Remaining:%ds)\n", p->name, p->rem);
            }
            p = p->next;
            usleep(100000);
        }
        printf("\n--- END OF CYCLE %d ---\n\n", cycle+1);
        p = queue_head;
        for(int i=0; i<NUM_PROCESSES; i++) { p->rem = p->burst; p = p->next; }
        cycle++;
    }
    printf("[SCHEDULER-2] Thread terminating\n");
    return NULL;
}

void* logger(void* arg) {
    printf("[LOGGER-3] Thread started\n");
    int count = 0;
    while(running && count < 3) {
        sleep(4);
        pthread_mutex_lock(&buf_mutex);
        printf("\n[LOGGER-3] === SYSTEM STATE REPORT #%d ===\n", ++count);
        printf("  Buffer: %d/%d items\n", buffer.count, BUFFER_SIZE);
        printf("  Total Produced: %d | Total Consumed: %d\n", total_produced, total_consumed);
        pthread_mutex_unlock(&buf_mutex);
    }
    printf("[LOGGER-3] Thread terminating\n");
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t t[NUM_THREADS];
    printf("============================================================\n");
    printf("  ST5004CEM - Task 1: Process Management & Threading\n");
    printf("============================================================\n\n");
    printf("[INIT] Buffer size: %d\n", BUFFER_SIZE);
    printf("[INIT] Time quantum: %d seconds\n", TIME_QUANTUM);
    printf("[INIT] Number of processes: %d\n\n", NUM_PROCESSES);

    init_queue();
    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&filled_slots, 0, 0);

    pthread_create(&t[0], NULL, producer, NULL);
    pthread_create(&t[1], NULL, consumer, NULL);
    pthread_create(&t[2], NULL, scheduler, NULL);
    pthread_create(&t[3], NULL, logger, NULL);

    sleep(12);
    printf("\n[MAIN] Signaling termination...\n");
    running = 0;
    for(int i=0; i<NUM_THREADS; i++) pthread_join(t[i], NULL);

    printf("\n[MAIN] Total produced: %d | Total consumed: %d\n", total_produced, total_consumed);
    printf("[MAIN] Application terminated successfully\n");
    return 0;
}
