#ifndef VIDEOASSIGNMODEL_H
#define VIDEOASSIGNMODEL_H

#include <QAbstractTableModel>

#include <QModelIndex>
#include <QWidget>
#include <QString>
#include <QVector>


class IVideoSource;
class VideoSourceModel;

class VideoAssignModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum
    {
        kLayoutType,
        kCutInType,
        kRowHeader
    };
    enum
    {
        kCutIn,
        kColHeader
    };

public:
    explicit VideoAssignModel(VideoSourceModel* p_sources, QObject* p_parent=nullptr);
            ~VideoAssignModel(void) override;

    int    rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool  setData(const QModelIndex& index, const QVariant & value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;

    int  colToPattern(int col) const;

    bool insertLayout(int pattern);
    bool removeLayout(int pattern);

    QString layoutType(int pattern) const;
    QString cutinType(int pattern) const;

    bool setAssign(int pattern, int video_source, int number);
    int     assign(int pattern, int video_source) const;

    bool setCutin(int video_source, bool active);
    bool    cutin(int video_source) const;

    int           videoSourceCount(void) const;
    IVideoSource* videoSource(int video_source);

signals:
    void layoutChanged(int pattern);
    void activateChanged(void);
    void assignChanged(int pattern);
    void cutInChanged(IVideoSource* p_source);

public slots:
    void submodel_rowsRemoved (const QModelIndex& parent, int first, int last);
    void submodel_rowsInserted(const QModelIndex& parent, int first, int last);
    void submodel_rowsMoved   (const QModelIndex& parent, int start, int end, const QModelIndex& destination, int row);
    void submodel_dataChanged (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

private:
    struct LayoutInfo
    {
        QString layout_type;
        QString cutin_type;
    };
    QVector<LayoutInfo> m_layouts;

    VideoSourceModel* mp_sources;
    struct VideoSource
    {
        IVideoSource* p_source;
        bool          cutin;
        QVector<int>  layout_no;
    };
    QList<VideoSource> m_assign;
};
#endif /* VIDEOASSIGNMODEL_H */
