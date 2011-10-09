#ifndef LOGIC_H
#define LOGIC_H

#include <QImage>

const double RED_INTENSE = 0.2125;
const double GREEN_INTENSE = 0.7154;
const double BLUE_INTENSE = 0.0721;
const int LIGHT_MAX = 256;

struct Kernel {
    double **kernel;
    int width;
    int height;

    Kernel(int width, int height);
    ~Kernel();
    Kernel(const Kernel& ker);
    Kernel& operator=(const Kernel& ker);

    Kernel& operator-(const Kernel& ker);
    Kernel& operator+(const Kernel& ker);
    friend Kernel& operator*(int alpha, const Kernel& ker);

    void reverse();
    void normalize();

    static Kernel id(int size);
};

class ImageLogic : public QImage {
private:
    Kernel gaussKernel(double sigma);
    void convolution(Kernel& ker);
    QRgb bilinearInterpolation(const QImage& original, double xOld, double yOld, int xFloor, int xCeil, int yFloor, int yCeil);
    void fillSelection();
    int getLuminosity(int r, int g, int b);

    bool selection;
    int x1, y1, x2, y2;

public:
    ImageLogic(const QImage& image);
    void linearCorrection();
    void linearHSVCorrection();
    void channelCorrection();
    void gaussianBlur(double sigma);
    void fastGaussianBlur(double sigma);
    void unsharpMask(double alpha);
    void glassEffect(int radius);
    void wavesEffect(double waveLength, double amplitude);
    void medianFilter(int radius);
    void greyWorld();
    void userFilter(Kernel& ker);
    void scaling(double scale);
    void scalingSelection(double scale);
    void rotate(double alpha);
    void setSelection(int _x1, int _y1, int _x2, int _y2);
    void resetSelection();
    void rotateSelection(double alpha);

};

#endif // LOGIC_H
