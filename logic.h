#ifndef LOGIC_H
#define LOGIC_H

#include <QImage>

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

public:
    void linearCorrection();
    void channelCorrection();
    void gaussianBlur(double sigma);
    void fastGaussianBlur(double sigma);
    void unsharpMask();
    void glassEffect();
    void wavesEffect();
    void medianFilter(int radius);
    void greyWorld();

};

#endif // LOGIC_H
