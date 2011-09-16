#ifndef LOGIC_H
#define LOGIC_H

class ImageLogic : public QImage {
public:
    linearCorrection();
    channelCorrection();

};

#endif // LOGIC_H
