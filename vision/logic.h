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

const int HOG_SIZE = 16;

const int STEP = 15;

const double THRESHOLD = 0.25;

const double EPS = 1e-8;

const int HUMAN_WIDTH = 80;
const int HUMAN_HEIGHT = 200;

const int NUM_FEATURES = (HUMAN_HEIGHT / Y_SIZE) * (HUMAN_WIDTH / X_SIZE) * HOG_SIZE;

class Logic {
    static double getBrightness(QRgb p);
    static QVector<int> getHOG(int **hog, int x0, int x1, int y0, int y1);
    static void addUnitHOG(int **hog, QVector<int>& hist, int x0, int y0, int x1, int y1);
    static int check(int x, int n);
    static QVector<QRect> detect(const QImage& image, struct model *model);
    static int** calculateHOG(const QImage& image);

public:
    static void learn(const QString& descrFileName, const QString& dirName, const QString& coefFileName);
    static QVector<QRect> detectOne(const QString& imageFileName, const QString& coefFileName, bool *ok);
    static void classify(const QString& descrFileName, const QString& dirName, const QString& coefFileName);
    static void evaluate(const QString& ansFileName, const QString& resFileName);

};

#endif
