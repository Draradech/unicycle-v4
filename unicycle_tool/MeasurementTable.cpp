#include <string>
#include "MeasurementTable.h"

#define ENTRY(ID, var, unit, name, minvalue, maxvalue) \
{(ID), (unit), (name), (minvalue), (maxvalue)},

measurement_t measurementTable[] =
{
    #include "../unicycle_firmware/mtable.h"
};

#define ENTRY(ID, var, increment, unit, name) \
{(ID), (increment), (unit), (name)},

parameter_t parameterTable[] =
{
    #include "../unicycle_firmware/ptable.h"
};
