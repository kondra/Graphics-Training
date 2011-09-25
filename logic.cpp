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
    if (sum < eps)
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

void ImageLogic::getLuminosity(int r, int g, int b)
{
    return r * RED_INTENSE + g * GREEN_INTENSE + b * BLUE_INTENSE;
}

void ImageLogic::linearCorrection()
{
    double lmax = 0;
    double lmin = INT_MAX;
    double luminosity[256];
    int r, g, b;
    int x, y;

    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            QRgb p = pixel(x, y);

            luminosity[round(getLuminosity(qRed(p), qGreen(p), qBlue(p)))]++;

            if (l > lmax) lmax = l;
            if (l < lmin) lmin = l;
        }
    }

    if (lmax - lmin < eps)
        return;

    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            QRgb p = pixel(x, y);

            r = checkColor((qRed(p) - lmin) * 255. / (lmax - lmin));
            g = checkColor((qGreen(p) - lmin) * 255. / (lmax - lmin));
            b = checkColor((qBlue(p) - lmin) * 255. / (lmax - lmin));

            l = 0.2125 * qRed(p) + 0.7154 * qGreen(p) + 0.0721 * qBlue(p);
            if (fabs(l - lmin) < eps)
                r = g = b = 0;
            if (fabs(l - lmax) < eps)
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

            if (rmax > rmin)
                r = checkColor((qRed(p) - rmin) * 255 / (rmax - rmin));
            if (gmax > gmin)
                g = checkColor((qGreen(p) - gmin) * 255 / (gmax - gmin));
            if (bmax > bmin)
                b = checkColor((qBlue(p) - bmin) * 255 / (bmax - bmin));

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

    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            rsum = gsum = bsum = 0.0;
            for (l = 0; l < ker.width; l++) {
                for (k = 0; k < ker.height; k++) {
                    n = check(x - (l - ker.width / 2), x1, x2);
                    m = check(y - (k - ker.height / 2), y1, y2);
                    p = original.pixel(n, m);
                    rsum += ker.kernel[k][l] * qRed(p);
                    gsum += ker.kernel[k][l] * qGreen(p);
                    bsum += ker.kernel[k][l] * qBlue(p);
                }
            }
            setPixel(x, y, qRgb(checkColor(rsum), checkColor(gsum), checkColor(bsum)));
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

void ImageLogic::glassEffect(int radius)
{
    int x, y, k, l;
    QImage original = *static_cast<QImage*>(this);
    QRgb p;

    for (k = x1; k < x2; k++) {
        for (l = y1; l < y2; l++) {
            x = check(k + (double(rand()) / double(RAND_MAX) - 0.5) * radius, x1, x2);
            y = check(l + (double(rand()) / double(RAND_MAX) - 0.5) * radius, y1, y2);
            p = original.pixel(k, l);
            setPixel(x, y, p);
        }
    }
}

void ImageLogic::wavesEffect(double waveLength, double amplitude)
{
    int x, y, k, l;
    QImage original = *static_cast<QImage*>(this);
    QRgb p;

    for (k = x1; k < x2; k++) {
        for (l = y1; l < y2; l++) {
            x = check(k + amplitude * sin(2 * M_PI * double(l) / waveLength), x1, x2);
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
            r = checkColor(qRed(p) * avg / redAvg);
            g = checkColor(qGreen(p) * avg / greenAvg);
            b = checkColor(qBlue(p) * avg / blueAvg);
            setPixel(x, y, qRgb(r, g, b));
        }
    }
}

void ImageLogic::userFilter(Kernel& ker)
{
    convolution(ker);
}

void ImageLogic::scaling(double scale)
{
    int Width = x2 - x1;
    int Height = y2 - y1;
    int newWidth = Width * scale;
    int newHeight = Height  * scale;
    int x, y;
    double xOld, yOld;
    int xCeil, yCeil, xFloor, yFloor;
    int bx, by, ex, ey;
    QImage original = *static_cast<QImage*>(this);
    QRgb p;

    fillSelection();

    if (selection) {
        bx = x1 - (newWidth - Width) / 2;
        by = y1 - (newHeight - Height) / 2;
        ex = x2 + (newWidth - Width) / 2;
        ey = y2 + (newHeight - Height) / 2;
    } else {
        bx = (newWidth - Width) / 2;
        by = (newHeight - Height) / 2;
        ex = (newWidth + Width) / 2;
        ey = (newHeight + Height) / 2;
    }

    for (x = bx; x < ex; x++) {
        for (y = by; y < ey; y++) {
            if (selection) {
                if (x >= width() || y >= height() || x < 0 || y < 0)
                    continue;
                xOld = x1 + (x - bx) / scale;
                yOld = y1 + (y - by) / scale;
            } else {
                xOld = x / scale;
                yOld = y / scale;
            }

            xCeil = ceil(xOld);
            yCeil = ceil(yOld);
            xFloor = floor(xOld);
            yFloor = floor(yOld);

            if (check2scale(xFloor, width()) || check2scale(xCeil, width()) || check2scale(yFloor, height()) || check2scale(yCeil, height()))
                continue;

            p = bilinearInterpolation(original, xOld, yOld, xFloor, xCeil, yFloor, yCeil);

            if (selection)
                setPixel(x, y, p);
            else
                setPixel(x - bx, y - by, p);
        }
    }
}

QRgb ImageLogic::bilinearInterpolation(const QImage& original, double xOld, double yOld, int xFloor, int xCeil, int yFloor, int yCeil)
{
    double xDelta, yDelta;
    double topRed, topGreen, topBlue, bottomRed, bottomGreen, bottomBlue;
    double red, green, blue;
    QRgb topLeft, topRight, bottomLeft, bottomRight;

    xDelta = xOld - double(xFloor);
    yDelta = yOld - double(yFloor);

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

    return qRgb(checkColor(red), checkColor(green), checkColor(blue));
}

void ImageLogic::fillSelection()
{
    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
            setPixel(x, y, qRgb(255, 255, 255));
        }
    }
}

void ImageLogic::rotate(double alpha)
{
    int Width = x2 - x1;
    int Height = y2 - y1;
    int x, y;
    double x0, y0;
    double xOld, yOld;
    int xCeil, yCeil, xFloor, yFloor;
    QImage original = *static_cast<QImage*>(this);
    QRgb p;

    fillSelection();

    x0 = x1 + Width / 2.;
    y0 = y1 + Height / 2.;

    alpha = -alpha;
    for (x = 0; x < width(); x++) {
        for (y = 0; y < height(); y++) {
            xOld = (x - x0) * cos(alpha) - (y - y0) * sin(alpha) + x0;
            yOld = (x - x0) * sin(alpha) + (y - y0) * cos(alpha) + y0;

            xCeil = ceil(xOld);
            yCeil = ceil(yOld);
            xFloor = floor(xOld);
            yFloor = floor(yOld);

            if (check2rot(xFloor, x1, x2) || check2rot(xCeil, x1, x2) || check2rot(yFloor, y1, y2) || check2rot(yCeil, y1, y2))
                continue;

            p = bilinearInterpolation(original, xOld, yOld, xFloor, xCeil, yFloor, yCeil);

            setPixel(x, y, p);
        }
    }
}

void ImageLogic::setSelection(int _x1, int _y1, int _x2, int _y2)
{
    x1 = _x1;
    y1 = _y1;
    x2 = _x2;
    y2 = _y2;
    if (x1 > x2)
        qSwap(x1, x2);
    if (y1 > y2)
        qSwap(y1, y2);
    if (y1 == y2 || x1 == x2) {
        selection = false;
        return;
    }
    if (x2 > width())
        x2 = width();
    if (y2 > height())
        y2 = height();
    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    selection = true;
}

void ImageLogic::resetSelection()
{
    selection = false;
    x1 = y1 = 0;
    x2 = width();
    y2 = height();
}
