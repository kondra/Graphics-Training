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
    resize(600, 600);

    image = 0;

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

void ImageEditor::autolevels()
{
    if (!image)
        return;

    image->channelCorrection();

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::autocontrast()
{
    if (!image)
        return;

    image->linearCorrection();

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::gaussian()
{
    if (!image)
        return;

    bool ok = false;
    double sigma = QInputDialog::getDouble(this, tr("Adjust parameters:"), tr("Sigma:"), 0.2, 0.2, 5.0, 1, &ok);
    if (ok) {
        image->gaussianBlur(sigma);

        imageLabel->setPixmap(QPixmap::fromImage(*image));
        imageLabel->adjustSize();
    }
}

void ImageEditor::fastGaussian()
{
    if (!image)
        return;

    bool ok = false;
    double sigma = QInputDialog::getDouble(this, tr("Adjust parameters:"), tr("Sigma:"), 0.2, 0.2, 5.0, 1, &ok);
    if (ok) {
        image->fastGaussianBlur(sigma);

        imageLabel->setPixmap(QPixmap::fromImage(*image));
        imageLabel->adjustSize();
    }
}

void ImageEditor::sharp()
{
    if (!image)
        return;

    image->unsharpMask();

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::glass()
{
    if (!image)
        return;

    image->glassEffect();

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::waves()
{
    if (!image)
        return;

    image->wavesEffect();

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::median()
{
    if (!image)
        return;

    bool ok = false;
    int radius = QInputDialog::getInt(this, tr("Adjust parameters:"), tr("Sigma:"), 1, 1, 30, 1, &ok);
    if (ok) {
        image->medianFilter(radius);

        imageLabel->setPixmap(QPixmap::fromImage(*image));
        imageLabel->adjustSize();
    }
}

void ImageEditor::greyWorld()
{
    if (!image)
        return;

    image->greyWorld();

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
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

    autocontrastAct = new QAction(tr("Autocontrast"), this);
    autocontrastAct->setShortcut(tr("Ctrl+A"));
    connect(autocontrastAct, SIGNAL(triggered()), this, SLOT(autocontrast()));

    autolevelsAct = new QAction(tr("Autolevels"), this);
    autolevelsAct->setShortcut(tr("Ctrl+L"));
    connect(autolevelsAct, SIGNAL(triggered()), this, SLOT(autolevels()));

    gaussianAct = new QAction(tr("Gaussian Blur"), this);
    gaussianAct->setShortcut(tr("Ctrl+B"));
    connect(gaussianAct, SIGNAL(triggered()), this, SLOT(gaussian()));

    fastGaussianAct = new QAction(tr("Fast Gaussian Blur"), this);
    fastGaussianAct->setShortcut(tr("Ctrl+Shift+B"));
    connect(fastGaussianAct, SIGNAL(triggered()), this, SLOT(fastGaussian()));

    sharpAct = new QAction(tr("Unsharp Mask"), this);
    sharpAct->setShortcut(tr("Ctrl+U"));
    connect(sharpAct, SIGNAL(triggered()), this, SLOT(sharp()));

    glassAct = new QAction(tr("Glass"), this);
    connect(glassAct, SIGNAL(triggered()), this, SLOT(glass()));

    wavesAct = new QAction(tr("Waves"), this);
    connect(wavesAct, SIGNAL(triggered()), this, SLOT(waves()));

    medianAct = new QAction(tr("Median Filter"), this);
    medianAct->setShortcut(tr("Ctrl+M"));
    connect(medianAct, SIGNAL(triggered()), this, SLOT(median()));

    greyWorldAct = new QAction(tr("Grey World"), this);
    connect(greyWorldAct, SIGNAL(triggered()), this, SLOT(greyWorld()));
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
    editMenu->addAction(autocontrastAct);
    editMenu->addAction(autolevelsAct);
    editMenu->addAction(gaussianAct);
    editMenu->addAction(fastGaussianAct);
    editMenu->addAction(sharpAct);
    editMenu->addAction(wavesAct);
    editMenu->addAction(glassAct);
    editMenu->addAction(medianAct);
    editMenu->addAction(greyWorldAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(editMenu);
}
