/************************************************************
    File Name : server.c
    Author: Ginakira
    Mail: ginakira@outlook.com
    Github: https://github.com/Ginakira
    Created Time: 2020/04/07 18:20:23
************************************************************/

#include "../common/color.h"
#include "../common/common.h"
#include "../common/head.h"
#include "../common/tcp_server.h"

#define MAXTASK 100
#define MAXTHREAD 20

char ch_char(char c) {
    if (c >= 'a' && c <= 'z')
        return c - 32;
    else
        return c;
}

void do_echo(int fd) {
    char buf[512] = {0}, ch;
    int ind = 0;
    while (1) {
        if (recv(fd, &ch, 1, 0) <= 0) {
            break;
        }
        if (ind < sizeof(buf)) {
            buf[ind++] = ch_char(ch);
        }
        if (ch == '\n') {
            send(fd, buf, ind, 0);
            ind = 0;
            continue;
        }
    }
}

typedef struct {
    int sum;  // Amount of tasks
    int* fd;
    int head, tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} TaskQueue;

void TaskQueueInit(TaskQueue* queue, int sum) {
    queue->sum = sum;
    queue->fd = (int*)calloc(sum, sizeof(int));
    queue->head = queue->tail = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    return;
}

void TaskQueuePush(TaskQueue* queue, int fd) {
    pthread_mutex_lock(&queue->mutex);
    queue->fd[queue->tail] = fd;
    printf(GREEN "<TaskPush>" NONE " : %d\n", fd);
    if (++queue->tail == queue->sum) {
        printf(RED "<QueueEnd>" NONE " : %d\n", fd);
        queue->tail = 0;
    }
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

int TaskQueuePop(TaskQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->tail == queue->head) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    int fd = queue->fd[queue->head];
    printf(GREEN "<TaskPop>" NONE " : %d\n", fd);
    if (++queue->head == queue->sum) {
        queue->head = 0;
        printf(RED "<QueueEnd>" NONE " : %d\n", fd);
    }
    pthread_mutex_unlock(&queue->mutex);
    return fd;
}

void* thread_run(void* arg) {
    pthread_t tid = pthread_self();
    pthread_detach(tid);

    TaskQueue* queue = (TaskQueue*)arg;

    while (1) {
        int fd = TaskQueuePop(queue);
        do_echo(fd);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }

    int port, server_listen, fd;
    port = atoi(argv[1]);

    if ((server_listen = socket_create(port)) < 0) {
        perror("socket_create");
        exit(1);
    }

    TaskQueue queue;
    TaskQueueInit(&queue, MAXTASK);

    pthread_t* tid = (pthread_t*)calloc(MAXTHREAD, sizeof(pthread_t));
    for (int i = 0; i < MAXTHREAD; ++i) {
        pthread_create(&tid[i], NULL, thread_run, (void*)&queue);
    }

    while (1) {
        if ((fd = accept(server_listen, NULL, NULL)) < 0) {
            perror("accept");
            exit(1);
        }
        printf(BLUE "<Login>" NONE " : %d\n", fd);
        TaskQueuePush(&queue, fd);
    }

    return 0;
}