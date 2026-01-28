#ifndef DIALOGBINEDITOR_LINEEDIT_H
#define DIALOGBINEDITOR_LINEEDIT_H
#include <QLineEdit>
#include <iostream>
//#include "BinCalibToolWidget.h"

//using namespace std;

class WidgetTreeTextBox : public QLineEdit
{
    Q_OBJECT

public slots:

    //  void on_table_itemChanged(int row, int col);

    // Q_OBJECT
private slots:
    void onTextChanged(const QString &text);

private:

    int m_firstInt;
    int m_secondInt;


    virtual void mouseReleaseEvent(QMouseEvent *e);

signals:
    void editingFinishedWithInts(int firstInt, int secondInt);
    void mouseReleasedWithInts(int firstInt, int secondInt);


public:
    uint32_t Int1, Int2;
    float DefaultVal;
   // BinCalibToolWidget *pDialog;
    bool showTable;
    uint32_t Idx;
    void (*pChangeCbk)(void * parent_class, int row, int col);
    void * ChangeCbk_class;
    QWidget *parent;
    uint32_t yLen;

    void parseTable();

    WidgetTreeTextBox(QWidget *parent, bool showTable, uint32_t Idx, int firstInt, int secondInt, float DefaultVal);// : QLineEdit(parent);


};


#endif // DIALOGBINEDITOR_LINEEDIT_H
