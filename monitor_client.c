/*
 * QNX Embedded Device Manager - Automatic Monitoring Client
 *
 * Polls the server every 2 seconds with CMD_GET_STATUS and displays
 * live device values.
 *
 * Build (QNX): qcc -o monitor_client monitor_client.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/dispatch.h>

#define SERVER_NAME "qnx_device_manager"

#define CMD_GET_STATUS 1

#define DEVICE_STOPPED 0
#define DEVICE_RUNNING 1

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

int main(void)
{
    int server_coid;

    request_t request;
    response_t response;

    server_coid =
        name_open(SERVER_NAME, 0);

    if(server_coid == -1)
    {
        perror("Unable to connect to server");
        return EXIT_FAILURE;
    }

    printf("=====================================\n");
    printf(" QNX LIVE DEVICE MONITOR\n");
    printf("=====================================\n");

    while(1)
    {
        memset(&request, 0, sizeof(request));
        memset(&response, 0, sizeof(response));

        request.command = CMD_GET_STATUS;

        if(MsgSend(server_coid,
                   &request,
                   sizeof(request),
                   &response,
                   sizeof(response)) == -1)
        {
            perror("MsgSend failed");
            break;
        }

        /* Clear screen and move cursor to top-left */
        printf("\033[2J\033[H");

        printf("=====================================\n");
        printf("       QNX DEVICE MONITOR\n");
        printf("=====================================\n");

        printf("Temperature : %d C\n",
               response.temperature);

        printf("Speed       : %d\n",
               response.speed);

        printf("RPM         : %d\n",
               response.rpm);

        printf("Device State: %s\n",
               response.device_state ==
               DEVICE_RUNNING ?
               "RUNNING" : "STOPPED");

        printf("Messages    : %d\n",
               response.messages_received);

        printf("=====================================\n");

        sleep(2);
    }

    name_close(server_coid);

    return EXIT_SUCCESS;
}
