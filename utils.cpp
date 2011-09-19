#include <cmath>

#include "utils.h"

int checkBound(int i)
{
    if (i < 0)
        return 0;
    if (i > 255)
        return 255;
    return i;
}

int check(int x, int b)
{
    if (x < 0)
        return 0;
    if (x >= b)
        return b - 1;
    return x;
}

double normalDistrib(int x, int y, double sigma)
{
    return (1 / (2 * M_PI * sigma * sigma)) * exp(-double(x * x + y * y) / double(2 * sigma * sigma));
}

int search(int *a, int k, int l, int r)
{
    int s, m, i=l, j=r, tmp;
    if (l == r) return a[r];
    s = (l + r) / 2;
    m = a[s];
    while (i < j)
    {
        while (a[i] < m) i++;
        while (a[j] > m) j--;
        if (i < j)
        {
            tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
            i++; j--;
        }
    }
    if (k <= j) return search(a, k, l, j);
    else return search(a, k, j+1, r);
}
