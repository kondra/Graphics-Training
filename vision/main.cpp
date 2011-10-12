#include <QApplication>

#include <stdio.h>

#include "logic.h"
#include "main_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList args = app.arguments();

    if (args.size() == 1) {
        printf("You should specify one or more parameters\n");
        return 0;
    }

    if (args.at(1) == "--help") {
        printf( "Usage: vision OPTION [FILE] ...\n"
                "Available options:\n"
                "--learn DIR FILE1 FILE2      learn SVM, where:\n"
                "                               DIR   - directory with training data\n"
                "                               FILE1 - human description file\n"
                "                               FILE2 - where to save SVM model\n"
                "--classify DIR FILE1 FILE2   classify a set of images, where:\n"
                "                               DIR   - directory with test data\n"
                "                               FILE1 - where to save result\n"
                "                               FILE2 - SVM model file\n"
                "--detect FILE1 FILE2         detect human on image, where:\n"
                "                               FILE1 - image file\n"
                "                               FILE2 - SVM model file\n"
                "--evaluate FILE1 FILE2       evaluate classifier, where:\n"
                "                               FILE1 - answer file\n"
                "                               FILE2 - result file (output of SVM)\n"
                "--help    print this message and exit\n"
              );
        return 0;
    } else if (args.at(1) == "--learn") {
        if (args.size() != 5) {
            printf("Wrong parameters\n");
            return 0;
        }
        QString dirName = args.at(2);
        QString descrFileName = args.at(3);
        QString coefFileName = args.at(4);
        Logic::learn(descrFileName, dirName, coefFileName);
    } else if (args.at(1) == "--classify") {
        if (args.size() != 5) {
            printf("Wrong parameters\n");
            return 0;
        }
        QString dirName = args.at(2);
        QString descrFileName = args.at(3);
        QString coefFileName = args.at(4);
        Logic::classify(descrFileName, dirName, coefFileName);
    } else if (args.at(1) == "--detect") {
        if (args.size() != 4) {
            printf("Wrong parameters\n");
            return 0;
        }
        QString imageFileName = args.at(2);
        QString coefFileName = args.at(3);
        bool ok;

        MainWindow win(imageFileName);
        win.show();

        QVector<QRect> result = Logic::detectOne(imageFileName, coefFileName, &ok);

        if (!ok) {
            return 0;
        }

        for (int i = 0; i < result.size(); i++) {
            win.drawRectangle(result[i]);
        }

        return app.exec();
    } else if (args.at(1) == "--evaluate") {
        if (args.size() != 4) {
            printf("Wrong parameters\n");
            return 0;
        }
        QString ansFileName = args.at(2);
        QString resFileName = args.at(3);
        Logic::evaluate(ansFileName, resFileName);
    } else {
        printf("Wrong parameters\n");
    }

    return 0;
}
