#ifndef GUIOPTIONSDIALOG_H
#define GUIOPTIONSDIALOG_H

#include <QDialog>
#include "GUI_MainWindow.h"
#include "FileInputScreen.h"
#include "FileDialog.h"

namespace Ui {
class GUIoptionsDialog;
}

class GUIoptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GUIoptionsDialog(QWidget *parent = 0);
    ~GUIoptionsDialog();
    bool returnGUIchoice();

private slots:
    void on_pushButton_clicked();

private:
    Ui::GUIoptionsDialog *ui;

};



#endif // GUIOPTIONSDIALOG_H
