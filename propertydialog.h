#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QImage>
#include "common.h"

namespace Ui {
class PropertyDialog;
}

class PropertyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PropertyDialog(QWidget *parent = nullptr);
    ~PropertyDialog();
    void set(FileFormat* fmt);
    void get(FileFormat* fmt);
    void setFileName(const char* path);
    const char* getFileName();
    QImage* CreateImage(const char* filename, FileFormat* fmt);
private:
    int parserFileName(const char* pathname, FileFormat* fmt);
    Ui::PropertyDialog *ui;

    FileFormat mFmt;
    char mFilePath[256];
signals:
public slots:
    void onClickList(int);
    void onBrowse();
private slots:
    void on_buttonBox_accepted();
    void onListChanged();
};

#endif // PROPERTYDIALOG_H
