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

char tle_raw[] = {  "TEST\n"
                    "1 39166U 13023A   17278.73176791 -.00000052 +00000-0 +00000-0 0  9998\n"
                    "2 39166 055.9189 035.8040 0048654 019.8635 340.3273 02.00564007032172"};

int main(void)
{
    GsControl gs;
    /* Object initialization */
    //gs.setMinElevation(5.0);
    gs.setFrequencies(145.25e6, 145.25e6);
    //gs.setTimestep(5);
    gs.setLocation(41.0, 2.11, 200.0);
    gs.setTLE(tle_raw);
    gs.loadParms();
    while (1) {
        gs.handleCommand();
        gs.addNextPassFrom(gs.getActualTime());
        //gs.printPass();
        gs.runSatelliteTracking();
    }
}
