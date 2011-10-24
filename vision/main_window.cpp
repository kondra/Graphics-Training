#include <QtGui>

#include "main_window.h"

MainWindow::MainWindow(const QString& imageFileName)
{
    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::NoRole);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);

    exitAct = new QAction(this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    setWindowTitle(tr("Vision"));

    QImage image;
    if (!imageFileName.isEmpty()) {
        image = QImage(imageFileName);
        if (image.isNull()) {
            qWarning("Cannot load image file");
            return;
        }
        imageLabel->setPixmap(QPixmap::fromImage(image));
        imageLabel->adjustSize();
    }

    resize(imageLabel->width() + 2, imageLabel->height() + 2);
}

void MainWindow::drawRectangle(QRect rect)
{
    QPainter painter;
    painter.begin(const_cast<QPixmap*>(imageLabel->pixmap()));
    painter.setPen(qRgb(0, 255, 0));
    painter.drawRect(rect);
    painter.end();
}
