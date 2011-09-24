#include <cmath>

#include "utils.h"

int checkColor(int i)
{
    if (i < 0)
        return 0;
    if (i > 255)
        return 255;
    return i;
}

int check(int x, int l, int b)
{
    if (x < l)
        return l;
    if (x >= b)
        return b - 1;
    return x;
}

double normalDistrib(int x, int y, double sigma)
{
    return (1 / (2 * M_PI * sigma * sigma)) * exp(-double(x * x + y * y) / double(2 * sigma * sigma));
}

bool check2rot(int x, int b, int w)
{
    if (x > w + 1 || x < b - 1)
        return true;
    return false;
}

bool check2scale(int x, int w)
{
    if (x > w - 1 || x < 0)
        return true;
    return false;
}
