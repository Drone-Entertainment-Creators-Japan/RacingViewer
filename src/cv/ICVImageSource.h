#ifndef ICVIMAGESOURCE_H
#define ICVIMAGESOURCE_H

namespace cv { class Mat; }

class ICVImageSource
{
public:
    virtual ~ICVImageSource(void) {}

    virtual bool getImage(cv::Mat& image, bool* p_is_bottom_to_top=nullptr) = 0;
};
//inline IVideoSource::~IVideoSource(void) {};

#endif /* ICVIMAGESOURCE_H */
