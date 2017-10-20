#ifndef __SWITCH_CONTROL_H__
#define __SWITCH_CONTROL_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum ERRNO_CODE_SW {
    NO_ERROR,
    ERROR,
}ERRNO_CODE_SW;

typedef enum GSSW_MODE {
    GSSW_MODE_GET = 0,
    GSSW_MODE_SET = 1,
    GSSW_MODE_LCD = 2,
}GSSW_MODE;

typedef enum GSSW_OUT {
    GSSW_OUT_DISABLED = 0,
    GSSW_OUT_ENABLED = 1,
}GSSW_OUT;


typedef struct __attribute__ ((packed)) GenericData {
    uint8_t mode;
    struct __attribute__ ((packed)) {
        uint16_t    temp;
        uint16_t    analog[3];
        uint8_t     manual;
        uint8_t     channel[8];
        uint8_t     fan;
    }get;
    struct __attribute__ ((packed)) {
        uint8_t     channel[8];
        uint8_t     fan;
        char        line1[17];
        char        line2[17];
    }set;
}GenericData;

class SwitchControl {
    private:
        struct sockaddr_in
                    _server;
        int         _socket_fd;

        GenericData _lcd_data;
        GenericData _set_data;
        GenericData _get_data;

        void        init_socket(SwitchControl *sc);

        int         sendAndReceive(void *p, size_t len);
        int         blockingRead(int timeout_ms);

        void        setOutput(uint8_t output, GSSW_OUT state);
        void        setFan(GSSW_OUT state);

        ERRNO_CODE_SW  set();
        ERRNO_CODE_SW  get();

    public:
        SwitchControl();
        SwitchControl(char *ip, int port);

        void        setIP(char *ip);
        void        setPort(int port);
        void        setIP_Port(char *ip, int port);

        void        executeQuery();

        void        enableOutput(uint8_t output);
        void        disableOutput(uint8_t output);

        void        enableFan();
        void        disableFan();

        GenericData getStatus();

        void        setLCD(const char *line1, const char *line2);

        void        printContents(GenericData *data);
        void        printContents();

};

#endif
