#ifndef INFLUX_H
#define INFLUX_H

#include <stdint.h>

void formatSmlForInflux(char* influxString,
                        uint16_t stringLength,
                        double value180,
                        double value280,
                        double voltageL1,
                        double voltageL2,
                        double voltageL3,
                        double sumActiveInstantaneousPowerTotal,
                        double sumActiveInstantaneousPowerL1,
                        double sumActiveInstantaneousPowerL2,
                        double sumActiveInstantaneousPowerL3 );

#endif