#include <QtGui>

#include <cmath>

#include <QDebug>

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

    imageLabel->installEventFilter(this);

//    image = static_cast<ImageLogic*>(new QImage(tr("test/Lenna.png")));
    image = new ImageLogic(QImage(tr("test/Lenna.png")));
//    image->init();
    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());
    if (!fileName.isEmpty()) {
 //       image = static_cast<ImageLogic*>(new QImage(fileName));
 //       image->init();
        image = new ImageLogic(QImage(fileName));
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

    bool ok = false;
    double alpha = QInputDialog::getDouble(this, tr("Adjust parameters:"), tr("Alpha:"), 1.5, 0.0, 100.0, 1, &ok);
    if (ok) {
        image->unsharpMask(alpha);

        imageLabel->setPixmap(QPixmap::fromImage(*image));
        imageLabel->adjustSize();
    }
}

void ImageEditor::glass()
{
    if (!image)
        return;

    bool ok = false;
    int radius = QInputDialog::getInt(this, tr("Adjust parameters:"), tr("Radius:"), 10, 1, 50, 1, &ok);
    if (ok) {
        image->glassEffect(radius);

        imageLabel->setPixmap(QPixmap::fromImage(*image));
        imageLabel->adjustSize();
    }
}

void ImageEditor::waves()
{
    if (!image)
        return;

    QDialog *dialog = new QDialog(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QGridLayout *gridLayout = new QGridLayout;

    QLabel *waveLengthLabel = new QLabel(tr("Wavelength:"));
    QLabel *amplitudeLabel = new QLabel(tr("Amplitude:"));

    QDoubleSpinBox *waveLengthBox = new QDoubleSpinBox;
    QDoubleSpinBox *amplitudeBox = new QDoubleSpinBox;

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->addButton(QDialogButtonBox::Cancel);

    gridLayout->addWidget(waveLengthLabel, 0, 0);
    gridLayout->addWidget(waveLengthBox, 0, 1);
    gridLayout->addWidget(amplitudeLabel, 1, 0);
    gridLayout->addWidget(amplitudeBox, 1, 1);

    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(buttonBox);
    dialog->setLayout(mainLayout);

    connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

    int code = dialog->exec();
    if (code == QDialog::Rejected)
        return;

    double wl = waveLengthBox->value();
    double amp = amplitudeBox->value();

    image->wavesEffect(wl, amp);

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

void ImageEditor::userFilter()
{
    if (!image)
        return;

    bool ok;
    int size = QInputDialog::getInt(this, tr("Filter size:"), tr("size:"), 1, 1, 10, 1, &ok);

    if (!ok)
        return;

    QDialog *dialog = new QDialog(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QGridLayout *gridLayout = new QGridLayout;
    QDoubleSpinBox ***widgetsMatrix;
    double **filter;
    int i, j;

    filter = new double*[size];
    widgetsMatrix = new QDoubleSpinBox**[size];
    for (i = 0; i < size; i++) {
        widgetsMatrix[i] = new QDoubleSpinBox*[size];
        filter[i] = new double[size];
    }

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            widgetsMatrix[i][j] = new QDoubleSpinBox;
            gridLayout->addWidget(widgetsMatrix[i][j], i, j);
        }
    }

    QCheckBox *normalizeCheckBox = new QCheckBox(tr("Normalize"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->addButton(QDialogButtonBox::Cancel);

    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(normalizeCheckBox);
    mainLayout->addWidget(buttonBox);
    dialog->setLayout(mainLayout);

    connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

    int code = dialog->exec();
    if (code == QDialog::Rejected)
        return;

    Kernel ker(size, size);
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            ker.kernel[i][j] = widgetsMatrix[i][j]->value();
        }
    }

    if (normalizeCheckBox->checkState() == Qt::Checked) {
        ker.normalize();
    }

    image->userFilter(ker);

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    imageLabel->adjustSize();
}

void ImageEditor::scaling()
{
    if (!image)
        return;

    bool ok = false;
    double scale = QInputDialog::getDouble(this, tr("Adjust parameters:"), tr("Scale:"), 1.0, 0.0, 100.0, 1, &ok);
    if (ok) {
        image->scaling(scale);

        imageLabel->setPixmap(QPixmap::fromImage(*image));
        imageLabel->adjustSize();
    }
}

void ImageEditor::rotation()
{
    if (!image)
        return;

    bool ok = false;
    int alpha = QInputDialog::getInt(this, tr("Adjust parameters:"), tr("Angle(in degrees from -180 to 180):"), 0, -180, 180, 1, &ok);
    if (ok) {
        image->rotation(alpha / 180.0 * M_PI);

        imageLabel->setPixmap(QPixmap::fromImage(*image));
        imageLabel->adjustSize();
    }
}

bool ImageEditor::eventFilter(QObject *someOb, QEvent *ev)
{
    if(someOb == imageLabel) {
        QMouseEvent *mEv = static_cast<QMouseEvent*>(ev);
        if (ev->type() == QEvent::MouseButtonPress) {
            imageLabel->setPixmap(QPixmap::fromImage(*image));
            x1 = mEv->x();
            y1 = mEv->y();
            pixmap = imageLabel->pixmap()->copy();
            return true;
        }
        if (ev->type() == QEvent::MouseButtonRelease) {
            x2 = mEv->x();
            y2 = mEv->y();
            if (x2 == x1 && y2 == y1) {
                image->resetSelection();
                imageLabel->setPixmap(pixmap);
                return true;
            }
            captured = true;
            drawRectangle();
            image->setSelection(x1, y1, x2, y2);
            return true;
        }
        if (ev->type() == QEvent::MouseMove) {
            x2 = mEv->x();
            y2 = mEv->y();
            drawRectangle();
            return true;
        }
    }
    return false;
}

void ImageEditor::drawRectangle()
{
    QPainter painter;
    QPixmap pix = pixmap.copy();
//    QPixmap *pix = const_cast<QPixmap*>(imageLabel->pixmap());
    painter.begin(&pix);
//    painter.begin(pix);
    painter.drawRect(x1, y1, x2 - x1, y2 - y1);
    painter.end();
    imageLabel->setPixmap(pix);
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

    userFilterAct = new QAction(tr("User Defined Filter"), this);
    connect(userFilterAct, SIGNAL(triggered()), this, SLOT(userFilter()));

    scalingAct = new QAction(tr("Scale Image"), this);
    connect(scalingAct, SIGNAL(triggered()), this, SLOT(scaling()));

    rotationAct = new QAction(tr("Rotate Image"), this);
    connect(rotationAct, SIGNAL(triggered()), this, SLOT(rotation()));
}

void ImageEditor::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(scalingAct);
    viewMenu->addAction(rotationAct);

    filtersMenu = new QMenu(tr("F&ilters"), this);
    filtersMenu->addAction(gaussianAct);
    filtersMenu->addAction(fastGaussianAct);
    filtersMenu->addAction(sharpAct);
    filtersMenu->addAction(medianAct);
    filtersMenu->addAction(userFilterAct);

    toolsMenu = new QMenu(tr("&Tools"), this);
    toolsMenu->addAction(autocontrastAct);
    toolsMenu->addAction(autolevelsAct);
    toolsMenu->addAction(greyWorldAct);

    effectsMenu = new QMenu(tr("&Effects"), this);
    effectsMenu->addAction(wavesAct);
    effectsMenu->addAction(glassAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(toolsMenu);
    menuBar()->addMenu(filtersMenu);
    menuBar()->addMenu(effectsMenu);
}
