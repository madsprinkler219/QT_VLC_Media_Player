#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QHash>
#include <QListWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QVector<QString> searchPaths,mediaPath,mediaType,mediaMovie,mediaShow,mediaSeason,mediaEpisode,mediaFolder;
    void addMedia();
    QProcess *play;
    QHash<QString,QStringList> seasonMap;
    QHash<QString,QStringList> episodeMap;
    QHash<QString,QString> pathMap;
    QHash<QString,QString> workDirMap;
    void testVLC();
    void changeFocus(int);
    void moveIndex(int);
    bool eventFilter(QObject *obj, QEvent *event);
    int currentFocus;
    void playShow(QString);
    void playSeason(QString,QString);
    void playMedia(QString);

private slots:
    void on_pushButton_clicked();
    void videoFinished();
    void on_listWidget_2_currentRowChanged(int currentRow);
    void on_listWidget_3_currentRowChanged(int currentRow);
    void on_pushButton_4_clicked();
    void on_close();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
