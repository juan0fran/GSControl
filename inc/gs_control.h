#ifndef __GS_CONTROL_H__
#define __GS_CONTROL_H__

#include <thread>         // std::this_thread::sleep_until
#include <chrono>         // std::chrono::system_clock
#include <ctime>          // std::time_t, std::tm, std::localtime, std::mktime
#include <fstream>
#include <cmath>
#include <cstdlib>

#include <map>
#include <tuple>

#include "socket_utils.h"
#include "orbit_simulator.h"
#include "rotor_control.h"
#include "switch_control.h"
#include "gs_commands.h"

#include <yaml-cpp/yaml.h>

#if defined(__APPLE__)
#include <libpq-fe.h>
#else
#include <postgresql/libpq-fe.h>
#endif

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

class GsControl {
    public:

        typedef std::tuple<unsigned long, unsigned long> SatPassTuple;
        typedef std::vector<SatPassTuple> SatPassTupleVector;
        typedef std::map<int, SatPassTupleVector> NotificationMap;

        GsControl(double gs_lat, double gs_lon, double gs_h, int timestep, int max_propagations);

        void    get_config(std::string path);

        void    addNewSatellite(double min_el, double dl_f, double ul_f, char *tle);
        NotificationMap getMap();
        void    propagate();

        void    runSatelliteTracking();

        int     handleCommand(int timeout);
        /* 0 means no timeout --> just see if there is something there */
        int     handleCommand() { return (handleCommand(0)); }

        time_t  getActualTime();
        time_t  fakeGetActualTime(long long offset);

        void    sendDoppler(float freq_dl, float freq_ul);

        std::string     printReadble(time_t t);

        void    storeInDB();

        std::vector<OrbitSimulator> propagationVector;

    private:

        OrbitSimulator      initializeOrbitObject(  double min_el,
                                                    double dl_f, double ul_f,
                                                    double gs_lat, double gs_lon, double gs_h,
                                                    char *tle, int sim_timestep, int max_propagations);

        int _max_propagations;
        int _sim_timestep;
        double _gs_lat, _gs_lon, _gs_h;

        RotorControl    *_rot;              /* IP: 192.168.0.204:8888 */
        SwitchControl   *_supply;           /* IP: 192.168.0.20X:8888 */
        SwitchControl   *_pol_selector;     /* IP: 192.168.0.20X:8888 */
        SwitchControl   *_trx_selector;     /* IP: 192.168.0.20X:8888 */

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
