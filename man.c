/*
 * Source code for the manager.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "man.h"
#include "net.h"
#include "host.h"

#define MAXBUFFER 1000
#define PIPE_WRITE 1
#define PIPE_READ  0
#define TENMILLISEC 10000
#define DELAY_FOR_HOST_REPLY 10  /* Delay in ten of milliseconds */

void display_host(struct man_port_at_man *list, struct man_port_at_man *curr_host);

void change_host(struct man_port_at_man *list, struct man_port_at_man **curr_host);

void display_host(struct man_port_at_man *list, struct man_port_at_man *curr_host);

void display_host_state(struct man_port_at_man *curr_host);

void set_host_dir(struct man_port_at_man *curr_host);

char man_get_user_cmd(int curr_host);


/* Get the user command */
char man_get_user_cmd(int curr_host) {
    char cmd;

    while (1) {
        /* Display command options */
        printf("\nCommands (Current host ID = %d):\n", curr_host);
        printf("   (s) Display host's state\n");
        printf("   (m) Set host's main directory\n");
        printf("   (h) Display all hosts\n");
        printf("   (c) Change host\n");
        printf("   (p) Ping a host\n");
        printf("   (u) Upload a file to a host\n");
        printf("   (d) Download a file from a host\n");
        printf("   (r) Register a new Domain name with the Domain Name Server\n");
        printf("   (l) Lookup a host with their Domain Name\n");
        printf("   (P) Ping a host with their Domain Name\n");
        printf("   (D) Download from a host by giving its Domain Name\n");
        printf("   (q) Quit\n");
        printf("   Enter Command: ");
        do {
            cmd = getchar();
        } while (cmd == ' ' || cmd == '\n'); /* get rid of junk from stdin */

/* Ensure that the command is valid */
        switch (cmd) {
            case 's':
            case 'm':
            case 'h':
            case 'c':
            case 'p':
            case 'u':
            case 'd':
            case 'q':
            case 'r':
            case 'l':
            case 'P':
            case 'D':
                return cmd;
            default:
                printf("Invalid: you entered %c\n\n", cmd);
        }
    }
}

/* Change the current host */
void change_host(struct man_port_at_man *list, struct man_port_at_man **curr_host) {
    int new_host_id;

// display_host(list, *curr_host);
    printf("Enter new host: ");
    scanf("%d", &new_host_id);
    printf("\n");

/* Find the port of the new host, and then set it as the curr_host */
    struct man_port_at_man *p;
    for (p = list; p != NULL; p = p->next) {
        if (p->host_id == new_host_id) {
            *curr_host = p;
            break;
        }
    }
}

/* Display the hosts on the console */
void display_host(struct man_port_at_man *list, struct man_port_at_man *curr_host) {
    struct man_port_at_man *p;

    printf("\nHost list:\n");
    for (p = list; p != NULL; p = p->next) {
        printf("   Host id = %d ", p->host_id);
        if (p->host_id == curr_host->host_id) {
            printf("(<- connected)");
        }
        printf("\n");
    }
}

/*
 * Send command to the host for it's state.  The command
 * is a single character 's'
 *
 * Wait for reply from host, which should be the host's state.
 * Then display on the console.
 */
void display_host_state(struct man_port_at_man *curr_host) {
    char msg[MAN_MSG_LENGTH];
    char reply[MAN_MSG_LENGTH];
    char dir[NAME_LENGTH];
    int host_id;
    int n;

    msg[0] = 's';
    write(curr_host->send_fd, msg, 1);

    n = 0;
    while (n <= 0) {
        usleep(TENMILLISEC);
        n = read(curr_host->recv_fd, reply, MAN_MSG_LENGTH);
    }
    reply[n] = '\0';
    sscanf(reply, "%s %d", dir, &host_id);
    printf("Host %d state: \n", host_id);
    printf("    Directory = %s\n", dir);
}


void set_host_dir(struct man_port_at_man *curr_host) {
    char name[NAME_LENGTH];
    char msg[NAME_LENGTH];
    int n;

    printf("Enter directory name: ");
    scanf("%s", name);
    n = sprintf(msg, "m %s", name);
    write(curr_host->send_fd, msg, n);
}

/*
 * Command host to send a ping to the host with id "curr_host"
 *
 * User is queried for the id of the host to ping.
 *
 * A command message is sent to the current host.
 *    The message starrts with 'p' followed by the id
 *    of the host to ping.
 *
 * Wait for a reply
 */

void ping(struct man_port_at_man *curr_host) {
    char msg[MAN_MSG_LENGTH];
    char reply[MAN_MSG_LENGTH];
    int host_to_ping;
    int n;

    printf("Enter id of host to ping: ");
    scanf("%d", &host_to_ping);
    n = sprintf(msg, "p %d", host_to_ping);

    write(curr_host->send_fd, msg, n);

    n = 0;
    while (n <= 0) {
        usleep(TENMILLISEC);
        n = read(curr_host->recv_fd, reply, MAN_MSG_LENGTH);
    }
    reply[n] = '\0';
    printf("%s\n", reply);
}


/*
 * Command host to send a file to another host.
 *
 * User is queried for the
 *    - name of the file to transfer;
 *        the file is in the current directory 'dir'
 *    - id of the host to ping.
 *
 * A command message is sent to the current host.
 *    The message starrts with 'u' followed by the
 *    -  id of the destination host
 *    -  name of file to transfer
 */
int file_upload(struct man_port_at_man *curr_host) {
    int n;
    int host_id;
    char name[NAME_LENGTH];
    char msg[NAME_LENGTH];

    printf("Enter file name to upload: ");
    scanf("%s", name);
    printf("Enter host id of destination:  ");
    scanf("%d", &host_id);
    printf("\n");

    n = sprintf(msg, "u %d %s", host_id, name);
    write(curr_host->send_fd, msg, n);
    usleep(TENMILLISEC);
}


int file_download(struct man_port_at_man *curr_host) {
    int n;
    int host_id;
    char name[NAME_LENGTH];
    char msg[NAME_LENGTH];

    printf("Enter file name to download: ");
    scanf("%s", name);
    printf("Enter host id of file location:  ");
    scanf("%d", &host_id);
    printf("\n");

    n = sprintf(msg, "d %d %s", host_id, name);
    write(curr_host->send_fd, msg, n);
    usleep(TENMILLISEC);
}

void dns_register(struct man_port_at_man *curr_host) {
    int n;
    char domainName[NAME_LENGTH];
    char msg[NAME_LENGTH];
    char reply[MAN_MSG_LENGTH];

    printf("Enter name to register with DNS: ");
    scanf("%s", domainName);
    printf("\n");

    n = sprintf(msg, "r %s", domainName);
    write(curr_host->send_fd, msg, n);
    n = 0;
    while (n <= 0) {
        usleep(TENMILLISEC);
        n = read(curr_host->recv_fd, reply, MAN_MSG_LENGTH);
    }
    reply[n] = '\0';
    printf("%s\n", reply);
}

void dns_lookup(struct man_port_at_man *curr_host) {
    int n;
    char domainName[NAME_LENGTH];
    char msg[MAN_MSG_LENGTH];
    char reply[MAN_MSG_LENGTH];

    printf("Enter name to lookup with DNS: ");
    scanf("%s", domainName);
    printf("\n");

    n = sprintf(msg, "l %s", domainName);
    write(curr_host->send_fd, msg, n);

    n = 0;
    while (n <= 0) {
        usleep(TENMILLISEC);
        n = read(curr_host->recv_fd, reply, MAN_MSG_LENGTH);
    }
    reply[n] = '\0';
    printf("%s\n", reply);
}

void dns_ping(struct man_port_at_man *curr_host) {
    int n;
    char domainName[NAME_LENGTH];
    char msg[MAN_MSG_LENGTH];
    char reply[MAN_MSG_LENGTH];

    printf("Enter name to lookup with DNS: ");
    scanf("%s", domainName);
    printf("\n");

    n = sprintf(msg, "P %s", domainName);
    write(curr_host->send_fd, msg, n);

    n = 0;
    while (n <= 0) {
        usleep(TENMILLISEC);
        n = read(curr_host->recv_fd, reply, MAN_MSG_LENGTH);
    }
    reply[n] = '\0';
    printf("%s\n", reply);
}

// Format user input when requesting a download with DNS server
int dns_file_download(struct man_port_at_man *curr_host) {
    int n;
    char fileName[NAME_LENGTH];
    char msg[NAME_LENGTH];
    char domainName[NAME_LENGTH];

    printf("Enter file name to download: ");
    scanf("%s", fileName);
    printf("Enter domain name of file location:  ");
    scanf("%s", domainName);
    printf("\n");

    n = sprintf(msg, "D %s %s", domainName, fileName);
    write(curr_host->send_fd, msg, n);
    usleep(TENMILLISEC);
}


/*****************************
 * Main loop of the manager  *
 *****************************/
void man_main() {

// State
    struct man_port_at_man *host_list;
    struct man_port_at_man *curr_host = NULL;

    host_list = net_get_man_ports_at_man_list();
    curr_host = host_list;

    char cmd;          /* Command entered by user */

    while (1) {
        /* Get a command from the user */
        cmd = man_get_user_cmd(curr_host->host_id);

        /* Execute the command */
        switch (cmd) {
            case 's': /* Display the current host's state */
                display_host_state(curr_host);
                break;
            case 'm': /* Set host directory */
                set_host_dir(curr_host);
                break;
            case 'h': /* Display all hosts connected to manager */
                display_host(host_list, curr_host);
                break;
            case 'c': /* Change the current host */
                change_host(host_list, &curr_host);
                break;
            case 'p': /* Ping a host from the current host */
                ping(curr_host);
                break;
            case 'u': /* Upload a file from the current host
                             to another host */
                file_upload(curr_host);
                break;
            case 'd': /* Download a file from a host */
                file_download(curr_host);
                break;
            case 'r': // Register with a domain name
                dns_register(curr_host);
                break;
            case 'l': // Lookup a host id with a domain name
                dns_lookup(curr_host);
                break;
            case 'P': // ping a host with a domain name
                dns_ping(curr_host);
                break;
            case 'D': // Download from host by giving domain name
                dns_file_download(curr_host);
            case 'q':  /* Quit */
                return;
            default:
                printf("\nInvalid, you entered %c\n\n", cmd);
        }
    }
}


