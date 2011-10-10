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
    void learn();
    void detect();
    void classify();
    void evaluate();

private:
    void createActions();
    void createMenus();
    void drawRectangle(QRect rect);

    QLabel *imageLabel;
    QScrollArea *scrollArea;

    QAction *exitAct;
    QAction *learnAct;
    QAction *detectAct;
    QAction *classifyAct;
    QAction *evaluateAct;

    QMenu *fileMenu;
    QMenu *processMenu;
};

#endif
