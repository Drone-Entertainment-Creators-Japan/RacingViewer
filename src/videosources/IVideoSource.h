#ifndef IVIDEOSOURCE_H
#define IVIDEOSOURCE_H

#include <QVariant>
class QPainter;
class QRect;

class IVideoSource
{
public:
    virtual ~IVideoSource(void) {}
    virtual void paint(QPainter* p_painter, const QRect& target_rect) = 0;
    virtual QVariant  value(int property) const = 0;
    virtual bool   setValue(int property, const QVariant& value) = 0;
    virtual Qt::ItemFlags itemFlags(int property) const = 0;

    virtual QString deviceID(void) const = 0;
};
//inline IVideoSource::~IVideoSource(void) {};

#endif /* IVIDEOSOURCE_H */
