#include <stdio.h>
#include <pthread.h>

void *hello_thread (void *arg) {
    printf("Thread %d: Hello, World!\n", (int)arg);
    return arg;
}

int main() {
    pthread_t tid;
    int status;
    
    status = pthread_create(&tid, NULL, hello_thread, NULL);
    if(status != 0)
        perror("Create thread");
    pthread_exit(NULL);
}
