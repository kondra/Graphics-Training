#include <QtGui>

#include <QDebug>

#include "main_window.h"
#include "logic.h"

MainWindow::MainWindow()
{
    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::NoRole);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);

    createActions();
    createMenus();

    setWindowTitle(tr("Vision"));
    resize(800, 400);
}

void MainWindow::learn()
{
//    QString descrFileName = QFileDialog::getOpenFileName(this, tr("Open Description File"), QDir::currentPath());
//    QString coefFileName = QFileDialog::getSaveFileName(this, tr("Save Model File"), QDir::currentPath());
//    QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Train Directory"), QDir::currentPath());
//    if (descrFileName.isNull() || dirName.isNull() || coefFileName.isNull())
//        return;
//    Logic::learn(descrFileName, dirName, coefFileName);
    Logic::learn("train/train-processed.idl", "train", "learn.txt");
}

void MainWindow::evaluate()
{
//    QString ansFileName = QFileDialog::getOpenFileName(this, tr("Open Answer File"), QDir::currentPath());
//    QString resFileName = QFileDialog::getOpenFileName(this, tr("Open Result File"), QDir::currentPath());
//    if (ansFileName.isNull() || resFileName.isNull())
//        return;
//    Logic::evaluate(ansFileName, resFileName);
    Logic::evaluate("test/test-processed.idl", "result.txt");
}

void MainWindow::classify()
{
//    QString coefFileName = QFileDialog::getOpenFileName(this, tr("Open Model File"), QDir::currentPath());
//    QString descrFileName = QFileDialog::getSaveFileName(this, tr("Save Description File"), QDir::currentPath());
//    QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Test Directory"), QDir::currentPath());
//    if (descrFileName.isNull() || dirName.isNull() || coefFileName.isNull())
//        return;
//    Logic::classify(descrFileName, dirName, coefFileName);
    Logic::classify("result.txt", "test", "learn.txt");
}

void MainWindow::detect()
{
    bool ok;
    QString imageFileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());
    if (!imageFileName.isEmpty()) {
        QImage image(imageFileName);
        if (image.isNull()) {
            QMessageBox::information(this, tr("Vision"), tr("Cannot load %1.").arg(imageFileName));
            return;
        }
        imageLabel->setPixmap(QPixmap::fromImage(image));
        imageLabel->adjustSize();
    }

//    QString coefFileName = QFileDialog::getOpenFileName(this, tr("Open Model File"), QDir::currentPath());
//    if (coefFileName.isNull())
//        return;
//    QVector<QRect> result = Logic::detectOne(imageFileName, coefFileName, &ok);
    QVector<QRect> result = Logic::detectOne(imageFileName, "learn.txt", &ok);

    //////
//    QImage image("test/7.png");
//    imageLabel->setPixmap(QPixmap::fromImage(image));
//    imageLabel->adjustSize();
//
//    QVector<QRect> result = Logic::detectOne("test/7.png", "learn.txt", &ok);
    /////

    if (!ok) {
        return;
    }

    for (int i = 0; i < result.size(); i++) {
        drawRectangle(result[i]);
    }
}

void MainWindow::drawRectangle(QRect rect)
{
    QPainter painter;
    painter.begin(const_cast<QPixmap*>(imageLabel->pixmap()));
    painter.setPen(qRgb(0, 255, 0));
    painter.drawRect(rect);
    painter.end();
}

void MainWindow::createActions()
{
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    learnAct = new QAction(tr("Learn"), this);
    learnAct->setShortcut(tr("Ctrl+L"));
    connect(learnAct, SIGNAL(triggered()), this, SLOT(learn()));

    detectAct = new QAction(tr("Detect"), this);
    detectAct->setShortcut(tr("Ctrl+D"));
    connect(detectAct, SIGNAL(triggered()), this, SLOT(detect()));

    classifyAct = new QAction(tr("Classify"), this);
    classifyAct->setShortcut(tr("Ctrl+C"));
    connect(classifyAct, SIGNAL(triggered()), this, SLOT(classify()));
    
    evaluateAct = new QAction(tr("Evaluate"), this);
    evaluateAct->setShortcut(tr("Ctrl+E"));
    connect(evaluateAct, SIGNAL(triggered()), this, SLOT(evaluate()));
}

void MainWindow::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(exitAct);

    processMenu = new QMenu(tr("&Process"), this);
    processMenu->addAction(learnAct);
    processMenu->addAction(detectAct);
    processMenu->addAction(classifyAct);
    processMenu->addAction(evaluateAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(processMenu);
}
