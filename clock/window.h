#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QSlider>

class GLWidget;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QSlider *createSlider();

    GLWidget *glWidget;
    QSlider *xSlider;
    QSlider *ySlider;
    QSlider *zSlider;
};

#endif
