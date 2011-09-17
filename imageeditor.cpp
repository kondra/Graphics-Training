#include <QtGui>

#include "imageeditor.h"
#include "logic.h"

#include <QDebug>

ImageEditor::ImageEditor()
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

    setWindowTitle(tr("Image Editor"));
    resize(800, 600);

    image = static_cast<ImageLogic*>(new QImage(tr("test/Lenna.png")));
    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());
    if (!fileName.isEmpty()) {
        image = static_cast<ImageLogic*>(new QImage(fileName));
        if (image->isNull()) {
            QMessageBox::information(this, tr("Image Viewer"), tr("Cannot load %1.").arg(fileName));
            return;
        }
        imageLabel->setPixmap(QPixmap::fromImage(*image));
        imageLabel->adjustSize();
    }
}

void ImageEditor::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath());
    if (!fileName.isEmpty()) {
        if (!image->save(fileName)) {
            QMessageBox::information(this, tr("Image Viewer"), tr("Cannot save %1.").arg(fileName));
            return;
        }
    }
}

void ImageEditor::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    linearCorrAct = new QAction(tr("Autocontrast"), this);
    connect(linearCorrAct, SIGNAL(triggered()), this, SLOT(linearCorr()));

    channelCorrAct = new QAction(tr("Per Channel Autocontrast"), this);
    connect(channelCorrAct, SIGNAL(triggered()), this, SLOT(channelCorr()));

    gaussianAct = new QAction(tr("Gaussian Filter"), this);
    connect(gaussianAct, SIGNAL(triggered()), this, SLOT(gaussian()));
}

void ImageEditor::channelCorr()
{
    image->channelCorrection();

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::linearCorr()
{
    image->linearCorrection();

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::gaussian()
{
    image->gaussianFilter(1);

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    viewMenu = new QMenu(tr("&View"), this);

    editMenu = new QMenu(tr("&Edit"), this);
    editMenu->addAction(linearCorrAct);
    editMenu->addAction(channelCorrAct);
    editMenu->addAction(gaussianAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(editMenu);
}
