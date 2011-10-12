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
    if (x < 0)
        return 0;
    return x;
}

int** Logic::calculateHOG(const QImage& image)
{
    double gradX, gradY;
    double angle;
    double coef = HOG_SIZE / (2 * PI);
    int height = image.height();
    int width = image.width();
    int **hog = 0;
    int i, j;

    hog = new int*[width];
    for (i = 0; i < width; i++) {
        hog[i] = new int[height];
        for (j = 0; j < height; j++) {
            gradX = getBrightness(image.pixel(check(i + 1, width), check(j, height))) - getBrightness(image.pixel(check(i, width), check(j, height)));
            gradY = getBrightness(image.pixel(check(i, width), check(j + 1, height))) - getBrightness(image.pixel(check(i, width), check(j, height)));
            angle = atan2(gradY, gradX) + PI;
            if (angle > 2 * PI) {
                angle -= EPS;
            }
            int k = (int)(angle * coef);
            assert(k >= 0 && k < HOG_SIZE);
            hog[i][j] = k;
        }
    }

    return hog;
}

void Logic::addUnitHOG(int **hog, QVector<int>& hist, int x0, int y0, int x1, int y1)
{
    int h[HOG_SIZE];
    int i, j;

    assert(hog);

    for (i = 0; i < HOG_SIZE; i++) {
        h[i] = 0;
    }

    for (i = x0; i < x1; i++) {
        for (j = y0; j < y1; j++) {
            h[hog[i][j]]++;
        }
    }

    for (i = 0; i < HOG_SIZE; i++) {
        hist.push_back(h[i]);
    }
}

QVector<int> Logic::getHOG(int **hog, int x0, int x1, int y0, int y1)
{
    QVector<int> hist;

    for (int i = x0; i < x1; i += X_SIZE) {
        for (int j = y0; j < y1; j += Y_SIZE) {
            addUnitHOG(hog, hist, i, j, check(i + X_SIZE, x1), check(j + Y_SIZE, y1));
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

    int t = clock();

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
    int i, j, j1, j2;
    int w;

    int **hog;

    QVector<QVector<int> > trainFeatures;
    QVector<int> trainLabels;

    bool f = false;

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
        hog = calculateHOG(image);
        f = true;
        for (j = j1; j < j2; j++) {
            trainFeatures.push_back(getHOG(hog, description[j].x0, description[j].x1, 0, image.height()));
            trainLabels.push_back(1);
        }
        w = image.width();
        /*
         * too smart
        int x;
        w = image.width() - HUMAN_WIDTH;
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
        trainFeatures.push_back(getHOG(image, x, x + HUMAN_WIDTH));
        trainLabels.push_back(-1);
        */
        if (description[j1].x0 + 2 * HUMAN_WIDTH + 1 < w) {
            trainFeatures.push_back(getHOG(hog, description[j1].x0 + HUMAN_WIDTH + 1, description[j1].x0 + 2 * HUMAN_WIDTH + 1, 0, image.height()));
        } else if (description[j1].x0 - HUMAN_WIDTH - 1 >= 0) {
            trainFeatures.push_back(getHOG(hog, description[j1].x0 - HUMAN_WIDTH - 1, description[j1].x0 - 1, 0, image.height()));
        }
        trainLabels.push_back(-1);
        //printf("Processing %d.png\n", num);
        printf("#");
        fflush(stdout);
        delete [] hog;
    }
    printf("\n");

    struct problem prob;
    prob.l = (int)trainLabels.size();
    prob.bias = 0;
    prob.n = NUM_FEATURES + 1;

    prob.y = new int [prob.l];
    prob.x = new struct feature_node * [prob.l];

    for (i = 0; i < trainLabels.size(); i++) {
        prob.x[i] = new struct feature_node [NUM_FEATURES + 1];
        prob.x[i][NUM_FEATURES].index = -1;
        for (j = 0; j < NUM_FEATURES; j++) {
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

    struct model *model = train(&prob, &param);

    if (save_model(coefFileName.toAscii(), model)) {
        qWarning("Cannot save model to file");
    }

    printf("learning time: %lfs\n", double(clock() - t) / CLOCKS_PER_SEC);

    free_and_destroy_model(&model);
    destroy_param(&param);
    delete [] prob.y;
    for (i = 0; i < prob.l; i++) {
        delete [] prob.x[i];
    }
    delete [] prob.x;
}

QVector<QRect> Logic::detect(const QImage& image, struct model *model)
{
    int i, j;
    int width = image.width();
    QVector<int> features;
    QVector<double> prob;
    QVector<int> pos;
    QVector<QRect> answer;
    double p;

    struct feature_node *x = new struct feature_node [NUM_FEATURES + 1];
    x[NUM_FEATURES].index = -1;

    int **hog = calculateHOG(image);

    for (i = 0; i < width - HUMAN_WIDTH; i += STEP) {
        features = getHOG(hog, i, i + HUMAN_WIDTH, 0, image.height());
        for (j = 0; j < NUM_FEATURES; j++) {
            x[j].index = j + 1;
            x[j].value = features[j];
        }
        predict_values(model, x, &p);

        prob.push_back(p);
        pos.push_back(i);

        features.clear();
    }

    delete [] hog;

    double cur_max;
    int i_max;
    int sz;
    sz = prob.size();

 //   printf("%d\n", sz);
/*   for (i = 0; i < sz; i++) {
        if (prob[i] > THRESHOLD) {
       //     printf("%lf\n", prob[i]);
            answer.push_back(QRect(pos[i], 0, HUMAN_WIDTH, HUMAN_HEIGHT));
        }
    }
//    printf("\n");
//    printf("\n");
//    return answer;
//    */

    while (true) {
        cur_max = -1000;
        for (i = 0; i < prob.size(); i++) {
            if (prob[i] > cur_max) {
                cur_max = prob[i];
                i_max = i;
            }
        }
   //     printf("%lf\n", cur_max);
        if (cur_max < THRESHOLD) {
            break;
        }
        answer.push_back(QRect(pos[i_max], 0, HUMAN_WIDTH, HUMAN_HEIGHT));
        prob[i_max] = 0;
        for (i = 0; i < 20; i++) {
            prob[check(i_max - i, sz)] = 0;
            prob[check(i_max + i, sz)] = 0;
        }
    }

    delete [] x;

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
        //printf("Processing %d.png\n", num);
        printf("#");
        fflush(stdout);
    }
    printf("\n");

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
        if (!lbl[j]) {
            printf("%d\n", ans[j].num);
        }
    }

    recall = double(tp1) / double(gt);
    precision = double(tp) / double(tp + fp);

    printf("#TP=%d\n", tp);
    printf("#TP'=%d\n", tp1);
    printf("#FP=%d\n", fp);
    printf("#GT=%d\n", gt);

    double f1 = 2 * double(precision * recall) / double(precision + recall);

    printf("Recall %lf%%\nPrecision %lf%%\n", recall * 100.0, precision * 100.0);
    printf("F1 score: %lf\n", f1 * 100);
}
