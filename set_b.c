#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define CAPACITY 5

// Shared variables
int waiting_passengers = 0;
int boarded_count = 0;
int current_serving_ticket = 0;
int next_ticket = 0;
int shuttle_at_terminal = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_shuttle_arrival = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_boarding_done = PTHREAD_COND_INITIALIZER;

void* shuttle_func(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        
        // Rule: Wait until at least one passenger is available
        while (waiting_passengers == 0) {
            printf("[Shuttle] No passengers. Waiting...\n");
            pthread_cond_wait(&cond_shuttle_arrival, &lock);
        }

        printf("[Shuttle] Arrived at terminal. Starting boarding...\n");
        shuttle_at_terminal = 1;
        boarded_count = 0;

        // Notify waiting passengers that shuttle is here
        pthread_cond_broadcast(&cond_shuttle_arrival);

        // Wait until boarding is complete (either shuttle full or no more waiting)
        while (boarded_count < CAPACITY && waiting_passengers > 0) {
            pthread_cond_wait(&cond_boarding_done, &lock);
        }

        shuttle_at_terminal = 0;
        printf("[Shuttle] Boarding complete. Departing with %d passengers.\n", boarded_count);
        
        pthread_mutex_unlock(&lock);
        
        // Simulate travel time
        sleep(2); 
    }
    return NULL;
}

void* passenger_func(void* arg) {
    int id = *((int*)arg);
    free(arg);

    pthread_mutex_lock(&lock);
    
    // Assign a ticket to ensure FCFS order
    int my_ticket = next_ticket++;
    waiting_passengers++;
    printf("Passenger %d arrived. (Ticket: %d)\n", id, my_ticket);

    // If shuttle is away or not my turn or shuttle is full for this trip
    // Note: FCFS is handled by current_serving_ticket
    while (!shuttle_at_terminal || my_ticket != current_serving_ticket || boarded_count >= CAPACITY) {
        // Wake shuttle if it's waiting for the first passenger
        if (waiting_passengers == 1) {
            pthread_cond_signal(&cond_shuttle_arrival);
        }
        pthread_cond_wait(&cond_shuttle_arrival, &lock);
    }

    // Boarding
    boarded_count++;
    waiting_passengers--;
    current_serving_ticket++;
    printf("Passenger %d boarded the shuttle.\n", id);

    // Signal shuttle that a passenger has boarded
    pthread_cond_signal(&cond_boarding_done);
    
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    pthread_t shuttle_tid;
    pthread_create(&shuttle_tid, NULL, shuttle_func, NULL);

    // Simulate arriving passengers
    for (int i = 0; i < 12; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_t p_tid;
        pthread_create(&p_tid, NULL, passenger_func, id);
        usleep(500000); // Passengers arrive every 0.5s
    }

    pthread_join(shuttle_tid, NULL);
    return 0;
}
