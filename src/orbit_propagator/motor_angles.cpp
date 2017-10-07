#include "motor_angles.h"

Motor_Angles::Motor_Angles(){}

void Motor_Angles::clear()
{
    /* clear Angles vector */
    _Angles.clear();
}

void Motor_Angles::computeMotorAngle(SimulatorResultsVec Results)
{
    clear();
    NewAngles single_angle;
    if( ((Results[0].propagation.az < Results[1].propagation.az) &&
         (Results[0].propagation.az > Results[Results.size()-1].propagation.az)) ||
        ((Results[0].propagation.az > Results[1].propagation.az) &&
         (Results[0].propagation.az < Results[Results.size()-1].propagation.az))) {
             
        for (SimulatorResultsVec::iterator it = Results.begin(); it != Results.end(); it++) {
            if(it->propagation.az > 180.0f) {
                single_angle.motor_az = it->propagation.az - 180.0f;
            }else {
                single_angle.motor_az = it->propagation.az + 180.0f;
            }
            single_angle.propagation = it->propagation;
            single_angle.motor_el = 180.0f - it->propagation.el;
            _Angles.push_back(single_angle);
        }
    }else{
        for (SimulatorResultsVec::iterator it = Results.begin(); it != Results.end(); it++) {
            single_angle.propagation = it->propagation;
            single_angle.motor_az = it->propagation.az;
            single_angle.motor_el = it->propagation.el;
            _Angles.push_back(single_angle);
        }
    }
}
