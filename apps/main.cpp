#include <cstdio>
#include <iostream>
#include <string>
#include <string.h>

#include "socket_utils.h"

void print_bad_sintax()
{
    std::cout << "Bad sintax. Use: ./a.out X args" << std::endl;
    std::cout << "\tWhere X is s or c (server or client)" << std::endl;
    std::cout << "\tAnd args is:" << std::endl;
    std::cout << "\t\tport for server (as 8888)" << std::endl;
    std::cout << "\t\thost:port for client (as localhost:8888)" << std::endl;
}

void recursive_accept_new_client_and_work(server_handler_t *s)
{
    socket_handler_t c;
    while (1) {
        std::cout << "Waiting for a new client..." << std::endl;
        if (server_socket_new_client(s, &c) != SU_NO_ERROR) {
            std::cout << "Problem accepting a client..." << std::endl;
        }
        std::cout << "New client arrived!" << std::endl;
        /* Receive shit and print it */
        c.timeout_ms = 0;
        c.expected_len = 0;
        while (socket_read(&c) != SU_IO_ERROR) {
            if (c.len > 0) {
                std::cout << "Received message:. Size: " << c.len << std::endl << "Contents: " << c.buffer << std::endl;
            }
        }
    }
}

void run_server(char *args)
{
    socket_config_t conf;
    server_handler_t s;

    int port = atoi(args);
    conf.server.port = port;
    if (server_socket_init(&conf, &s) != SU_NO_ERROR) {
        std::cout << "Problem initializing server at port: " << conf.server.port << std::endl;
        print_bad_sintax();
        return;
    }
    recursive_accept_new_client_and_work(&s);
}

void recursive_connect_and_work(socket_config_t *conf)
{
    socket_handler_t h;
    su_errno_e err;
    while(1) {
        if (client_socket_init(conf, &h) == SU_NO_ERROR) {
            /* we are connected here, start sending shit */
            strcpy((char *) h.buffer, "Hi world!");
            h.len = strlen((char *) h.buffer);
            err = SU_NO_ERROR;
            while (err == SU_NO_ERROR) {
                sleep(1);
                err = socket_write(&h);
                std::cout << "Message sent: " << err << std::endl;
            }
            std::cout << "Server disconnected" << std::endl;
        }else {
            sleep(1);
            std::cout << "Trying to (re)connect..." << std::endl;
        }
    }
}

void run_client(char *args)
{
    socket_config_t conf;
    char host[128], cport[128];
    if (sscanf(args, "%[^:] %*[:] %[^\n]", host, cport) == 2) {
        strcpy(conf.client.ip, host);
        conf.client.port = atoi(cport);
        recursive_connect_and_work(&conf);
    }else {
        std::cout << "Read input string: " << args << std::endl;
        print_bad_sintax();
    }
}


int main(int argc, char **argv)
{
    int port;
    if (argc == 3) {
        /* In this case, check that X is s */
        if (argv[1][0] == 's' || argv[1][0] == 'S') {
            std::cout << "Running server..." << std::endl;
            run_server(argv[2]);
        }else if (argv[1][0] == 'c' || argv[1][0] == 'C') {
            std::cout << "Running client..." << std::endl;
            run_client(argv[2]);
        }else {
            print_bad_sintax();
            return 1;
        }
    }else {
        print_bad_sintax();
        return 1;
    }
    return 0;
}
