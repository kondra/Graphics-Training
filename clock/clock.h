#ifndef CLOCK_H
#define CLOCK_H

#include <QObject>
#include <QColor>

struct Geometry;
class Patch;
class Rectoid;
class RectPrism;
class RectTorus;

class Clock : public QObject
{
public:
    Clock(QObject *parent, int d = 128, qreal s = 1.0);
    ~Clock();
    void draw();
    void loadTex(GLuint _glassTexture, GLuint _metalTexture);
private:
    void buildGeometry(int d, qreal s);
    void translatePointers(bool direction);
    void rotatePointers();

    qreal secondAngle;
    qreal minuteAngle;
    qreal hourAngle;

    RectPrism *secondPointer;
    RectPrism *minutePointer;
    RectPrism *hourPointer;

    RectPrism **hourMarks;
    RectPrism **minuteMarks;

    RectTorus *body;
    RectTorus *center;
    RectTorus *center2;

    Geometry *geom;

    GLuint glassTexture;
    GLuint metalTexture;
};

#endif // CLOCK_H
