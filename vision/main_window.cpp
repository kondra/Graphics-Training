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

    image = 0;
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());
    if (!fileName.isEmpty()) {
        image = new QImage(fileName);
        if (image->isNull()) {
            QMessageBox::information(this, tr("Vision"), tr("Cannot load %1.").arg(fileName));
            return;
        }
        imageLabel->setPixmap(QPixmap::fromImage(*image));
        imageLabel->adjustSize();
    }
}

void MainWindow::learn()
{
//    QString descrFileName = QFileDialog::getOpenFileName(this, tr("Open Description File"), QDir::currentPath());
//    QString coefFileName = QFileDialog::getSaveFileName(this, tr("Choose Coeff File"), QDir::currentPath());
//    QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Train Directory"), QDir::currentPath());
//    if (descrFileName.isNull() || dirName.isNull() || coefFileName.isNull())
//        return;
//    Logic::learn(descrFileName, dirName, coefFileName);
    Logic::learn("train/train-processed.idl", "train", "learn.txt");
}

void MainWindow::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    learnAct = new QAction(tr("Learn"), this);
    learnAct->setShortcut(tr("Ctrl+L"));
    connect(learnAct, SIGNAL(triggered()), this, SLOT(learn()));
}

void MainWindow::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    processMenu = new QMenu(tr("Process"), this);
    processMenu->addAction(learnAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(processMenu);
}
