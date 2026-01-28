#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "WidgetBinCalib.hpp"
#include "FileBin_DWARF.h"
#include <QMainWindow>
#include <qitemselectionmodel.h>
#include <qstandarditemmodel.h>

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
    BinCalibToolWidget *ui_BinCalibWidget;

    void loadElf(std::string file_name);
    void onTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void displayBinaryFile(FileBin_VarInfoType *symbol);
    void populateTopLevelSymbol(FileBin_VarInfoType* node, QStandardItem* parentItem);
    void AddNewBaseFile(QString Filename);
};
#endif // MAINWINDOW_H
