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

#include <libpq-fe.h>

#define TIMESTAMP_T uint32_t

/* GS Commands:
 * - Change Transceiver
 * - Change Polarization
 * - Change GS inputs (reload GsControl object)
 * - Enable-disable rotor tracking
 * - Set manual position (disables rotor tracking by default)
 */

#define RELOAD_GS   1
#define NON_OP      0



class GsControl : Motor_Angles{
    public:
        GsControl();

        void setMinElevation(float el);
        void setFrequencies(float dl, float ul);
        void setTLE(char *path);
        void setTimestep(int times);
        void setLocation(float lat_gs, float lon_gs, float h_gs);

        void loadParms();

        void addNextPassFrom(TIMESTAMP_T t);
        void addNextPassFromLast();

        void clearPasses();
        void checkPasses();

        bool doesPassExist();

        void printPass(int pass_number);

        void waitForPass();
        void runSatelliteTracking();

        int handleCommand();
        int handleCommand(int timeout);

        time_t getActualTime();
        time_t fakeGetActualTime(long long offset);

        void storeInDB();

        PassesVec       _passes;

    private:
        void initializeOrbitObject(OrbitSimulator *orb,
                float minimum_elevation, float dl_frequency, float ul_frequency,
                char *tle_path, int timestep, float lat_gs, float lon_gs, float h_gs);

        OrbitSimulator  *_orb;
        RotorControl    *_rot;

        PGconn          *_pgconn;

        socket_config_t _server_conf;

        server_handler_t _server_fd;
        socket_handler_t _client;

        bool _connected_user;
        bool _rotors_enabled;
        bool _error_in_op;

        int _timestep;
        struct {
            float lat;
            float lon;
            float h;
        }_gs;
        char _tle_path[256];
        float _ul_freq, _dl_freq;
        float _min_el;

};

#endif
