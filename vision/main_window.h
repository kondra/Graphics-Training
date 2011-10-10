#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QLabel>
#include <QMenu>
#include <QAction>

#include "logic.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void open();
    void learn();

private:
    void createActions();
    void createMenus();
    void drawRectangle();

    QLabel *imageLabel;
    QScrollArea *scrollArea;
    QPixmap pixmap;

    QAction *openAct;
    QAction *exitAct;
    QAction *learnAct;

    QMenu *fileMenu;
    QMenu *processMenu;

    QImage *image;
};

#endif
