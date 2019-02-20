#ifndef RECOGNITIONTHREAD_H
#define RECOGNITIONTHREAD_H

#include <QThread>
#include <QSemaphore>
#include <QPoint>
#include <QMutex>
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>

#include <stdint.h>
#include <vector>

class ICVImageSource;

class RecognitionThread : public QThread
{
    Q_OBJECT

public:
    explicit RecognitionThread(QObject* p_parent=nullptr);
            ~RecognitionThread(void) override;

    bool setDictonary(cv::Ptr<cv::aruco::Dictionary> dictionary);

    bool setVideoSource(ICVImageSource* p_source);

    void stop(void);

    void update(void);

    void getDetectedParameters(QVector<int>* p_ids, QVector<QPoint>* p_corners);

signals:
    void passedThrough(int id, qint64 tick_count, double tick_frequency);

protected:
    void run(void) override;

private:
    ICVImageSource* mp_source;

    QSemaphore    m_sem;
    QMutex        m_mutex;
    bool          m_loop;


    cv::Ptr<cv::aruco::Dictionary>        m_dictionary;
    std::vector<int>                      m_detected_ids;
    std::vector<int>                      m_detected_id_count;
    size_t                                m_dominant_id;       /* 1 origin. 0 represents null */
    std::vector<std::vector<cv::Point2f>> m_detected_corners;

};

#endif /* RECOGNITIONTHREAD_H */
