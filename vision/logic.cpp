#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <QFile>
#include <QDir>
#include <QTextStream>

#include <QDebug>

#include "logic.h"
#include "liblinear-1.8/linear.h"

#define Malloc(type, size) (type *)malloc(sizeof(type) * (size))

double Logic::getBrightness(QRgb p)
{
    return qRed(p) * RED_INTENSE +
           qGreen(p) * GREEN_INTENSE +
           qBlue(p) * BLUE_INTENSE;
}

int Logic::check(int x, int n)
{
    if (x >= n)
        return n - 1;
    return x;
}

void Logic::addUnitHOG(const QImage& image, QVector<int>& hist, int x0, int y0, int x1, int y1)
{
    double gradX, gradY;
    double angle;
    int height = image.height();
    int width = image.width();
    int h[HOG_SIZE];
    int i, j;

    for (i = 0; i < HOG_SIZE; i++) {
        h[i] = 0;
    }

    for (i = x0; i < x1; i++) {
        for (j = y0; j < y1; j++) {
            gradX = getBrightness(image.pixel(check(i + 1, width), j)) - getBrightness(image.pixel(i, j));
            gradY = getBrightness(image.pixel(i, check(j + 1, height))) - getBrightness(image.pixel(i, j));
            angle = atan2(gradY, gradX) + 2 * PI;
            if ((int)(4 * angle / PI)) {
                qWarning("FUUUU");
            }
            h[(int)(4 * angle / PI)]++;
        }
    }

    for (i = 0; i < HOG_SIZE; i++) {
        hist.push_back(h[i]);
    }
}

QVector<int> Logic::getHOG(const QImage& image, int x0, int x1)
{
    QVector<int> hist;

    int height = image.height();
    printf("hog\n");

    for (int i = x0; i < x1; i += X_SIZE) {
        for (int j = 0; j < height; j += Y_SIZE) {
            addUnitHOG(image, hist, i, j, i + X_SIZE, j + Y_SIZE);
        }
    }

    printf("%d\n", hist.size());
    return hist;
}

struct Descr {
    int num;
    int x0, y0;
    int x1, y1;
};

bool operator<(const Descr& a,const Descr& b) {
    return a.num < b.num;
}

void Logic::learn(const QString& descrFileName, const QString& dirName, const QString& coefFileName)
{
    QFile file(descrFileName);
    QDir dir(dirName);
    QImage image;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Cannot open description file");
        return;
    }

    if (!dir.exists()) {
        qWarning("Cannot find directory");
        return;
    }

    dir.setFilter(QDir::Files | QDir::Readable);
    dir.setSorting(QDir::Name);
    QStringList list = dir.entryList();

    QTextStream in(&file);
    QVector<Descr> description;
    Descr tmp;

    while (!in.atEnd()) {
        in >> tmp.num >> tmp.x0 >> tmp.y0 >> tmp.x1 >> tmp.y1;
        description.push_back(tmp);
    }
    file.close();

    qSort(description.begin(), description.end());

    QString curName;
    bool ok;
    int num;
    int i, j, j1, j2, x;
    int w;

    QVector<QVector<int> > trainFeatures;
    QVector<int> trainLabels;

    for (i = 0; i < list.size(); i++) {
        curName = list.at(i);
        if (!curName.endsWith(".png")) {
            continue;
        }
        curName.replace(".png", "");
        num = curName.toInt(&ok);
        if (!ok) {
            continue;
        }
        curName = list.at(i);
        image = QImage(dir.absoluteFilePath(curName));
        if (image.isNull()) {
            continue;
        }
        qDebug() << "ok";
        j1 = -1;
        j2 = 0;
        for (j = 0; j < description.size(); j++) {
            if (description[j].num == num) {
                j1 = j;
            }
            if (j1 != -1 && description[j].num != num) {
                j2 = j;
                break;
            }
        }
        for (j = j1; j < j2; j++) {
            trainFeatures.push_back(getHOG(image, description[j].x0, description[j].x1));
            trainLabels.push_back(1);
        }
        w = image.width() - 80;
        ok = false;
        while (!ok) {
            x = rand() % w;
            for (j = j1; j < j2; j++) {
                if (x > description[j].x0 && x < description[j].x1) {
                    ok = false;
                    break;
                }
            }
        }
        trainFeatures.push_back(getHOG(image, x, x + 80));
        trainLabels.push_back(-1);
    }

    int num_features = trainFeatures[0].size();
    struct problem prob;
    prob.l = (int)trainLabels.size();
    prob.bias = 0;
    prob.n = num_features + 1;

    prob.y = Malloc(int, prob.l);
    prob.x = Malloc(struct feature_node *, prob.l);

    for (i = 0; i < trainLabels.size(); i++) {
        prob.x[i] = Malloc(struct feature_node, num_features + 1);
        prob.x[i][num_features].index = -1;
        for (j = 0; j < num_features; j++) {
            prob.x[i][j].index = 1 + j;
            prob.x[i][j].value = trainFeatures[i][j];
        }
        prob.y[i] = trainLabels[i];
    }

    struct parameter param;
    param.solver_type = L2R_L2LOSS_SVC_DUAL;
    param.C = 1;
    param.eps = 1e-4;
    param.nr_weight = 0;
    param.weight_label = NULL;
    param.weight = NULL;

    struct model *modelPedestrian = train(&prob, &param);

    if (save_model(coefFileName.toAscii(), modelPedestrian)) {
        qWarning("Cannot save model to file");
        return;
    }

    free_and_destroy_model(&modelPedestrian);
    destroy_param(&param);
    free(prob.y);
    for (i = 0; i < prob.n; i++) {
        free(prob.x[i]);
    }
    free(prob.x);
}
