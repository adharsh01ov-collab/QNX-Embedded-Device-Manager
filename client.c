/*
 * QNX Embedded Device Manager - Interactive Client
 *
 * Menu-driven client that sends commands to the device manager server
 * using MsgSend().
 *
 * Build (QNX): qcc -o client client.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void show_status(response_t *response)
{
    printf("\n========== DEVICE STATUS ==========\n");

    printf("Temperature : %d C\n",
           response->temperature);

    printf("Speed       : %d\n",
           response->speed);

    printf("RPM         : %d\n",
           response->rpm);

    printf("State       : %s\n",
           response->device_state ==
           DEVICE_RUNNING ? "RUNNING" : "STOPPED");

    printf("Messages    : %d\n",
           response->messages_received);

    printf("Message     : %s\n",
           response->message);

    printf("===================================\n");
}

int main(void)
{
    int server_coid;

    request_t request;
    response_t response;

    int choice;

    server_coid =
        name_open(SERVER_NAME, 0);

    if(server_coid == -1)
    {
        perror("Unable to connect to server");
        return EXIT_FAILURE;
    }

    printf("Connected to QNX Device Manager.\n");

    while(1)
    {
        printf("\n");
        printf("========== MENU ==========\n");
        printf("1. Get Status\n");
        printf("2. Set Speed\n");
        printf("3. Get Statistics\n");
        printf("4. Start Device\n");
        printf("5. Stop Device\n");
        printf("6. Reset Device\n");
        printf("7. Shutdown Server\n");
        printf("8. Exit Client\n");
        printf("==========================\n");

        printf("Enter choice: ");

        if(scanf("%d", &choice) != 1)
        {
            /* Non-numeric input or EOF: stop instead of looping forever */
            printf("\nInvalid input. Exiting.\n");
            break;
        }

        memset(&request, 0, sizeof(request));
        memset(&response, 0, sizeof(response));

        switch(choice)
        {
            case 1:

                request.command =
                    CMD_GET_STATUS;

                break;


            case 2:

                request.command =
                    CMD_SET_SPEED;

                printf("Enter speed (0-200): ");
                scanf("%d", &request.value);

                break;


            case 3:

                request.command =
                    CMD_GET_STATS;

                break;


            case 4:

                request.command =
                    CMD_START;

                break;


            case 5:

                request.command =
                    CMD_STOP;

                break;


            case 6:

                request.command =
                    CMD_RESET;

                break;


            case 7:

                request.command =
                    CMD_SHUTDOWN;

                break;


            case 8:

                name_close(server_coid);

                printf("Client exiting.\n");

                return EXIT_SUCCESS;


            default:

                printf("Invalid choice.\n");

                continue;
        }

        if(MsgSend(server_coid,
                   &request,
                   sizeof(request),
                   &response,
                   sizeof(response)) == -1)
        {
            perror("MsgSend failed");
            break;
        }

        printf("\nServer Response: %s\n",
               response.message);

        if(choice == 1)
        {
            show_status(&response);
        }

        if(choice == 3)
        {
            printf("Messages received by server: %d\n",
                   response.messages_received);
        }

        if(choice == 7)
        {
            break;
        }
    }

    name_close(server_coid);

    return EXIT_SUCCESS;
}
