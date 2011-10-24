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

const int STEP = 10;

const double THRESHOLD = 0.2;

const double APPROX_STEP = 0.35;
const int APPROX_ORDER = 1;

const double EPS = 1e-8;

const int HUMAN_WIDTH = 80;
const int HUMAN_HEIGHT = 200;

//const int NUM_FEATURES = (HUMAN_HEIGHT / Y_SIZE) * (HUMAN_WIDTH / X_SIZE) * HOG_SIZE;
const int NUM_FEATURES = 16128;

struct Descr {
    int num;
    int x0, y0;
    int x1, y1;
};

class Logic {
    static double getBrightness(QRgb p);
    static QVector<int> getHOG(int **hog, int x0, int x1, int y0, int y1);
    static void addUnitHOG(int **hog, QVector<int>& hist, int x0, int y0, int x1, int y1);
    static int check(int x, int n);
    static QVector<QRect> detect(QImage& image, struct model *model);
    static int** calculateHOG(QImage& image);
    static void outputDescr(const QString& fileName, QVector<Descr>& description);
    static QVector<Descr> inputDescr(const QString& fileName);
    static QVector<Descr> eval(const QVector<Descr>& ans, const QVector<Descr>& res);
    static int checkBound(int x);
    static QVector<int> nonlinear(QVector<int>& x);
    static void supressNonMax(QVector<QRect>& answer, QVector<double>& prob, QVector<int> pos);

public:
    static void bootStrap(const QString& descrFileName, const QString& dirName, const QString& coefFileName);
    static void learn(const QString& descrFileName, const QString& dirName, const QString& coefFileName, const QVector<Descr>& fp = QVector<Descr>());
    static QVector<QRect> detectOne(const QString& imageFileName, const QString& coefFileName, bool *ok);
    static QVector<Descr> classify(const QString& descrFileName, const QString& dirName, const QString& coefFileName);
    static QVector<Descr> evaluate(const QString& ansFileName, const QString& resFileName);
    static void Gauss(QImage& image);

};

#endif
