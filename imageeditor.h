#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <QMainWindow>
#include <QScrollArea>
#include <QLabel>
#include <QMenu>
#include <QAction>

#include "logic.h"

class ImageEditor : public QMainWindow
{
    Q_OBJECT

public:
    ImageEditor();

private slots:
    void open();
    void save();
    void autolevels();
    void autocontrast();
    void gaussian();
    void fastGaussian();
    void sharp();
    void glass();
    void waves();
    void median();
    void greyWorld();
    void userFilter();
    void scaling();
    void rotate();

protected:
    bool eventFilter(QObject *someOb, QEvent *ev);

private:
    void createActions();
    void createMenus();
    void drawRectangle();

    QLabel *imageLabel;
    QScrollArea *scrollArea;
    QPixmap pixmap;

    QAction *openAct;
    QAction *saveAct;
    QAction *exitAct;
    QAction *autolevelsAct;
    QAction *autocontrastAct;
    QAction *gaussianAct;
    QAction *fastGaussianAct;
    QAction *sharpAct;
    QAction *glassAct;
    QAction *wavesAct;
    QAction *medianAct;
    QAction *greyWorldAct;
    QAction *userFilterAct;
    QAction *scalingAct;
    QAction *rotationAct;

    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *filtersMenu;
    QMenu *toolsMenu;
    QMenu *effectsMenu;

    ImageLogic *image;

    int x1, y1, x2, y2;
    bool captured;
};

#endif // IMAGEEDITOR_H
