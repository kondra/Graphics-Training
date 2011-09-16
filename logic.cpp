#include <climits>
#include <iostream>

#include "logic.h"

using std::cout;

void Logic::linearCorrection()
{
    int lmax = 0;
    int lmin = INT_MAX;

    for (int x = 0; x < image->width(); x++) {
        for (int y = 0; y < image->height(); y++) {
            QRgb p = image->pixel(x, y);

            int l = 0.2125d * qRed(p) + 0.7154d * qGreen(p) + 0.0721d * qBlue(p);

            if (l > lmax)
                lmax = l;
            if (l < lmin)
                lmin = l;
        }
    }

    for (int x = 0; x < image->width(); x++) {
        for (int y = 0; y < image->height(); y++) {
            QRgb p = image->pixel(x, y);

            int r = checkBound((qRed(p) - lmin) * 255 / (lmax - lmin));
            int g = checkBound((qGreen(p) - lmin) * 255 / (lmax - lmin));
            int b = checkBound((qBlue(p) - lmin) * 255 / (lmax - lmin));

            image->setPixel(x, y, qRgb(r, g, b));
        }
    }

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::channelLinearCorrection()
{
    int rmax, bmax, gmax;
    int rmin, bmin, gmin;
    int r, g, b;

    rmax = bmax = gmax = 0;
    rmin = bmin = gmin = INT_MAX;

    for (int x = 0; x < image->width(); x++) {
        for (int y = 0; y < image->height(); y++) {
            QRgb p = image->pixel(x, y);

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

    for (int x = 0; x < image->width(); x++) {
        for (int y = 0; y < image->height(); y++) {
            QRgb p = image->pixel(x, y);

            int r = checkBound((qRed(p) - rmin) * 255 / (rmax - rmin));
            int g = checkBound((qGreen(p) - gmin) * 255 / (gmax - gmin));
            int b = checkBound((qBlue(p) - bmin) * 255 / (bmax - bmin));

            image->setPixel(x, y, qRgb(r, g, b));
        }
    }

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}
