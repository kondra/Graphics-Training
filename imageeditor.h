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

private:
    void createActions();
    void createMenus();

    QLabel *imageLabel;
    QScrollArea *scrollArea;

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

    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *editMenu;

    ImageLogic *image;
};

#endif // IMAGEEDITOR_H
