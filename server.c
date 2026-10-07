/*
 * QNX Embedded Device Manager - Server
 *
 * Registers the named service "qnx_device_manager", processes client
 * commands over QNX native IPC, and runs a monitor thread that
 * simulates temperature and RPM changes while the device is running.
 *
 * Build (QNX): qcc -o server server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/dispatch.h>

#define SERVER_NAME "qnx_device_manager"

#define CMD_GET_STATUS   1
#define CMD_SET_SPEED    2
#define CMD_GET_STATS    3
#define CMD_START        4
#define CMD_STOP         5
#define CMD_RESET        6
#define CMD_SHUTDOWN     7

#define DEVICE_STOPPED   0
#define DEVICE_RUNNING   1

typedef struct
{
    int command;
    int value;
} request_t;

typedef struct
{
    int status;
    int temperature;
    int speed;
    int rpm;
    int device_state;
    int messages_received;
    char message[128];
} response_t;

typedef struct
{
    int temperature;
    int speed;
    int rpm;
    int device_state;
    int messages_received;
} device_state_t;

static volatile int server_running = 1;

static device_state_t device =
{
    25,
    0,
    0,
    DEVICE_STOPPED,
    0
};

static pthread_mutex_t device_mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_signal(int sig)
{
    (void)sig;
    server_running = 0;
}

void *monitor_thread(void *arg)
{
    (void)arg;

    while(server_running)
    {
        sleep(2);

        pthread_mutex_lock(&device_mutex);

        if(device.device_state == DEVICE_RUNNING)
        {
            device.temperature++;

            if(device.temperature > 100)
            {
                device.temperature = 25;
            }

            device.rpm = device.speed * 40;
        }

        pthread_mutex_unlock(&device_mutex);
    }

    return NULL;
}

void process_command(request_t *req, response_t *res)
{
    pthread_mutex_lock(&device_mutex);

    memset(res, 0, sizeof(response_t));

    res->status = 0;

    switch(req->command)
    {
        case CMD_GET_STATUS:

            res->temperature = device.temperature;
            res->speed = device.speed;
            res->rpm = device.rpm;
            res->device_state = device.device_state;
            res->messages_received = device.messages_received;

            strcpy(res->message, "Status retrieved successfully");

            break;


        case CMD_SET_SPEED:

            if(req->value >= 0 && req->value <= 200)
            {
                device.speed = req->value;

                device.rpm = device.speed * 40;

                strcpy(res->message, "Speed updated successfully");
            }
            else
            {
                res->status = -1;

                strcpy(res->message, "Invalid speed. Range: 0-200");
            }

            break;


        case CMD_GET_STATS:

            res->messages_received =
                device.messages_received;

            strcpy(res->message,
                   "Statistics retrieved successfully");

            break;


        case CMD_START:

            device.device_state = DEVICE_RUNNING;

            strcpy(res->message,
                   "Device started successfully");

            break;


        case CMD_STOP:

            device.device_state = DEVICE_STOPPED;

            device.speed = 0;
            device.rpm = 0;

            strcpy(res->message,
                   "Device stopped successfully");

            break;


        case CMD_RESET:

            device.temperature = 25;
            device.speed = 0;
            device.rpm = 0;
            device.device_state = DEVICE_STOPPED;

            strcpy(res->message,
                   "Device reset successfully");

            break;


        case CMD_SHUTDOWN:

            server_running = 0;

            strcpy(res->message,
                   "Server shutting down");

            break;


        default:

            res->status = -1;

            strcpy(res->message,
                   "Unknown command");

            break;
    }

    device.messages_received++;

    pthread_mutex_unlock(&device_mutex);
}

int main(void)
{
    name_attach_t *attach;

    pthread_t monitor;

    request_t request;
    response_t response;

    int rcvid;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    printf("=====================================\n");
    printf(" QNX Embedded Device Manager Server\n");
    printf("=====================================\n");

    attach = name_attach(NULL, SERVER_NAME, 0);

    if(attach == NULL)
    {
        perror("name_attach failed");
        return EXIT_FAILURE;
    }

    printf("Server started successfully.\n");
    printf("Service Name: %s\n", SERVER_NAME);

    if(pthread_create(&monitor,
                      NULL,
                      monitor_thread,
                      NULL) != 0)
    {
        perror("pthread_create failed");

        name_detach(attach, 0);

        return EXIT_FAILURE;
    }

    while(server_running)
    {
        rcvid = MsgReceive(
                    attach->chid,
                    &request,
                    sizeof(request),
                    NULL);

        if(rcvid == -1)
        {
            continue;
        }

        process_command(&request, &response);

        MsgReply(
            rcvid,
            EOK,
            &response,
            sizeof(response));
    }

    pthread_join(monitor, NULL);

    name_detach(attach, 0);

    pthread_mutex_destroy(&device_mutex);

    printf("\nServer stopped.\n");

    return EXIT_SUCCESS;
}
