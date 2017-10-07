#include <cstdio>
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>     /* strtof */
#include <math.h>

#include "socket_utils.h"
#include "rotor_control.h"

#define ROTOR_CONTROL_PORT  54001

static bool connected_control = false;
static bool connected_motor = false;

static struct {
    float last_az;
    float last_el;
    float req_az;
    float req_el;
}rot_ctrl;

bool inRange(float num1, float num2, float range)
{
    float res = num1 - num2;
    if (fabs(res) < range) {
        return true;
    }else {
        return false;
    }
}

int rotorsActive(RotorsData *data)
{
    bool aux;
    int ret_az, ret_el;
    if (inRange(data->az, rot_ctrl.req_az, 2.0)) {
        ret_az = 0;
    }else if (inRange(data->az, rot_ctrl.last_az, 2.0)) {
        ret_az = 1;
    }else {
        ret_az = 2;
    }
    if (inRange(data->el, rot_ctrl.req_el, 1.0)) {
        ret_el = 0;
    }else if (inRange(data->el, rot_ctrl.last_el, 1.0)) {
        ret_el = 1;
    }else {
        ret_el = 2;
    }
    return ((ret_az&0x0F) | ((ret_el&0x0F) << 4));
}

void requestRotorStatus(RotorControl &rot, RotorsData *data)
{
    data->mode = ROT_GET;
    if (rot.sendAndReceive(data, sizeof(RotorsData), 250) > 0) {
        connected_motor = true;
    }else {
        connected_motor = false;
    }
}

void setRotorPositionFromMessage(RotorControl &rot, char *buff, int len)
{
    RotorsData set_data;
    RotorsData get_data;
    char az[128], el[128];
    if (len > 0) {
        requestRotorStatus(rot, &get_data);
        if (connected_motor == true) {
            /* calculate how much degrees it will take */
            if (sscanf(buff, "%[^,] %*[,] %[^\n]", az, el) == 2) {
                set_data.mode = ROT_SET;
                set_data.az = strtof(az, NULL);
                set_data.el = strtof(el, NULL);

                rot_ctrl.last_az = get_data.az;
                rot_ctrl.last_el = get_data.el;
                rot_ctrl.req_az = set_data.az;
                rot_ctrl.req_el = set_data.el;

                /* send to other thread */
                if (rot.sendAndReceive(&set_data, sizeof(set_data), 250) > 0) {
                    connected_motor = true;
                }else {
                    connected_motor = false;
                }
            }
        }
    }
}

void run_server()
{
    RotorsData get_data;
    RotorControl rot((char *) "192.168.0.204", 8888);
    socket_config_t conf;
    server_handler_t s;
    socket_handler_t c;
    su_errno_e err;

    conf.server.port = ROTOR_CONTROL_PORT;
    if (server_socket_init(&conf, &s) != SU_NO_ERROR) {
        std::cout << "Problem initializing server at port: " << conf.server.port << std::endl;
        return;
    }
    while (1) {
        if (!connected_control) {
            std::cout << "Waiting for a new client..." << std::endl;
            if (server_socket_new_client(&s, &c) != SU_NO_ERROR) {
                std::cout << "Problem accepting a client..." << std::endl;
            }
            std::cout << "New client arrived!" << std::endl;
            connected_control = true;
        }else {
            /* timeout to look again the motors */
            err = SU_NO_ERROR;
            c.timeout_ms = 2000;
            c.expected_len = 0;
            err = socket_read(&c);
            if (err == SU_IO_ERROR) {
                connected_control = false;
            }else if(err == SU_TIMEOUT) {
                /* timeout waiting for message from upper layers */
                requestRotorStatus(rot, &get_data);
                sprintf((char *) c.buffer, "Rotor status: %s -- %d\n\tPosition: %f,%f\n",
                                        connected_motor ? "Connected" : "Disconnected",
                                        rotorsActive(&get_data),
                                        get_data.az, get_data.el);
                c.len = strlen((char *) c.buffer);
                socket_write(&c);
                /* send back the status */
            }else {
                /* correct message received! */
                /* Process incoming message:
                    - Set position message
                    - Get status message
                    - Stop rotor message
                */
                setRotorPositionFromMessage(rot, (char *) c.buffer, c.len);
            }
        }
    }
}

int main (int argc, char **argv)
{
    run_server();
    return 0;
}
