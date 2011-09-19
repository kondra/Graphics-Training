#include <climits>
#include <cmath>

#include <iostream>
#include <algorithm>
#include <vector>

using std::cout;
using std::sort;
using std::vector;

#include "logic.h"
#include "utils.h"

Kernel::Kernel(int width, int height) : width(width), height(height)
{
    kernel = new double*[height];
    for (int i = 0; i < height; i++) {
        kernel[i] = new double[width];
    }
}

Kernel::~Kernel()
{
    for (int i = 0; i < height; i++) {
        delete [] kernel[i];
    }
    delete [] kernel;
}

Kernel::Kernel(const Kernel& ker)
{
    height = ker.height;
    width = ker.width;
    kernel = new double*[height];
    for (int i = 0; i < height; i++) {
        kernel[i] = new double[width];
        for (int j = 0; j < width; j++) {
            kernel[i][j] = ker.kernel[i][j];
        }
    }
}

Kernel& Kernel::operator=(const Kernel& ker)
{
    if (this == &ker)
        return *this;

    for (int i = 0; i < height; i++) {
        delete [] kernel[i];
    }
    delete [] kernel;

    height = ker.height;
    width = ker.width;
    kernel = new double*[height];
    for (int i = 0; i < height; i++) {
        kernel[i] = new double[width];
        for (int j = 0; j < width; j++) {
            kernel[i][j] = ker.kernel[i][j];
        }
    }
    return *this;
}

Kernel operator*(double alpha, const Kernel& ker)
{
    Kernel tmp(ker);
    for (int i = 0; i < ker.height; i++) {
        for (int j = 0; j < ker.width; j++) {
            tmp.kernel[i][j] *= alpha;
        }
    }
    return tmp;
}

Kernel& Kernel::operator-(const Kernel& ker) {
    *this = *this + (-1.) * ker;
    return *this;
}

Kernel& Kernel::operator+(const Kernel& ker) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (i < ker.height && j < ker.width)
                kernel[i][j] += ker.kernel[i][j];
        }
    }
    return *this;
}

void Kernel::reverse()
{
    for (int i = 0; i < height / 2; i++) {
        for (int j = 0; j < width; j++) {
            qSwap(kernel[i][j], kernel[height - i - 1][j]);
        }
    }
}

void Kernel::normalize()
{
    double sum = 0.0;
    int i, j;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            sum += kernel[i][j];
        }
    }
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            kernel[i][j] /= sum;
        }
    }
}

Kernel Kernel::id(int size)
{
    Kernel tmp(size, size);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            tmp.kernel[i][j] = 0;
        }
    }
    tmp.kernel[size / 2][size / 2] = 1;
    return tmp;
}

inline int ImageLogic::check(int x, int b)
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
            if (l == lmin)
                r = g = b = 0;
            if (l == lmax)
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

void ImageLogic::convolution(Kernel& ker)
{
    int x, y, k, l, n, m;
    double rsum, gsum, bsum;
    QRgb p;

    ker.reverse();

    QImage original = *static_cast<QImage*>(this);

    for (x = 0; x < original.width(); x++) {
        for (y = 0; y < original.height(); y++) {
            rsum = gsum = bsum = 0.0;
            for (l = 0; l < ker.width; l++) {
                for (k = 0; k < ker.height; k++) {
                    n = check(x - (l - ker.width / 2), original.width());
                    m = check(y - (k - ker.height / 2), original.height());
                    p = original.pixel(n, m);
                    rsum += ker.kernel[k][l] * qRed(p);
                    gsum += ker.kernel[k][l] * qGreen(p);
                    bsum += ker.kernel[k][l] * qBlue(p);
                }
            }
            setPixel(x, y, qRgb(checkBound(rsum), checkBound(gsum), checkBound(bsum)));
        }
    }
}

void ImageLogic::unsharpMask()
{
    double alpha = 1.0;

    Kernel ker = gaussKernel(0.5);
    Kernel id = Kernel::id(3);
    ker = (-1.) * ker;
    ker = ker + id;
    ker = alpha * ker;
    ker = ker + id;
    ker.normalize();

    convolution(ker);
}

Kernel ImageLogic::gaussKernel(double sigma)
{
    int size = 6.0 * sigma;
    if (size % 2 == 0) 
        size--;

    if (size <= 0)
        return Kernel(0, 0);

    int i, j;
    Kernel ker(size, size);

    for (i = 0; i < ker.height; i++) {
        for (j = 0; j < ker.width; j++) {
            ker.kernel[i][j] = normalDistrib(j - size / 2, i - size / 2, sigma);
        }
    }

    return ker;
}

void ImageLogic::gaussianBlur(double sigma)
{
    Kernel ker = gaussKernel(sigma);
    ker.normalize();
    if (ker.height == 0 || ker.width== 0)
        return;
    convolution(ker);
}

void ImageLogic::fastGaussianBlur(double sigma)
{
    int size = 6.0 * sigma;
    if (size % 2 == 0) 
        size--;

    if (size <= 0)
        return;

    Kernel column(1, size);
    Kernel row(size, 1);

    for (int i = 0; i < size; i++) {
        column.kernel[i][0] = normalDistrib(0, i - size / 2, sigma);
        row.kernel[0][i] = normalDistrib(i - size / 2, 0, sigma);
    }

    column.normalize();
    row.normalize();

    convolution(column);
    convolution(row);
}

void ImageLogic::glassEffect()
{
    int x, y, k, l;
    QImage original = *static_cast<QImage*>(this);
    QRgb p;

    for (k = 0; k < original.width(); k++) {
        for (l = 0; l < original.height(); l++) {
            x = check(k + (double(rand()) / double(RAND_MAX) - 0.5) * 10, original.width());
            y = check(l + (double(rand()) / double(RAND_MAX) - 0.5) * 10, original.height());
            p = original.pixel(k, l);
            setPixel(x, y, p);
        }
    }
}

void ImageLogic::wavesEffect()
{
    int x, y, k, l;
    QImage original = *static_cast<QImage*>(this);
    QRgb p;

    for (k = 0; k < original.width(); k++) {
        for (l = 0; l < original.height(); l++) {
            x = check(k + 20.0 * sin(2 * M_PI * l / 128.0), original.width());
            y = l;
            p = original.pixel(k, l);
            setPixel(x, y, p);
        }
    }
}

struct _pixel {
    int x, y;
    int value;
};

bool operator<(const _pixel& a, const _pixel& b) {
    return a.value < b.value;
}

/*
void ImageLogic::medianFilter(int radius)
{

    int diam = radius * 2 - 1;
    int size = diam * diam;
    int s2 = size / 2;
    int d2 = diam / 2;
    vector<_pixel> pixels(size);
    double light;
    int n, m, x, y, k, l, i;
    QRgb p;

    QImage original = *static_cast<QImage*>(this);

    for (x = 0; x < original.width(); x++) {
        for (y = 0; y < original.height(); y++) {
            i = 0;
            for (k = 0; k < diam; k++) {
                for (l = 0; l < diam; l++) {
                    n = check(x - (l - d2), original.width());
                    m = check(y - (k - d2), original.height());
                    p = original.pixel(n, m);
                    light = 0.2125d * qRed(p) + 0.7154d * qGreen(p) + 0.0721d * qBlue(p);
                    pixels[i].x = n;
                    pixels[i].y = m;
                    pixels[i].light = light;
                    i++;
                }
            }
            sort(pixels.begin(), pixels.begin() + i);
            setPixel(x, y, original.pixel(pixels[s2].x, pixels[s2].y));
        }
    }
}
*/

void ImageLogic::medianFilter(int radius)
{
    int diam = radius * 2 - 1;
    int size = diam * diam;
    int s2 = size / 2;
    int d2 = diam / 2;
    vector<_pixel> red(size);
    vector<_pixel> green(size);
    vector<_pixel> blue(size);
    int n, m, x, y, k, l, i;
    QRgb p;

    QImage original = *static_cast<QImage*>(this);

    for (x = 0; x < original.width(); x++) {
        for (y = 0; y < original.height(); y++) {
            i = 0;
            for (k = 0; k < diam; k++) {
                for (l = 0; l < diam; l++) {
                    n = check(x - (l - d2), original.width());
                    m = check(y - (k - d2), original.height());
                    p = original.pixel(n, m);
                    red[i].x = green[i].x = blue[i].x = n;
                    red[i].y = green[i].y = blue[i].y = m;
                    red[i].value = qRed(p);
                    green[i].value = qGreen(p);
                    blue[i].value = qBlue(p);
                    i++;
                }
            }
            sort(red.begin(), red.begin() + i);
            sort(green.begin(), green.begin() + i);
            sort(blue.begin(), blue.begin() + i);
            setPixel(x, y, qRgb(red[s2].value, green[s2].value, blue[s2].value));
        }
    }
}
