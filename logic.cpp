#include <climits>
#include <cmath>
#include <iostream>

using std::cout;

#include "logic.h"
#include "utils.h"


int ImageLogic::checkWidth(int x)
{
    if (x < 0)
        return 0;
    if (x >= width())
        return width() - 1;
    return x;
}

int ImageLogic::checkHeight(int x)
{
    if (x < 0)
        return 0;
    if (x >= height())
        return height() - 1;
    return x;
}

void ImageLogic::linearCorrection()
{
    int lmax = 0;
    int lmin = INT_MAX;

    for (int x = 0; x < width(); x++) {
        for (int y = 0; y < height(); y++) {
            QRgb p = pixel(x, y);

            int l = 0.2125d * qRed(p) + 0.7154d * qGreen(p) + 0.0721d * qBlue(p);

            if (l > lmax)
                lmax = l;
            if (l < lmin)
                lmin = l;
        }
    }

    for (int x = 0; x < width(); x++) {
        for (int y = 0; y < height(); y++) {
            QRgb p = pixel(x, y);

            int r = checkBound((qRed(p) - lmin) * 255 / (lmax - lmin));
            int g = checkBound((qGreen(p) - lmin) * 255 / (lmax - lmin));
            int b = checkBound((qBlue(p) - lmin) * 255 / (lmax - lmin));

            setPixel(x, y, qRgb(r, g, b));
        }
    }
}

void ImageLogic::channelCorrection()
{
    int rmax, bmax, gmax;
    int rmin, bmin, gmin;
    int r, g, b;

    rmax = bmax = gmax = 0;
    rmin = bmin = gmin = INT_MAX;

    for (int x = 0; x < width(); x++) {
        for (int y = 0; y < height(); y++) {
            QRgb p = pixel(x, y);

            r = qRed(p);
            g = qGreen(p);
            b = qBlue(p);

            if (r > rmax)
                rmax = r;
            if (r < rmin)
                rmin = r;

            if (g > gmax)
                gmax = g;
            if (g < gmin)
                gmin = g;

            if (b > bmax)
                bmax = b;
            if (b < bmin)
                bmin = b;
        }
    }

    for (int x = 0; x < width(); x++) {
        for (int y = 0; y < height(); y++) {
            QRgb p = pixel(x, y);

            int r = checkBound((qRed(p) - rmin) * 255 / (rmax - rmin));
            int g = checkBound((qGreen(p) - gmin) * 255 / (gmax - gmin));
            int b = checkBound((qBlue(p) - bmin) * 255 / (bmax - bmin));

            setPixel(x, y, qRgb(r, g, b));
        }
    }
}

void ImageLogic::gaussianFilter(int sigma)
{
    int size = 6 * sigma - 1;
    int s2 = size / 2;

    int x, y, i, j, k, l;

    double norm = 0.0;

    double **filter;
    filter = new double*[size];
    for (i = 0; i < size; i++) {
        filter[i] = new double[size];
    }

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            filter[i][j] = normalDistrib(j - s2, i - s2, sigma);
            norm += filter[i][j];
        }
    }

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            filter[i][j] /= norm;
        }
    }

    cout << normalDistrib(0, -1, 1) << "\n";
    cout << normalDistrib(0, 0, 1) << "\n";

    for (i = 0; i < size / 2; i++) {
        for (j = 0; j < size; j++) {
            qSwap(filter[i][j], filter[i][size - j - 2]);
        }
    }

    double rsum, gsum, bsum;
    for (x = 0; x < width(); x++) {
        for (y = 0; y < height(); y++) {
            rsum = gsum = bsum = 0.0;
            for (k = 0; k < size; k++) {
                for (l = 0; l < size; l++) {
                    int n = checkWidth(x - k);
                    int m = checkHeight(y - l);
                    QRgb p = pixel(n,m);
                    rsum += filter[l][k] * qRed(p);
                    gsum += filter[l][k] * qGreen(p);
                    bsum += filter[l][k] * qBlue(p);
                }
            }
            setPixel(x, y, qRgb(checkBound(rsum), checkBound(gsum), checkBound(bsum)));
        }
    }
}
