#include "RecognitionThread.h"

#include "cv/ICVImageSource.h"
#include <QMutexLocker>
#include <QDebug>

/* ------------------------------------------------------------------------------------------------ */
RecognitionThread::RecognitionThread(QObject* p_parent) : QThread(p_parent)
, mp_source( nullptr )
, m_loop   ( true )
{

}

/* ------------------------------------------------------------------------------------------------ */
RecognitionThread::~RecognitionThread(void)
{
    stop();
}

/* ------------------------------------------------------------------------------------------------ */
bool RecognitionThread::setDictonary(cv::Ptr<cv::aruco::Dictionary> dictionary)
{
    QMutexLocker locker(&m_mutex);
    m_dictionary = dictionary;

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool RecognitionThread::setVideoSource(ICVImageSource* p_source)
{
    mp_source = p_source;
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
void RecognitionThread::stop(void)
{
    m_mutex.lock();
    m_loop = false;
    m_mutex.unlock();

    m_sem.release();
    wait();
    terminate();
}

/* ------------------------------------------------------------------------------------------------ */
void RecognitionThread::update(void)
{
    if( m_sem.available() != 0 ) { return; }
    m_sem.release();
}

/* ------------------------------------------------------------------------------------------------ */
void RecognitionThread::getDetectedParameters(QVector<int>* p_ids, QVector<QPoint>* p_corners)
{
    QMutexLocker locker(&m_mutex);

    if( p_ids )
    {
        p_ids->fromStdVector( m_detected_ids );
    }

    if( p_corners )
    {
        p_corners->resize( static_cast<int>(m_detected_corners.size()*4) );
        QVector<QPoint>::iterator dst = p_corners->begin();
        std::vector<std::vector<cv::Point2f>>::iterator src = m_detected_corners.begin();
        for(; dst != p_corners->end(); dst+=4, src+=1)
        {
            if( src->size() < 4 ) { continue; }
            (dst  )->setX(  static_cast<int>((*src)[0].x) );
            (dst  )->setY(  static_cast<int>((*src)[0].y) );
            (dst+1)->setX(  static_cast<int>((*src)[1].x) );
            (dst+1)->setY(  static_cast<int>((*src)[1].y) );
            (dst+2)->setX(  static_cast<int>((*src)[2].x) );
            (dst+2)->setY(  static_cast<int>((*src)[2].y) );
            (dst+3)->setX(  static_cast<int>((*src)[3].x) );
            (dst+3)->setY(  static_cast<int>((*src)[3].y) );
        }
    }
}

/* ------------------------------------------------------------------------------------------------ */
void RecognitionThread::run(void)
{
    m_mutex.lock();
    m_loop = true;
    m_mutex.unlock();

    if( ! mp_source ) { return; }
    cv::Mat image;

    cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
    parameters->markerBorderBits = 1;
    parameters->adaptiveThreshWinSizeMin  =  3;
    parameters->adaptiveThreshWinSizeMax  = 30;
    parameters->adaptiveThreshWinSizeStep =  3;
    parameters->minMarkerPerimeterRate = 0.1;
    parameters->maxMarkerPerimeterRate = 4.00;

    int marker_id = -1;
    int anti_flicker_count = 0;
    while( m_loop )
    {
        m_sem.acquire();

        mp_source->getImage(image);

        m_mutex.lock();
        cv::aruco::detectMarkers(image, m_dictionary, m_detected_corners, m_detected_ids, parameters);
        m_mutex.unlock();

        if( m_detected_corners.empty() )
        {
            if( marker_id < 0 ) { continue; }
            anti_flicker_count -= 1;
            if( anti_flicker_count > 0) { continue; }
            emit passedThrough(marker_id, cv::getTickCount(), cv::getTickFrequency());
            marker_id = -1;
            continue;
        }

        if( m_detected_corners.size() < 2 ) { continue; }

        /* gate detection */
        cv::Point2f center(image.cols/2.f, image.rows/2.f);
        std::vector<std::vector<cv::Point2f>>::iterator corner = m_detected_corners.begin();
        for(; corner!=m_detected_corners.end(); ++corner)
        {
            cv::Point2f vec_a = (*corner)[1] - center;
            cv::Point2f vec_b = (*corner)[0] - center;

            /* if vector product < 0 break this loop */
            if( (vec_a.x*vec_b.y) < (vec_a.y*vec_b.x) ) { break; }
        }
        if( corner == m_detected_corners.end() )
        {
            marker_id = m_detected_ids[0];
            anti_flicker_count = 7;
        } else
        {
            marker_id = -1;
        }
    }

    m_detected_ids.clear();
    m_detected_corners.clear();
}

