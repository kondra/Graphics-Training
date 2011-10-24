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

inline double Logic::getBrightness(QRgb p)
{
    return qRed(p) * RED_INTENSE +
           qGreen(p) * GREEN_INTENSE +
           qBlue(p) * BLUE_INTENSE;
}

inline int Logic::check(int x, int n)
{
    if (x >= n)
        return n - 1;
    if (x < 0)
        return 0;
    return x;
}

inline int Logic::checkBound(int x)
{
    if (x > 255)
        return 255;
    if (x < 0)
        return 0;
    return x;
}

void Logic::Gauss(QImage& image)
{
    const int size = 5;
    int x, y, k, l, n, m;
    double rsum, gsum, bsum;
    QRgb p;

    double gauss[size] = { 0.05448868454964295172,
                           0.24420134200323337370,
                           0.40261994689424751570,
                           0.24420134200323337370,
                           0.05448868454964295172
                         };

    QImage original(image);

    int w = original.width();
    int h = original.height();

    QRgb *line;

    for (x = 0; x < w; x++) {
        for (y = 0; y < h; y++) {
            rsum = gsum = bsum = 0.0;
            k = 0;
            m = check(y - (k - size / 2), h);
            line = (QRgb*)original.scanLine(m);
            for (l = 0; l < size; l++) {
                n = check(x - (l - size / 2), w);
                p = line[n];
                rsum += gauss[l] * qRed(p);
                gsum += gauss[l] * qGreen(p);
                bsum += gauss[l] * qBlue(p);
            }
            line = (QRgb*)image.scanLine(y);
            line[x] = qRgb(checkBound(rsum), checkBound(gsum), checkBound(bsum));
        }
    }

    original = image;

    for (x = 0; x < w; x++) {
        for (y = 0; y < h; y++) {
            rsum = gsum = bsum = 0.0;
            l = 0;
            n = check(x - (l - size / 2), w);
            for (k = 0; k < size; k++) {
                m = check(y - (k - size / 2), h);
                line = (QRgb*)original.scanLine(m);
                p = line[n];
                rsum += gauss[k] * qRed(p);
                gsum += gauss[k] * qGreen(p);
                bsum += gauss[k] * qBlue(p);
            }
            line = (QRgb*)image.scanLine(y);
            line[x] = qRgb(checkBound(rsum), checkBound(gsum), checkBound(bsum));
        }
    }
}

int** Logic::calculateHOG(QImage& image)
{
    Gauss(image);

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
            gradX = getBrightness(image.pixel(check(i + 1, width), check(j, height))) - getBrightness(image.pixel(check(i - 1, width), check(j, height)));
            gradY = getBrightness(image.pixel(check(i, width), check(j + 1, height))) - getBrightness(image.pixel(check(i, width), check(j - 1, height)));
            gradX /= 2.;
            gradY /= 2.;
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

    return nonlinear(hist);
}

QVector<int> Logic::nonlinear(QVector<int>& x)
{
    QVector<int> res = x; //FIXME
    double k, v, l, z;
    int i, j;
    for (i = 0; i < x.size(); i++) {
        z = x[i];
        for (j = -APPROX_ORDER; j <= APPROX_ORDER; j++) {
            if (z == 0) {
                res.push_back(0);
                res.push_back(0);
                continue;
            }
            l = j * APPROX_STEP;
            k = 1 / cosh(PI * l);
            v = sqrt(z * k);
            res.push_back(cos(-l * log(z)) * v * 10);
            res.push_back(sin(-l * log(z)) * v * 10);
        }
    }
    return res;
}

bool operator<(const Descr& a,const Descr& b) {
    return a.num < b.num;
}

void Logic::bootStrap(const QString& descrFileName, const QString& dirName, const QString& coefFileName)
{
    learn(descrFileName, dirName, coefFileName);
    QVector<Descr> res = classify("", dirName, coefFileName);
    QVector<Descr> ans = inputDescr(descrFileName);
    QVector<Descr> fp = eval(ans, res);
    learn(descrFileName, dirName, coefFileName, fp);
}

void Logic::outputDescr(const QString& fileName, QVector<Descr>& description)
{
    int j;
    Descr tmp;
    if (fileName.length() == 0)
        return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning("Cannot create description file");
        return;
    }
    QTextStream out(&file);
    qSort(description.begin(), description.end());
    for (j = 0; j < description.size(); j++) {
        tmp = description[j];
        out << tmp.num << " " << tmp.y0 << " " << tmp.x0 << " " << tmp.y1 << " " << tmp.x1 << "\n";
    }
    file.close();
}

QVector<Descr> Logic::inputDescr(const QString& fileName)
{
    QVector<Descr> description;
    Descr tmp;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Cannot open description file");
        return QVector<Descr>();
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        in >> tmp.num >> tmp.y0 >> tmp.x0 >> tmp.y1 >> tmp.x1;
        description.push_back(tmp);
    }
    file.close();
    description.pop_back();
    qSort(description.begin(), description.end());
    return description;
}

void Logic::learn(const QString& descrFileName, const QString& dirName, const QString& coefFileName, const QVector<Descr>& fp)
{
    QDir dir(dirName);
    QImage image;

    if (!dir.exists()) {
        qWarning("Cannot find directory");
        return;
    }

    int t = clock();

    dir.setFilter(QDir::Files | QDir::Readable);
    QStringList list = dir.entryList();

    QVector<Descr> description = inputDescr(descrFileName);

    QString curName;
    bool ok;
    int num;
    int i, j, j1, j2;
    int w;

    int **hog;

    QVector<QVector<int> > trainFeatures;
    QVector<int> trainLabels;

    bool flagF = false;

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
        hog = calculateHOG(image);

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

        if (j1 != -1) {
            for (j = j1; j < j2; j++) {
                if (!flagF) {
                    trainFeatures.push_front(getHOG(hog, description[j].x0, description[j].x1, 0, image.height()));
                    trainLabels.push_front(1);
                    flagF = true;
                    continue;
                }
                trainFeatures.push_back(getHOG(hog, description[j].x0, description[j].x1, 0, image.height()));
                trainLabels.push_back(1);
            }
            w = image.width();
            bool f1 = false;
            if (description[j1].x0 + 2 * HUMAN_WIDTH + 1 < w) {
                trainFeatures.push_back(getHOG(hog, description[j1].x0 + HUMAN_WIDTH + 1, description[j1].x0 + 2 * HUMAN_WIDTH + 1, 0, image.height()));
                f1 = true;
            } else if (description[j1].x0 - HUMAN_WIDTH - 1 >= 0) {
                trainFeatures.push_back(getHOG(hog, description[j1].x0 - HUMAN_WIDTH - 1, description[j1].x0 - 1, 0, image.height()));
                f1 = true;
            }
            if (f1)
                trainLabels.push_back(-1);
        }

        if (fp.size()) {
            j1 = -1;
            j2 = 0;
            for (j = 0; j < fp.size(); j++) {
                if (fp[j].num == num) {
                    j1 = j;
                }
                if (j1 != -1 && fp[j].num != num) {
                    j2 = j;
                    break;
                }
            }
            if (j == fp.size()) {
                j2 = j;
            }
            if (j1 != -1) {
                for (j = j1; j < j2; j++) {
                    trainFeatures.push_back(getHOG(hog, fp[j].x0, fp[j].x1, 0, image.height()));
                    trainLabels.push_back(-1);
                }
            }
        }

        delete [] hog;

        printf("#");
        fflush(stdout);
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

    printf("1\n");

    struct model *model = train(&prob, &param);

    printf("1\n");

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

QVector<QRect> Logic::detect(QImage& image, struct model *model)
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

        if (p > THRESHOLD) {
            prob.push_back(p);
            pos.push_back(i);
        }

        features.clear();
    }

    delete [] hog;

    supressNonMax(answer, prob, pos);

    delete [] x;

    return answer;
}

void Logic::supressNonMax(QVector<QRect>& answer, QVector<double>& prob, QVector<int> pos)
{
    double cur_max;
    int i_max;
    int sz;
    sz = prob.size();

    bool f;
    int i;

    double thrs = THRESHOLD;
    while (true) {
        cur_max = 0;
        for (i = 0; i < prob.size(); i++) {
            if (prob[i] > cur_max) {
                cur_max = prob[i];
                i_max = i;
            }
        }
        if (cur_max < thrs) {
            break;
        }
        answer.push_back(QRect(pos[i_max], 0, HUMAN_WIDTH, HUMAN_HEIGHT));

        i = 1;
        f = true;
        while (f) {
            f = false;
            if (i_max - i >= 0 && pos[i_max] - pos[i_max - i] < HUMAN_WIDTH) {
                prob[i_max - i] = 0;
                f = true;
            }
            if (i_max + i < sz && pos[i_max + i] - pos[i_max] < HUMAN_WIDTH) {
                prob[i_max + i] = 0;
                f = true;
            }
            i++;
        }
        prob[i_max] = 0;
    }
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

QVector<Descr> Logic::classify(const QString& descrFileName, const QString& dirName, const QString& coefFileName)
{
    QDir dir(dirName);
    QImage image;
    struct model *model;

    if (!dir.exists()) {
        qWarning("Cannot find directory");
        return QVector<Descr>();
    }

    if ((model = load_model(coefFileName.toAscii())) == 0) {
        qWarning("Cannot load model from file");
        return QVector<Descr>();
    }

    dir.setFilter(QDir::Files | QDir::Readable);
    QStringList list = dir.entryList();

    QVector<Descr> description;

    QString curName;
    QVector<QRect> result;

    int j, i, num;
    bool ok;

    int t = clock();

    Descr tmp;

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

    outputDescr(descrFileName, description);

    printf("classifieing time %lfs\n", double(clock() - t) / CLOCKS_PER_SEC);

    free_and_destroy_model(&model);

    return description;
}

QVector<Descr> Logic::eval(const QVector<Descr>& ans, const QVector<Descr>& res)
{
    double precision, recall;
    int i, j, k;
    QVector<bool> lbl(ans.size());

    int gt, tp, fp, tp1;
    bool ok;
    gt = tp = fp = tp1 = 0;

    gt = ans.size();

    QVector<Descr> falsePositive;

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
            falsePositive.push_back(res[i]);
//            printf("%d\n", res[i].num);
        }
    }
//    printf("\n");

    for (j = 0; j < lbl.size(); j++) {
        tp1 += lbl[j];
//        if (!lbl[j]) {
//            printf("%d\n", ans[j].num);
//        }
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

    qSort(falsePositive.begin(), falsePositive.end());

    return falsePositive;
}

QVector<Descr> Logic::evaluate(const QString& ansFileName, const QString& resFileName)
{
    QVector<Descr> ans;
    QVector<Descr> res;

    ans = inputDescr(ansFileName);
    res = inputDescr(resFileName);

    return eval(ans, res);
}
