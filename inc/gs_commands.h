#include <stdint.h>

#define CMD_ID_DISABLE_ENABLE_ROTOR     0
#define CMD_ID_SET_MANUAL_POSITION      1
#define CMD_ID_CHNG_TRX                 2
#define CMD_ID_CHNG_POL                 3
#define CMD_ID_CHANGE_OP_PARMS          4

#define ENABLE_ROTOR                    1
#define DISABLE_ROTOR                   0

typedef struct __attribute__ ((__packed__)) disable_enable_rotor_cmd {
    uint8_t cmd_id;
    /* just to identify which command I am */
    uint8_t enable_flag;
}disable_enable_rotor_cmd;

typedef struct __attribute__ ((__packed__)) set_manual_position_cmd {
    uint8_t cmd_id;
    /* just to identify which command I am */
    float position_az;
    float position_el;
}set_manual_position_cmd;

typedef struct __attribute__ ((__packed__)) change_transceiver_cmd {
    uint8_t cmd_id;
    /* just to identify which command I am */
    uint8_t transceiver_selection;
}change_transceiver_cmd;

typedef struct __attribute__ ((__packed__)) change_polarization_cmd {
    uint8_t cmd_id;
    /* just to identify which command I am */
    uint8_t polarization_select;
}change_polarization_cmd;

typedef struct __attribute__ ((__packed__)) change_op_parameters_cmd {
    uint8_t cmd_id;
    /* just to identify which command I am */
    char    full_tle_file[256];
    float   minimum_elevation;
    float   ul_frequency;
    float   dl_frequency;
    int     simulation_timestep;
    struct {
        float lat;
        float lon;
        float h;
    }gs;
}change_op_parameters_cmd;
