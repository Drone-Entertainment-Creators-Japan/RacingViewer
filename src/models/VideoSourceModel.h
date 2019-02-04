#ifndef VIDEOSOURCEMODEL_H
#define VIDEOSOURCEMODEL_H

#include <QStandardItemModel>

class IVideoSource;

class VideoSourceModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit VideoSourceModel(QObject* p_parent = nullptr);
            ~VideoSourceModel(void) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool        setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant       data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool                    setVideoSources(const QList<IVideoSource*> sources);
    const QList<IVideoSource*> videoSources(void) const;

public slots:
    void pilotDescChanged(QString pilot, int descriptor_idx, QVariant value); /* to update pilot name */

private:
    QList<IVideoSource*> m_sources;
};

#endif /* VIDEOSOURCEMODEL_H */
