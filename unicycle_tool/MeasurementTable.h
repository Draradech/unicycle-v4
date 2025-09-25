#pragma once

#include <string>

typedef struct
{
    unsigned char id;
    std::string unit;
    std::string name;
    float minvalue;
    float maxvalue;
} measurement_t;

typedef struct
{
    unsigned char id;
    float increment;
    std::string unit;
    std::string name;
} parameter_t;

extern measurement_t measurementTable[];
extern parameter_t parameterTable[];
