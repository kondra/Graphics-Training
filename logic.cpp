#include <climits>
#include <cmath>

#include <iostream>
using std::cout;

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
    if (sum < 0.0000001)
        return;
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

ImageLogic::ImageLogic(const QImage& image)
{
    *static_cast<QImage*>(this) = image;
    selection = false;
    x1 = y1 = 0;
    x2 = width();
    y2 = height();
}

void ImageLogic::linearCorrection()
{
    int lmax = 0;
    int lmin = INT_MAX;
    int l;

    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
            QRgb p = pixel(x, y);

            l = 0.2125d * qRed(p) + 0.7154d * qGreen(p) + 0.0721d * qBlue(p);

            if (l > lmax) lmax = l;
            if (l < lmin) lmin = l;
        }
    }

    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
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

    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
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

    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
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

    for (x = x1; x < x2; x++) {     //original.width()
        for (y = y1; y < y2; y++) { //original.height()
            rsum = gsum = bsum = 0.0;
            for (l = 0; l < ker.width; l++) {
                for (k = 0; k < ker.height; k++) {
                    n = check(x - (l - ker.width / 2), x1, x2);   //original.width()
                    m = check(y - (k - ker.height / 2), y1, y2); //original.height()
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

void ImageLogic::unsharpMask(double alpha)
{
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

    for (k = x1; k < x2; k++) {     //original.width()  
        for (l = y1; l < y2; l++) { //original.height()
            x = check(k + (double(rand()) / double(RAND_MAX) - 0.5) * 10, x1, x2);
            y = check(l + (double(rand()) / double(RAND_MAX) - 0.5) * 10, y1, y2);
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

    for (k = x1; k < x2; k++) {
        for (l = y1; l < y2; l++) {
            x = check(k + 20.0 * sin(2 * M_PI * l / 128.0), x1, x2);
            y = l;
            p = original.pixel(k, l);
            setPixel(x, y, p);
        }
    }
}

void ImageLogic::medianFilter(int radius)
{
    int diam = radius * 2;
    int size = diam * diam;
    int s2 = size / 2;
    int d2 = diam / 2;
    int rm, gm, bm;
    int *red, *green, *blue;
    int n, m, x, y, k, l, i;
    QRgb p;

    QImage original = *static_cast<QImage*>(this);
    red = new int[size];
    green = new int[size];
    blue = new int[size];

    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            i = 0;
            for (k = 0; k < diam; k++) {
                for (l = 0; l < diam; l++) {
                    n = check(x - (l - d2), x1, x2);
                    m = check(y - (k - d2), y1, y2);
                    p = original.pixel(n, m);
                    red[i] = qRed(p);
                    green[i] = qGreen(p);
                    blue[i] = qBlue(p);
                    i++;
                }
            }
            rm = search(red, s2, 0, i - 1);
            gm = search(green, s2, 0, i - 1);
            bm = search(blue, s2, 0, i - 1);
            setPixel(x, y, qRgb(rm, gm, bm));
        }
    }

    delete [] red;
    delete [] green;
    delete [] blue;
}

void ImageLogic::greyWorld()
{
    double redAvg, greenAvg, blueAvg, avg;
    double n = width() * height();
    int x, y;
    int r, g, b;
    QRgb p;

    avg = redAvg = greenAvg = blueAvg = 0.0;
    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            p = pixel(x, y);
            redAvg += qRed(p);
            greenAvg += qGreen(p);
            blueAvg += qBlue(p);
        }
    }

    redAvg /= n;
    greenAvg /= n;
    blueAvg /= n;
    avg = (redAvg + greenAvg + blueAvg) / 3.;

    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            p = pixel(x, y);
            r = checkBound(qRed(p) * avg / redAvg);
            g = checkBound(qGreen(p) * avg / greenAvg);
            b = checkBound(qBlue(p) * avg / blueAvg);
            setPixel(x, y, qRgb(r, g, b));
        }
    }
}

void ImageLogic::userFilter(Kernel& ker)
{
    convolution(ker);
}

static bool check2rot(int x, int w)
{
    if (x > w + 1 || x < -1)
        return true;
    return false;
}

static bool check2scale(int x, int w)
{
    if (x > w - 1 || x < 0)
        return true;
    return false;
}

void ImageLogic::scalingSelection(double scale)
{
    int Width = abs(x2 - x1);
    int Height = abs(y2 - y1);
    int newWidth = Width * scale;
    int newHeight = Height  * scale;
    int x, y;
    double xOld, yOld;
    int xCeil, yCeil, xFloor, yFloor;
    double xDelta, yDelta;
    double topRed, topGreen, topBlue, bottomRed, bottomGreen, bottomBlue;
    double red, green, blue;
    QRgb topLeft, topRight, bottomLeft, bottomRight;
    QImage original = *static_cast<QImage*>(this);

    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            setPixel(x, y, qRgb(255, 255, 255));
        }
    }

    int bx = x1 - (newWidth - Width) / 2;
    int by = y1 - (newHeight - Height) / 2;
    for (x = bx; x < x2 + (newWidth - Width) / 2; x++) {
        for (y = by; y < y2 + (newHeight - Height) / 2; y++) {
            if (x >= width() || y >= height() || x < 0 || y < 0)
                continue;

            xOld = x1 + (x - bx) / scale;
            yOld = y1 + (y - by) / scale;

            xCeil = ceil(xOld);
            yCeil = ceil(yOld);
            xFloor = floor(xOld);
            yFloor = floor(yOld);

            xDelta = xOld - double(xFloor);
            yDelta = yOld - double(yFloor);

            if (check2scale(xFloor, width()) || check2scale(xCeil, width()) || check2scale(yFloor, height()) || check2scale(yCeil, height()))
                continue;

            topLeft = original.pixel(xFloor, yFloor);
            topRight = original.pixel(xCeil, yFloor);
            bottomLeft = original.pixel(xFloor, yCeil);
            bottomRight = original.pixel(xCeil, yCeil);

            topRed = (1 - xDelta) * qRed(topLeft) + xDelta * qRed(topRight);
            topGreen = (1 - xDelta) * qGreen(topLeft) + xDelta * qGreen(topRight);
            topBlue = (1 - xDelta) * qBlue(topLeft) + xDelta * qBlue(topRight);

            bottomRed = (1 - xDelta) * qRed(bottomLeft) + xDelta * qRed(bottomRight);
            bottomGreen = (1 - xDelta) * qGreen(bottomLeft) + xDelta * qGreen(bottomRight);
            bottomBlue = (1 - xDelta) * qBlue(bottomLeft) + xDelta * qBlue(bottomRight);

            red = (1 - yDelta) * topRed + yDelta * bottomRed;
            green = (1 - yDelta) * topGreen + yDelta * bottomGreen;
            blue = (1 - yDelta) * topBlue + yDelta * bottomBlue;

            setPixel(x, y, qRgb(checkBound(red), checkBound(green), checkBound(blue)));
        }
    }
}

void ImageLogic::scaling(double scale)
{
    if (x1 != 0 || y1 != 0 || x2 != width() || y2 != height())
        return scalingSelection(scale);
    int Width = abs(x2 - x1);
    int Height = abs(y2 - y1);
    int newWidth = Width * scale;
    int newHeight = Height  * scale;
    int x, y;
    double xOld, yOld;
    int xCeil, yCeil, xFloor, yFloor;
    double xDelta, yDelta;
    double topRed, topGreen, topBlue, bottomRed, bottomGreen, bottomBlue;
    double red, green, blue;
    QRgb topLeft, topRight, bottomLeft, bottomRight;
    QImage original = *static_cast<QImage*>(this);

    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            setPixel(x, y, qRgb(255, 255, 255));
        }
    }

    for (x = (newWidth - Width) / 2; x < (newWidth + Width) / 2; x++) {
        for (y = (newHeight - Height) / 2; y < (newHeight + Height) / 2; y++) {
            xOld = x / scale;
            yOld = y / scale;

            xCeil = ceil(xOld);
            yCeil = ceil(yOld);
            xFloor = floor(xOld);
            yFloor = floor(yOld);

            xDelta = xOld - double(xFloor);
            yDelta = yOld - double(yFloor);

            if (check2scale(xFloor, width()) || check2scale(xCeil, width()) || check2scale(yFloor, height()) || check2scale(yCeil, height()))
                continue;

            topLeft = original.pixel(xFloor, yFloor);
            topRight = original.pixel(xCeil, yFloor);
            bottomLeft = original.pixel(xFloor, yCeil);
            bottomRight = original.pixel(xCeil, yCeil);

            topRed = (1 - xDelta) * qRed(topLeft) + xDelta * qRed(topRight);
            topGreen = (1 - xDelta) * qGreen(topLeft) + xDelta * qGreen(topRight);
            topBlue = (1 - xDelta) * qBlue(topLeft) + xDelta * qBlue(topRight);

            bottomRed = (1 - xDelta) * qRed(bottomLeft) + xDelta * qRed(bottomRight);
            bottomGreen = (1 - xDelta) * qGreen(bottomLeft) + xDelta * qGreen(bottomRight);
            bottomBlue = (1 - xDelta) * qBlue(bottomLeft) + xDelta * qBlue(bottomRight);

            red = (1 - yDelta) * topRed + yDelta * bottomRed;
            green = (1 - yDelta) * topGreen + yDelta * bottomGreen;
            blue = (1 - yDelta) * topBlue + yDelta * bottomBlue;

            setPixel(x - (newWidth - Width) / 2, y - (newHeight - Height) / 2, qRgb(checkBound(red), checkBound(green), checkBound(blue)));
        }
    }
}

void ImageLogic::rotation(double alpha)
{
    int x, y;
    double x0, y0;
    double xOld, yOld;
    int xCeil, yCeil, xFloor, yFloor;
    double xDelta, yDelta;
    double topRed, topGreen, topBlue, bottomRed, bottomGreen, bottomBlue;
    double red, green, blue;
    QRgb topLeft, topRight, bottomLeft, bottomRight;
    QImage original = *static_cast<QImage*>(this);

    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            setPixel(x, y, qRgb(255, 255, 255));
        }
    }

    x0 = width() / 2.;
    y0 = height() / 2.;

    alpha = -alpha;
    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            xOld = (x - x0) * cos(alpha) - (y - y0) * sin(alpha) + x0;
            yOld = (x - x0) * sin(alpha) + (y - y0) * cos(alpha) + y0;

            xCeil = ceil(xOld);
            yCeil = ceil(yOld);
            xFloor = floor(xOld);
            yFloor = floor(yOld);

            xDelta = xOld - double(xFloor);
            yDelta = yOld - double(yFloor);

            if (check2rot(xFloor, width()) || check2rot(xCeil, width()) || check2rot(yFloor, height()) || check2rot(yCeil, height()))
                continue;

            topLeft = original.pixel(check(xFloor, 0, width()), check(yFloor, 0, height()));
            topRight = original.pixel(check(xCeil, 0, width()), check(yFloor, 0, height()));
            bottomLeft = original.pixel(check(xFloor, 0, width()), check(yCeil, 0, height()));
            bottomRight = original.pixel(check(xCeil, 0, width()), check(yCeil, 0, height()));

            topRed = (1 - xDelta) * qRed(topLeft) + xDelta * qRed(topRight);
            topGreen = (1 - xDelta) * qGreen(topLeft) + xDelta * qGreen(topRight);
            topBlue = (1 - xDelta) * qBlue(topLeft) + xDelta * qBlue(topRight);

            bottomRed = (1 - xDelta) * qRed(bottomLeft) + xDelta * qRed(bottomRight);
            bottomGreen = (1 - xDelta) * qGreen(bottomLeft) + xDelta * qGreen(bottomRight);
            bottomBlue = (1 - xDelta) * qBlue(bottomLeft) + xDelta * qBlue(bottomRight);

            red = (1 - yDelta) * topRed + yDelta * bottomRed;
            green = (1 - yDelta) * topGreen + yDelta * bottomGreen;
            blue = (1 - yDelta) * topBlue + yDelta * bottomBlue;

            setPixel(x, y, qRgb(checkBound(red), checkBound(green), checkBound(blue)));
        }
    }
}

void ImageLogic::setSelection(int _x1, int _y1, int _x2, int _y2)
{
    x1 = _x1;
    y1 = _y1;
    x2 = _x2;
    y2 = _y2;
    selection = true;
}

void ImageLogic::resetSelection()
{
    selection = false;
    x1 = y1 = 0;
    x2 = width();
    y2 = height();
}
