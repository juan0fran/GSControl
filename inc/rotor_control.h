#ifndef __ROTOR_CONTROL_H__
#define __ROTOR_CONTROL_H__

#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

typedef enum rotor_mode_e{
    ROT_STOP = 0,
    ROT_SET = 1,
    ROT_GET = 2,
}rotor_mode_e;

typedef struct __attribute__ ((packed)) RotorsData {
    uint8_t mode;
    float   az;
    float   el;
}RotorsData;

class RotorControl {
    private:
        struct sockaddr_in
                    _server;
        int         _socket_fd;

        RotorsData  _data;

        void        init_socket(RotorControl *sc);
        int         blockingRead(int timeout_ms);

        bool        connected_motor;

    public:
        RotorControl();
        RotorControl(char *ip, int port);

        void        setIP(char *ip);
        void        setPort(int port);
        void        setIP_Port(char *ip, int port);

        RotorsData  getStatus();

        int         sendAndReceive(void *p, size_t len);
        int         sendAndReceive(void *p, size_t len, int ms_timeout);

        void        setRotorPosition(float az, float el);
        void        requestRotorStatus(RotorsData *data);
};

#endif
