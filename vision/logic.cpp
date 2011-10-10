#include <cmath>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>

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
    double coef = HOG_SIZE / (2 * PI);
    int height = image.height();
    int width = image.width();
    int h[HOG_SIZE];
    int i, j;

    for (i = 0; i < HOG_SIZE; i++) {
        h[i] = 0;
    }

    for (i = x0; i < x1; i++) {
        for (j = y0; j < y1; j++) {
            gradX = getBrightness(image.pixel(check(i + 1, width), check(j, height))) - getBrightness(image.pixel(check(i, width), check(j, height)));
            gradY = getBrightness(image.pixel(check(i, width), check(j + 1, height))) - getBrightness(image.pixel(check(i, width), check(j, height)));
            angle = atan2(gradY, gradX) + PI;
            h[(int)(angle * coef)]++;
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

    for (int i = x0; i < x1; i += X_SIZE) {
        for (int j = 0; j < height - 1; j += Y_SIZE) {
            addUnitHOG(image, hist, i, j, i + X_SIZE, j + Y_SIZE);
        }
    }

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
    QStringList list = dir.entryList();

    QTextStream in(&file);
    QVector<Descr> description;
    Descr tmp;

    while (!in.atEnd()) {
        in >> tmp.num >> tmp.y0 >> tmp.x0 >> tmp.y1 >> tmp.x1;
        description.push_back(tmp);
    }
    file.close();
    description.pop_back();

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
        if (j == description.size()) {
            j2 = j;
        }
        assert(j1 != -1);
        for (j = j1; j < j2; j++) {
            trainFeatures.push_back(getHOG(image, description[j].x0, description[j].x1));
            trainLabels.push_back(1);
        }
        w = image.width() - PEDESTRIAN_WIDTH;
        ok = false;
        while (!ok) {
            x = rand() % w;
            ok = true;
            for (j = j1; j < j2; j++) {
                if (x > description[j].x0 && x < description[j].x1) {
                    ok = false;
                    break;
                }
            }
        }
        trainFeatures.push_back(getHOG(image, x, x + PEDESTRIAN_WIDTH));
        trainLabels.push_back(-1);
        printf("Processing %d.png\n", num);
    }

    struct problem prob;
    prob.l = (int)trainLabels.size();
    prob.bias = 0;
    prob.n = NUM_FEATURES + 1;

    prob.y = Malloc(int, prob.l);
    prob.x = Malloc(struct feature_node *, prob.l);

    for (i = 0; i < trainLabels.size(); i++) {
        prob.x[i] = Malloc(struct feature_node, NUM_FEATURES + 1);
        prob.x[i][NUM_FEATURES].index = -1;
        for (j = 0; j < NUM_FEATURES; j++) {
            prob.x[i][j].index = 1 + j;
            prob.x[i][j].value = trainFeatures[i][j];
        }
        printf("\n");
        prob.y[i] = trainLabels[i];
    }

    struct parameter param;
    param.solver_type = L2R_L2LOSS_SVC_DUAL;
    param.C = 1;
    param.eps = 1e-4;
    param.nr_weight = 0;
    param.weight_label = NULL;
    param.weight = NULL;

    struct model *model = train(&prob, &param);

    if (save_model(coefFileName.toAscii(), model)) {
        qWarning("Cannot save model to file");
    }

    free_and_destroy_model(&model);
    destroy_param(&param);
    free(prob.y);
    for (i = 0; i < prob.l; i++) {
        free(prob.x[i]);
    }
    free(prob.x);
}

QVector<QRect> Logic::detect(const QImage& image, struct model *model)
{
    int i, j;
    int width = image.width();
//    int predict_label;
    QVector<int> features;
    QVector<QRect> answer;
    double prob[1];

    struct feature_node *x = Malloc(struct feature_node, NUM_FEATURES + 1);
    x[NUM_FEATURES].index = -1;

    for (i = 0; i < width - PEDESTRIAN_WIDTH; i += STEP) {
        features = getHOG(image, i, i + PEDESTRIAN_WIDTH);
        for (j = 0; j < NUM_FEATURES; j++) {
            x[j].index = j + 1;
            x[j].value = features[j];
        }
        //predict_label = predict(model, x);
        predict_values(model, x, prob);
        //printf("%lf\n", prob[0]);
        //if (predict_label == 1) {
        if (prob[0] > 0.0) {
            answer.push_back(QRect(i, 0, PEDESTRIAN_WIDTH, PEDESTRIAN_HEIGHT));
        }
        features.clear();
    }

    free(x);

    return answer;
}

QVector<QRect> Logic::detectOne(const QString& imageFileName, const QString& coefFileName, bool *ok)
{
    QImage image = QImage(imageFileName);
    struct model *model;

    if (image.isNull()) {
        qWarning("Cannot open image file");
        *ok = false;
        return QVector<QRect>();
    }

    if ((model = load_model(coefFileName.toAscii())) == 0) {
        qWarning("Cannot load model from file");
        *ok = false;
        return QVector<QRect>();
    }

    int t = clock();
    QVector<QRect> result = detect(image, model);
    printf("detection time: %lfs\n", double(clock() - t) / CLOCKS_PER_SEC);
    *ok = true;

    free_and_destroy_model(&model);

    return result;
}

void Logic::classify(const QString& descrFileName, const QString& dirName, const QString& coefFileName)
{
    QFile file(descrFileName);
    QDir dir(dirName);
    QImage image;
    struct model *model;

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning("Cannot create description file");
        return;
    }

    if (!dir.exists()) {
        qWarning("Cannot find directory");
        return;
    }

    if ((model = load_model(coefFileName.toAscii())) == 0) {
        qWarning("Cannot load model from file");
        return;
    }

    dir.setFilter(QDir::Files | QDir::Readable);
    QStringList list = dir.entryList();

    QTextStream out(&file);
    QVector<Descr> description;
    Descr tmp;

    QString curName;
    QVector<QRect> result;

    int j, i, num;
    bool ok;

    int t = clock();

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
        result = detect(image, model);
        for (j = 0; j < result.size(); j++) {
            tmp.num = num;
            tmp.y0 = result[j].y();
            tmp.x0 = result[j].x();
            tmp.y1 = result[j].y() + result[j].height();
            tmp.x1 = result[j].x() + result[j].width();
            description.push_back(tmp);
        }
        printf("Processing %d.png\n", num);
    }

    qSort(description.begin(), description.end());
    for (j = 0; j < description.size(); j++) {
        tmp = description[j];
        out << tmp.num << " " << tmp.y0 << " " << tmp.x0 << " " << tmp.y1 << " " << tmp.x1 << "\n";
    }

    printf("classifieing time %lfs\n", double(clock() - t) / CLOCKS_PER_SEC);

    file.close();

    free_and_destroy_model(&model);
}

void Logic::evaluate(const QString& ansFileName, const QString& resFileName)
{
    QFile ansFile(ansFileName);
    QFile resFile(resFileName);

    double precision, recall;
    int i, j, k;

    if (!ansFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Cannot open answer file");
        return;
    }

    if (!resFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Cannot open result file");
        return;
    }

    QVector<Descr> ans;
    QVector<bool> lbl;
    QVector<Descr> res;
    Descr tmp;

    {
        QTextStream in(&ansFile);
        while (!in.atEnd()) {
            in >> tmp.num >> tmp.y0 >> tmp.x0 >> tmp.y1 >> tmp.x1;
            ans.push_back(tmp);
            lbl.push_back(false);
        }
        ans.pop_back();
        lbl.pop_back();
        ansFile.close();
    }
    printf("%d\n", ans[ans.size()-1].num);

    {
        QTextStream in(&resFile);
        while (!in.atEnd()) {
            in >> tmp.num >> tmp.y0 >> tmp.x0 >> tmp.y1 >> tmp.x1;
            res.push_back(tmp);
        }
        res.pop_back();
        resFile.close();
    }

    qSort(res.begin(), res.end());
    qSort(ans.begin(), ans.end());

    int gt, tp, fp, tp1;
    bool ok;
    gt = tp = fp = tp1 = 0;

    gt = ans.size();

    k = 0;
    for (i = 0; i < res.size(); i++) {
        ok = false;
        for (j = 0; j < ans.size(); j++) {
            if (ans[j].num == res[i].num) {
                if (abs(res[i].x0 - ans[j].x0) <= 40) {
//                    printf("%d %d\n", res[i].x0, ans[j].x0);
                    tp++;
                    lbl[j] = true;
                    ok = true;
                    break;
                }
            }
            if (ans[j].num > res[i].num) {
                break;
            }
        }
        if (!ok) {
            fp++;
        }
    }

    for (j = 0; j < lbl.size(); j++) {
        tp1 += lbl[j];
    }

    recall = double(tp1) / double(gt);
    precision = double(tp) / double(tp + fp);

    printf("#TP=%d\n", tp);
    printf("#TP'=%d\n", tp1);
    printf("#FP=%d\n", fp);
    printf("#GT=%d\n", gt);

    printf("Recall %lf%%\nPrecision %lf%%\n", recall * 100.0, precision * 100.0);
}
