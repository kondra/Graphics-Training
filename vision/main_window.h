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
    MainWindow(const QString& imageFileName);
    void drawRectangle(QRect rect);

private:
    QLabel *imageLabel;
    QScrollArea *scrollArea;

    QAction *exitAct;

};

#endif
