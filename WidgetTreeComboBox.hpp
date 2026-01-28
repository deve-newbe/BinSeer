#ifndef HOVERCOMBOBOX_HPP
#define HOVERCOMBOBOX_HPP

#include "qapplication.h"
#include "qcoreevent.h"
#include "qstyleditemdelegate.h"
#include <QComboBox>

// Custom delegate to handle hover cursor for dropdown items
class HoverDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (event->type() == QEvent::HoverEnter) {
            QApplication::setOverrideCursor(Qt::PointingHandCursor); // Change cursor to hand pointer
        } else if (event->type() == QEvent::HoverLeave) {
            QApplication::restoreOverrideCursor(); // Restore default cursor
        }
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }
};

// Custom QComboBox to change cursor on hover
class WidgetTreeComboBox : public QComboBox {

    Q_OBJECT
public:

    int Int1, Int2;
    uint32_t DefaultValIdx;

    using QComboBox::QComboBox; // Inherit constructors

    WidgetTreeComboBox(QWidget *parent = nullptr, int firstInt = -1, int SecondInt= -1, uint32_t DefaultValIdx = 0);
private:
    int m_firstInt;
    int m_secondInt;
signals:
    void editingFinishedWithInts(int firstInt, int secondInt);

protected:
    void wheelEvent( QWheelEvent * e) override;
    void paintEvent(QPaintEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

protected:
    void enterEvent(QEnterEvent *event) override;


    void leaveEvent(QEvent *event) override;
};

#endif // HOVERCOMBOBOX_HPP
