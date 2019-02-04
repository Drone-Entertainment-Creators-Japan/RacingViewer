#ifndef ICVIMAGESOURCE_H
#define ICVIMAGESOURCE_H

namespace cv { class Mat; }

class ICVImageSource
{
public:
    virtual ~ICVImageSource(void) {}

    virtual bool getImage(cv::Mat& image) = 0;
};
//inline IVideoSource::~IVideoSource(void) {};

#endif /* ICVIMAGESOURCE_H */
