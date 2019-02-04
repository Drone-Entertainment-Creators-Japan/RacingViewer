#ifndef FILEITEMDELEGATE_H
#define FILEITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QString>
class QSettings;

class FileItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit FileItemDelegate(QSettings* p_settings, const QString& extention, QObject* p_parent = nullptr);

    QWidget* createEditor(QWidget* p_parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget* p_editor, const QModelIndex& index) const override;
    void setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* p_editor, const QStyleOptionViewItem&option, const QModelIndex& index ) const override;

    void paint(QPainter* p_painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
private:
    QSettings* mp_settings;
    QString    m_extention;
};

#endif /* FILEITEMDELEGATE_H */
