#ifndef _LOGIC_H_
#define _LOGIC_H_

#include <QImage>
#include <QVector>

const double RED_INTENSE = 0.2125;
const double GREEN_INTENSE = 0.7154;
const double BLUE_INTENSE = 0.0721;

const double PI = 3.1415926535897;

const int X_SIZE = 10;
const int Y_SIZE = 10;

const int HOG_SIZE = 8;

class Logic {

    static double getBrightness(QRgb p);
    static QVector<int> getHOG(const QImage& image, int x0, int x1);
    static void addUnitHOG(const QImage& image, QVector<int>& hist, int x0, int y0, int x1, int y1);
    static int check(int x, int n);
public:
    static void learn(const QString& descrFileName, const QString& dirName, const QString& coefFileName);
};

#endif
