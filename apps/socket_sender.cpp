#include <cstdio>
#include <iostream>
#include <string.h>
#include <stdlib.h>     /* strtof */
#include <math.h>

#include "socket_utils.h"
#include "gs_control.h"

#if 1
char tle_raw[] = {  "ISS\n"
                    "1 25544U 98067A   17278.53241556  .00004346  00000-0  73008-4 0  9995\n"
                    "2 25544  51.6413 208.2406 0004244 348.0772 162.9907 15.54071733 78914\n"};
#else
char tle_raw[] = {  "TEST\n"
                    "1 39166U 13023A   17278.73176791 -.00000052 +00000-0 +00000-0 0  9998\n"
                    "2 39166 055.9189 035.8040 0048654 019.8635 340.3273 02.00564007032172"};
#endif

int main (void)
{
    socket_config_t conf;
    socket_handler_t client;

    /* cmd initialization */
    disable_enable_rotor_cmd cmd1;
    memset(&cmd1, 0, sizeof(cmd1));
    cmd1.cmd_id = CMD_ID_DISABLE_ENABLE_ROTOR;

    set_manual_position_cmd cmd2;
    memset(&cmd2, 0, sizeof(cmd2));
    cmd2.cmd_id = CMD_ID_SET_MANUAL_POSITION;

    change_transceiver_cmd cmd3;
    memset(&cmd3, 0, sizeof(cmd3));
    cmd3.cmd_id = CMD_ID_CHNG_TRX;

    change_polarization_cmd cmd4;
    memset(&cmd4, 0, sizeof(cmd4));
    cmd4.cmd_id = CMD_ID_CHNG_POL;

    change_op_parameters_cmd cmd5;
    memset(&cmd5, 0, sizeof(cmd5));
    cmd5.cmd_id = CMD_ID_CHANGE_OP_PARMS;

    /* socket initialization */
    strcpy(conf.client.ip, "localhost");
    conf.client.port = 55001;
    if (client_socket_init(&conf, &client) != SU_NO_ERROR) {
        std::cout << "Server not available at " << conf.client.ip << ":" << conf.client.port << std::endl;
    }

    /* sending cmd1 */
    /* seting shit */
    cmd1.enable_flag = DISABLE_ROTOR;
    cmd2.az = 120.0;
    cmd2.el = 12.0;

    strcpy(cmd5.filepath, "test.conf");
    memcpy(client.buffer, &cmd5, sizeof(cmd5));
    client.len = sizeof(cmd5);
    socket_write(&client);

    close(client.fd);
    return 0;
}
