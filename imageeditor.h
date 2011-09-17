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
    void channelCorr();
    void linearCorr();
    void gaussian();

private:
    void createActions();
    void createMenus();

    QLabel *imageLabel;
    QScrollArea *scrollArea;

    QAction *openAct;
    QAction *saveAct;
    QAction *exitAct;
    QAction *linearCorrAct;
    QAction *channelCorrAct;
    QAction *gaussianAct;

    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *editMenu;

    ImageLogic *image;
};

#endif // IMAGEEDITOR_H
