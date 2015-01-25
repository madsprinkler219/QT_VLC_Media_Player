#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QHash>
#include <QListWidget>
#include "mediacontrolsite.h"
#include "serverthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QVector<QString> searchPaths;
    void addMedia();
    QProcess *play;

    QMap<QString,QString> movieList,tvShows,workDirMap,fileNameMap;
    QMap<QString,QVector<QString> > seasonMap;
    QMap<QString,QVector<QString> > episodeMap;

    void testVLC();
    void changeFocus(int);
    void moveIndex(int);
    bool eventFilter(QObject *obj, QEvent *event);
    int currentFocus;
    void playMedia(QString);
    bool userEndClick;
    void addMediaFolder(QString);
    QString vlcLocation;
    bool fullScreen,subs;
    QVector<QString> playlistPath;
    MediaControlSite *site;
    QString siteCommands;
    void runSiteCommand(QString);
    ServerThread *t;

private slots:
    void videoFinished();
    void on_listWidget_2_currentRowChanged(int currentRow);
    void on_listWidget_3_currentRowChanged(int currentRow);
    void on_close();
    void on_actionExit_triggered();
    void on_actionSettings_triggered();
    void on_pushButton_6_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_7_clicked();
    void on_settingsChanged();
    void on_pushButton_clicked();

public slots:
    void userDataWritten();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
