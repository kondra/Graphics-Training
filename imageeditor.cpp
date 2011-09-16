#include <QtGui>

#include "imageeditor.h"
#include "logic.h"

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

    image = new QImage(tr("Lenna.png"));
    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());
    if (!fileName.isEmpty()) {
        image = new QImage(fileName);
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

    lCorrAct = new QAction(tr("Autocontrast"), this);
    connect(lCorrAct, SIGNAL(triggered()), this, SLOT(linearCorrection()));

    chCorAct = new QAction(tr("Per Channel Autocontrast"), this);
    connect(chCorAct, SIGNAL(triggered()), this, SLOT(channelLinearCorrection()));
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
    editMenu->addAction(lCorrAct);
    editMenu->addAction(chCorAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(editMenu);
}

int ImageEditor::checkBound(int i) {
    if (i < 0)
        return 0;
    if (i > 255)
        return 255;
    return i;
}

void ImageEditor::linearCorrection()
{
    int lmax = 0;
    int lmin = INT_MAX;

    for (int x = 0; x < image->width(); x++) {
        for (int y = 0; y < image->height(); y++) {
            QRgb p = image->pixel(x, y);

            int l = 0.2125d * qRed(p) + 0.7154d * qGreen(p) + 0.0721d * qBlue(p);

            if (l > lmax)
                lmax = l;
            if (l < lmin)
                lmin = l;
        }
    }

    for (int x = 0; x < image->width(); x++) {
        for (int y = 0; y < image->height(); y++) {
            QRgb p = image->pixel(x, y);

            int r = checkBound((qRed(p) - lmin) * 255 / (lmax - lmin));
            int g = checkBound((qGreen(p) - lmin) * 255 / (lmax - lmin));
            int b = checkBound((qBlue(p) - lmin) * 255 / (lmax - lmin));

            image->setPixel(x, y, qRgb(r, g, b));
        }
    }

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::channelLinearCorrection()
{
    int rmax, bmax, gmax;
    int rmin, bmin, gmin;
    int r, g, b;

    rmax = bmax = gmax = 0;
    rmin = bmin = gmin = INT_MAX;

    for (int x = 0; x < image->width(); x++) {
        for (int y = 0; y < image->height(); y++) {
            QRgb p = image->pixel(x, y);

            r = qRed(p);
            g = qGreen(p);
            b = qBlue(p);

            if (r > rmax)
                rmax = r;
            if (r < rmin)
                rmin = r;

            if (g > gmax)
                gmax = g;
            if (g < gmin)
                gmin = g;

            if (b > bmax)
                bmax = b;
            if (b < bmin)
                bmin = b;
        }
    }

    for (int x = 0; x < image->width(); x++) {
        for (int y = 0; y < image->height(); y++) {
            QRgb p = image->pixel(x, y);

            int r = checkBound((qRed(p) - rmin) * 255 / (rmax - rmin));
            int g = checkBound((qGreen(p) - gmin) * 255 / (gmax - gmin));
            int b = checkBound((qBlue(p) - bmin) * 255 / (bmax - bmin));

            image->setPixel(x, y, qRgb(r, g, b));
        }
    }

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}
