#pragma once

#include <QtGlobal>
#include <QtWidgets/QMainWindow>

#include "fontrender.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);
    void writeSettings();
    void readSettings();

private:
    Ui::MainWindow *ui;
    FontRender *thread;
    QString projectDir;
    QString project;
    QString homeDir, outFile;
private slots:
    void addFont();
    void removeFont();
    void bruteForce();
    void getFolder();
    void exportFont();
    void saveProject();
    void loadProject();
    void bitDepthChanged(int index);
    void changeFontColor();
    void changeBkgColor();
};
