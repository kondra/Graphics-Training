#include <cmath>
#include <iostream>

using std::cout;

#include "utils.h"

int checkBound(int i)
{
    if (i < 0)
        return 0;
    if (i > 255)
        return 255;
    return i;
}

double normalDistrib(int x, int y, double sigma)
{
    return (1 / (2 * M_PI * sigma * sigma)) * exp(-double(x * x + y * y) / double(2 * sigma * sigma));
}
