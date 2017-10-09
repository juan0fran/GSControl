#include <cstdio>
#include <iostream>
#include <string.h>
#include <stdlib.h>     /* strtof */
#include <math.h>

#include "socket_utils.h"
#include "rotor_control.h"
#include "motor_angles.h"
#include "orbit_simulator.h"
#include "gs_control.h"

/*
auto start = std::chrono::high_resolution_clock::now();
---
auto elapsed = std::chrono::high_resolution_clock::now() - start;
std::cout   << "Milliseconds elapsed: "
            << ((std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count())/1000)
            << " ms" << std::endl;
*/

#if 0
char tle_raw[] = {  "ISS\n"
                    "1 25544U 98067A   17278.53241556  .00004346  00000-0  73008-4 0  9995\n"
                    "2 25544  51.6413 208.2406 0004244 348.0772 162.9907 15.54071733 78914\n"};
#else
char tle_raw[] = {  "TEST\n"
                    "1 39166U 13023A   17278.73176791 -.00000052 +00000-0 +00000-0 0  9998\n"
                    "2 39166 055.9189 035.8040 0048654 019.8635 340.3273 02.00564007032172"};
#endif
int main(void)
{
    GsControl gs;
    /* Object initialization */
    //gs.setMinElevation(5.0);
    gs.setFrequencies(145.25e6, 145.25e6);
    gs.setTimestep(5);
    gs.setLocation(41.0, 2.11, 200.0);
    gs.setTLE(tle_raw);
    gs.loadParms();
    while (1) {
        /* propagate for the next pass from now */
        //gs.clearPasses();
        gs.checkPasses();
        if (gs._passes.size() < 2) {
            gs.addNextPassFromLast();
            gs.addNextPassFromLast();
            gs.checkPasses();
        }
        /* print it */
        std::cout << gs._passes.size() << " passes available" << std::endl;
        //gs.printPass(1);
        /* track the current pass if available */
        gs.runSatelliteTracking();
        /* look for commands during 10 seconds and then propagate&make the stuff again */
        gs.handleCommand(5);
    }
}
