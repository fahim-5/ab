#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define N 8
#define CAPACITY 3


pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv_shuttle_arrived = PTHREAD_COND_INITIALIZER;
pthread_cond_t cv_passenger_boarded = PTHREAD_COND_INITIALIZER;
pthread_cond_t cv_next_trip = PTHREAD_COND_INITIALIZER;


int queue[N];
int front = 0, rear = 0;
int waiting_count = 0;
bool shuttle_here = false;
int passengers_on_board = 0;


void board(int id) {
    printf("Passenger %d boarded the shuttle\n", id);
}

void depart() {
    printf("Shuttle departed\n");
    sleep(1);
}

void enqueue(int id) {
    queue[rear++] = id;
    waiting_count++;
}

int dequeue() {
    int id = queue[front++];
    waiting_count--;
    return id;
}


void *shuttle_func(void *arg) {
    while (1) {
        pthread_mutex_lock(&m);

       
        while (waiting_count == 0) {
            pthread_cond_wait(&cv_next_trip, &m);
        }

     
        shuttle_here = true;
        passengers_on_board = 0;
        
        
        pthread_cond_broadcast(&cv_shuttle_arrived);

        
        while (passengers_on_board < CAPACITY && waiting_count > 0) {
            pthread_cond_wait(&cv_passenger_boarded, &m);
        }

       
        shuttle_here = false;
        depart();

        pthread_mutex_unlock(&m);
        
    
        if (front == N) break;
    }
    return NULL;
}


void *passenger_func(void *arg) {
    int id = *((int *)arg);
    free(arg);

    pthread_mutex_lock(&m);

    enqueue(id);
    printf("Passenger %d is waiting\n", id);

    
    if (waiting_count == 1) {
        pthread_cond_signal(&cv_next_trip);
    }


    while (!shuttle_here || queue[front] != id || passengers_on_board >= CAPACITY) {
        pthread_cond_wait(&cv_shuttle_arrived, &m);
    }

 
    dequeue();
    board(id);
    passengers_on_board++;

   
    pthread_cond_signal(&cv_passenger_boarded);

    pthread_mutex_unlock(&m);
    return NULL;
}

int main() {
    pthread_t shuttle;
    pthread_t passengers[N];

    pthread_create(&shuttle, NULL, shuttle_func, NULL);

   
    for (int i = 0; i < N; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&passengers[i], NULL, passenger_func, id);
        usleep(100000); // Small delay to simulate arrival spread
    }


    for (int i = 0; i < N; i++) {
        pthread_join(passengers[i], NULL);
    }

   
    pthread_join(shuttle, NULL);

    return 0;
}
