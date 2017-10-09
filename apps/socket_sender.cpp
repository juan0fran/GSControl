#include <cstdio>
#include <iostream>
#include <string.h>
#include <stdlib.h>     /* strtof */
#include <math.h>

#include "socket_utils.h"
#include "gs_control.h"

int main (void)
{
    socket_config_t conf;
    socket_handler_t client;

    disable_enable_rotor_cmd cmd1;
    set_manual_position_cmd cmd2;
    change_transceiver_cmd cmd3;
    change_polarization_cmd cmd4;
    change_op_parameters_cmd cmd5;

    strcpy(conf.client.ip, "localhost");
    conf.client.port = 55001;
    if (client_socket_init(&conf, &client) != SU_NO_ERROR) {
        std::cout << "Server not available at " << conf.client.ip << ":" << conf.client.port << std::endl;
    }

    cmd1.cmd_id = CMD_ID_DISABLE_ENABLE_ROTOR;
    cmd2.cmd_id = CMD_ID_SET_MANUAL_POSITION;
    cmd3.cmd_id = CMD_ID_CHNG_TRX;
    cmd4.cmd_id = CMD_ID_CHNG_POL;
    cmd5.cmd_id = CMD_ID_CHANGE_OP_PARMS;

    /* sending cmd1 */
    cmd1.enable_flag = DISABLE_ROTOR;

    memcpy(client.buffer, &cmd1, sizeof(cmd1));
    client.len = sizeof(cmd1);
    socket_write(&client);
    return 0;
}
