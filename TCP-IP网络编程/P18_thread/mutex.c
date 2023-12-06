/*
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL); pthread_mutex_destroy(&mutex);
    pthread_mutex_lock(&mutex); pthread_mutex_unlock(&mutex);
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

const int NUM_THREAD=100;
long long num=0;
pthread_mutex_t mutex;

void* thread_inc(void* arg);
void* thread_des(void* arg);

int main(int argc, char* argv[])
{
    pthread_t thread_id[NUM_THREAD];
    pthread_mutex_init(&mutex, NULL);
    for(int i=0;i<NUM_THREAD;++i)
    {
        if(i%2)
            pthread_create(&thread_id[i], NULL, thread_inc, NULL);
        else
            pthread_create(&thread_id[i], NULL, thread_des, NULL);
    }
    for(int i=0;i<NUM_THREAD;++i)
        pthread_join(thread_id[i], NULL);
    printf("result: %lld \n", num);
    pthread_mutex_destroy(&mutex);

    return 0;
}

void* thread_inc(void* arg)
{
    pthread_mutex_lock(&mutex);
    for(int i=0;i<50000;++i)
    {
        num++;
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}
void* thread_des(void* arg)
{
    pthread_mutex_lock(&mutex);
    for(int i=0;i<50000;++i)
    {
        num--;
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

