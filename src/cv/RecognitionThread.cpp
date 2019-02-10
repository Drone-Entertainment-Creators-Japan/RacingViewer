#include "RecognitionThread.h"

#include "cv/ICVImageSource.h"
#include <QMutexLocker>
#include <QDebug>

/* ------------------------------------------------------------------------------------------------ */
RecognitionThread::RecognitionThread(QObject* p_parent) : QThread(p_parent)
, mp_source( nullptr )
, m_loop   ( true )
, m_dominant_id( 0 )
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
    if( dictionary->bytesList.rows < 0 ) { return false; }
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
    parameters->minMarkerPerimeterRate = 0.05;
    parameters->maxMarkerPerimeterRate = 4.00;

    size_t last_recognized_marker = 0;
    int anti_flicker_count = 0;
    qint64 last_detected_timestamp = 0;
    while( m_loop )
    {
        m_sem.acquire();

        mp_source->getImage(image);

        m_mutex.lock(); /* ----------------------- */
        size_t total_id_count = static_cast<size_t>(m_dictionary->bytesList.rows);
        cv::aruco::detectMarkers(image, m_dictionary, m_detected_corners, m_detected_ids, parameters);
        if( m_detected_id_count.size() < (total_id_count+1) ) { m_detected_id_count.resize( total_id_count + 1 ); } /* first element is used to avoid buffer overrun */

        /* dominant id detection */
        size_t current_dominant_id = 0;
        int dominant_id_count = 0;
        m_detected_id_count[m_dominant_id] = 0;
        for(size_t i=0; i<m_detected_ids.size(); ++i) /* count only detected ids is reseted */
        {
            if( m_detected_ids[i] < 0 ) { continue; }
            m_detected_id_count[static_cast<size_t>( m_detected_ids[i] )] = 0;
        }

        for(size_t i=0; i<m_detected_ids.size(); ++i)
        {
            if( m_detected_ids[i] < 0 ) { continue; }
            if( m_detected_ids[i] >= static_cast<int>(total_id_count) ) { continue; }
            size_t idx = static_cast<size_t>( m_detected_ids[i]+1 );
            m_detected_id_count[idx] += 1;
            if( dominant_id_count >= m_detected_id_count[idx] ) { continue; }
            dominant_id_count = m_detected_id_count[idx];
            current_dominant_id = idx;
        }
        /* prefer last dominant id */
        if( dominant_id_count > m_detected_id_count[m_dominant_id] ) { m_dominant_id = current_dominant_id; }
        m_mutex.unlock(); /* ----------------------- */

        if( m_detected_ids.empty()  )
        {
            if( last_recognized_marker == 0 ) { continue; }
            anti_flicker_count -= 1;
            if( anti_flicker_count > 0) { continue; }
            emit passedThrough(static_cast<int>(last_recognized_marker)-1, last_detected_timestamp, cv::getTickFrequency());
            last_recognized_marker = 0;
            continue;
        }

        /* enableded area checking */
        cv::Point2f center(image.cols/2.f, image.rows/2.f);
        size_t corner = 0;
        double min_edge_length = image.cols * parameters->minMarkerPerimeterRate/2;
        for(; corner<m_detected_corners.size(); ++corner)
        {
            const std::vector<cv::Point2f>& points = m_detected_corners[corner];
            if( points.size() < 4 ) { continue; }
            if( m_detected_ids[corner] != (static_cast<int>(m_dominant_id)-1) ) { continue; }
            cv::Point2f vec_a = points[1] - center;
            cv::Point2f vec_b = points[0] - center;

            double max_edge_length = cv::norm(points[0] - points[3]);
            for(size_t i=1; i<points.size(); ++i)
            {
                double len = cv::norm(points[i]-points[i-1]);
                if( len > max_edge_length ) { max_edge_length = len; }
            }
            if( max_edge_length < min_edge_length ) { break; } /* marker is too small */

            if( (vec_a.x*vec_b.y) < (vec_a.y*vec_b.x) ) { break; } /* if vector product < 0, break this loop */
        }
        if( corner < m_detected_corners.size() )
        {
            last_recognized_marker = 0;
        } else if( dominant_id_count >= 2 )
        {
            last_recognized_marker = m_dominant_id;
        }

        if( m_detected_id_count[last_recognized_marker] > 0 )
        {
            last_detected_timestamp = cv::getTickCount();
            anti_flicker_count = 7;
        }


    }

    m_detected_ids.clear();
    m_detected_corners.clear();
}

