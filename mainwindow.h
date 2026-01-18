#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "FileBin_DWARF.h"
#include <QMainWindow>
#include <qitemselectionmodel.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered(bool checked);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;

private:
    Ui::MainWindow *ui;

    void loadElf(std::string file_name);
    void onTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void displayBinaryFile(FileBin_VarInfoType *symbol);
};
#endif // MAINWINDOW_H
