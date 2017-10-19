#ifndef __GS_CONTROL_H__
#define __GS_CONTROL_H__

#include <thread>         // std::this_thread::sleep_until
#include <chrono>         // std::chrono::system_clock
#include <ctime>          // std::time_t, std::tm, std::localtime, std::mktime

#include "socket_utils.h"
#include "orbit_simulator.h"
#include "motor_angles.h"
#include "rotor_control.h"
#include "gs_commands.h"

#include <yaml-cpp/yaml.h>

#include <libpq-fe.h>

#define TIMESTAMP_T                 uint32_t

#define TLE_KEY         "tle"
#define UL_FREQ_KEY     "ul"
#define DL_FREQ_KEY     "dl"
#define GS_KEY          "gs"
#define MIN_EL_KEY      "min_el"
#define PRE_PROP_KEY    "pre_prop"
#define TIMESTEP_KEY    "timestep"
#define PROP_AMOUNT     "amount_prop"

/* GS Commands:
 * - Change Transceiver
 * - Change Polarization
 * - Change GS inputs (reload GsControl object)
 * - Enable-disable rotor tracking
 * - Set manual position (disables rotor tracking by default)
 */

#define RELOAD_GS   1
#define NON_OP      0

class GsControl : Motor_Angles {
    public:
        GsControl();

        void    get_config(std::string path);

        void    setMinElevation(float el);
        void    setFrequencies(float dl, float ul);
        void    setTLE(char *path);
        void    setTLE(std::string path);
        void    setTimestep(int times);
        void    setLocation(float lat_gs, float lon_gs, float h_gs);
        void    setMaxPropagations(int max);
        void    setTimePointingOffset(time_t offset);

        void    loadParms();

        void    addNextPassFrom(TIMESTAMP_T t);
        void    addNextPassFromLast();

        void    clearPasses();
        void    checkPasses();

        bool    doesPassExist();

        void    printPass(int pass_number);

        void    waitForPass();
        void    runSatelliteTracking();

        int     handleCommand();
        int     handleCommand(int timeout);

        bool    isPassesFull();

        time_t  getActualTime();
        time_t  fakeGetActualTime(long long offset);

        void    sendDoppler(float freq_dl, float freq_ul);

        std::string     printReadble(time_t t);

        void    storeInDB();

        PassesVec       _passes;

    private:

        void initializeOrbitObject(OrbitSimulator *orb);

        struct op_conf_s{
            int     _timestep;
            char    _tle_string[256];
            float   _ul_freq, _dl_freq;
            float   _min_el;
            time_t  _pre_propagation_offset_seconds;
            int     _max_propagations;
            struct {
                float lat;
                float lon;
                float h;
            }_gs;
        }op_conf_s;

        OrbitSimulator  *_orb;
        RotorControl    *_rot;

        PGconn          *_pgconn;

        int _doppler_port_ul;
        int _doppler_port_dl;

        socket_config_t _server_conf;

        server_handler_t _server_fd;
        socket_handler_t _client;

        bool _connected_user;
        bool _rotors_enabled;
        bool _error_in_op;
};

#endif
