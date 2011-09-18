#include <climits>
#include <cmath>
#include <iostream>

using std::cout;

#include "logic.h"
#include "utils.h"

int ImageLogic::check(int x, int b)
{
    if (x < 0)
        return 0;
    if (x >= b)
        return b - 1;
    return x;
}

void ImageLogic::linearCorrection()
{
    int lmax = 0;
    int lmin = INT_MAX;
    int l;

    for (int x = 0; x < width(); x++) {
        for (int y = 0; y < height(); y++) {
            QRgb p = pixel(x, y);

            l = 0.2125d * qRed(p) + 0.7154d * qGreen(p) + 0.0721d * qBlue(p);

            if (l > lmax) lmax = l;
            if (l < lmin) lmin = l;
        }
    }

    for (int x = 0; x < width(); x++) {
        for (int y = 0; y < height(); y++) {
            QRgb p = pixel(x, y);

            int r = checkBound((qRed(p) - lmin) * 255 / (lmax - lmin));
            int g = checkBound((qGreen(p) - lmin) * 255 / (lmax - lmin));
            int b = checkBound((qBlue(p) - lmin) * 255 / (lmax - lmin));

            l = 0.2125d * qRed(p) + 0.7154d * qGreen(p) + 0.0721d * qBlue(p);
            if (l < lmin + 1)
                r = g = b = 0;
            if (l > lmax - 1)
                r = g = b = 255;

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

            if (r > rmax) rmax = r;
            if (r < rmin) rmin = r;

            if (g > gmax) gmax = g;
            if (g < gmin) gmin = g;

            if (b > bmax) bmax = b;
            if (b < bmin) bmin = b;
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

void ImageLogic::gaussianFilter(double sigma)
{
    int size = 6.0 * sigma;
    if (!size % 2)
        size--;
    int s2 = size / 2;

    if (size == 0)
        return;

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

    for (i = 0; i < size / 2; i++) {
        for (j = 0; j < size; j++) {
            qSwap(filter[i][j], filter[i][size - i - 2]);
        }
    }

    int n, m;
    double rsum, gsum, bsum;
    QRgb p;

    QImage prev = *static_cast<QImage*>(this);
    for (x = 0; x < prev.width(); x++) {
        for (y = 0; y < prev.height(); y++) {
            rsum = gsum = bsum = 0.0;
            for (k = 0; k < size; k++) {
                for (l = 0; l < size; l++) {
                    n = check(x - (l - size / 2), prev.width());
                    m = check(y - (k - size / 2), prev.height());
                    p = prev.pixel(n,m);
                    rsum += filter[l][k] * qRed(p);
                    gsum += filter[l][k] * qGreen(p);
                    bsum += filter[l][k] * qBlue(p);
                }
            }
            setPixel(x, y, qRgb(checkBound(rsum), checkBound(gsum), checkBound(bsum)));
        }
    }

    for (i = 0; i < size; i++) {
        delete [] filter[i];
    }
    delete [] filter;
}
