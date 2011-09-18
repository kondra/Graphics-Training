#ifndef LOGIC_H
#define LOGIC_H

#include <QImage>

class ImageLogic : public QImage {
private:
    int check(int x, int b);
public:
    void linearCorrection();
    void channelCorrection();
    void gaussianFilter(double sigma);

};

#endif // LOGIC_H
