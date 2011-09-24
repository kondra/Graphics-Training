#ifndef UTILS_H
#define UTILS_H

#define M_PI 3.14159265358979323846

const double eps = 0.0000001;

int checkColor(int i);
int check(int x, int l, int b);
bool check2rot(int x, int b, int w);
bool check2scale(int x, int w);
double normalDistrib(int x, int y, double sigma);

#endif // UTILS_H
