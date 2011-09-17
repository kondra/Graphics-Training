#ifndef LOGIC_H
#define LOGIC_H

#include <QImage>

class ImageLogic : public QImage {
private:
    int checkHeight(int x);
    int checkWidth(int x);
public:
    void linearCorrection();
    void channelCorrection();
    void gaussianFilter(int sigma);

};

#endif // LOGIC_H
