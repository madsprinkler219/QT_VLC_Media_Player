#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QKeyEvent>
//#include <vlc/libvlc.h>
//#include <vlc/libvlc_media_player.h>
//#include <vlc/libvlc_media.h>
//#include <vlc/libvlc_events.h>
//#include <vlc/libvlc_media_library.h>
//#include <vlc/libvlc_media_discoverer.h>
//#include <vlc/libvlc_media_list.h>
//#include <vlc/libvlc_media_list_player.h>
//#include <vlc/libvlc_structures.h>
//#include <vlc/libvlc_version.h>
#include <vlc/vlc.h>
//#include <vlc/deprecated.h>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    play = new QProcess();

    this->currentFocus = 1;
    this->ui->listWidget->installEventFilter(this);
    this->ui->listWidget_2->installEventFilter(this);
    this->ui->listWidget_3->installEventFilter(this);
    this->ui->listWidget_4->installEventFilter(this);

    QVector<QString> mediaExtensions;
    mediaExtensions.append(".m4v");
    mediaExtensions.append(".mkv");
    mediaExtensions.append(".mp4");
    mediaExtensions.append(".avi");

    searchPaths.append("//MEDIA-PC/Users/MEDIA/Desktop/Home Videos");

    mediaPath.clear();
    mediaType.clear();
    mediaMovie.clear();
    mediaShow.clear();
    mediaSeason.clear();
    mediaEpisode.clear();

    for (int i=0;i<searchPaths.size();i++)
    {
        QDir searchDir = QDir(searchPaths[i]);
        QStringList files = searchDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        QStringList subDirs = searchDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        for (int j=0;j<files.size();j++)
        {
            if (mediaExtensions.contains(files[i].right(4)))
            {
                mediaType.append("Movie");
                mediaPath.append(searchPaths[i] + "/" + files[j]);
                mediaMovie.append(files[j]);
                mediaFolder.append(searchPaths[i]);
            }
        }
        for (int j=0;j<subDirs.size();j++)
        {
            QDir showFolder = QDir(searchPaths[i] + "/" + subDirs[j]);
            QStringList showList = showFolder.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
            for (int k=0;k<showList.size();k++)
            {
                qDebug() << showList[k];
                QDir seasonFolder = QDir(searchPaths[i] + "/" + subDirs[j] + "/" + showList[k]);
                QStringList seasonList = seasonFolder.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
                if (seasonList.size() > 0)
                {
                    seasonMap.insert(QString(showList[k]),seasonList);
                }
                for (int m=0;m<seasonList.size();m++)
                {
                    QDir episodes = QDir(seasonFolder.path() + "/" + seasonList[m]);
                    QStringList episodeList = episodes.entryList(QDir::Files | QDir::NoDotAndDotDot);
                    QStringList tmp;
                    for (int p=0;p<episodeList.size();p++)
                    {
                        if (mediaExtensions.contains(episodeList[p].right(4)))
                        {
                            mediaType.append("TV Show");
                            mediaPath.append(episodes.path() + "/" + episodeList[p]);
                            mediaShow.append(showList[k]);
                            mediaSeason.append(seasonList[m]);
                            mediaEpisode.append(episodes[p]);
                            pathMap.insert(QString(showList[k] + "," + seasonList[m] + "," + episodeList[p]),episodes.path() + "/" + episodeList[p]);
                            tmp << episodeList[p];
                            workDirMap.insert(QString(showList[k] + "," + seasonList[m]),episodes.path());
                        }
                    }
                    if (tmp.size() > 0)
                    {
                        episodeMap.insert(QString(showList[k] + "," + seasonList[m]),tmp);
                    }
                }
            }
        }
    }

    addMedia();
}

void MainWindow::testVLC()
{
   libvlc_instance_t *inst;
   libvlc_media_player_t *mp;
   libvlc_media_t *m;
   char ops1[] = "fullscreen";
   char const* const ops2[] = {"--fullscreen"};

   inst = libvlc_new(1,ops2);
   m = libvlc_media_new_location (inst, "file:///C:/Users/Kevin/Videos/TV_Shows/AmericanHorrorStory/Season4/american.horror.story.401.hdtv-lol.mp4");
   //libvlc_media_add_option(m,ops);
   mp = libvlc_media_player_new_from_media (m);
   //libvlc_media_release (m);
   libvlc_media_player_play (mp);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addMedia()
{
    QVector<QString> tmp;
    for (int i=0;i<mediaMovie.size();i++)
    {
        tmp.append(mediaMovie[i]);
    }
    std::sort(tmp.begin(),tmp.end());

    for (int i=0;i<tmp.size();i++)
    {
        this->ui->listWidget->addItem(new QListWidgetItem(tmp[i]));
        this->ui->listWidget->item(0)->setSelected(true);
    }

    tmp.clear();
    for (int i=0;i<seasonMap.keys().size();i++)
    {
        tmp.append(seasonMap.keys()[i]);
    }
    std::sort(tmp.begin(),tmp.end());

    for (int i=0;i<tmp.size();i++)
    {
        this->ui->listWidget_2->addItem(new QListWidgetItem(tmp[i]));
        this->ui->listWidget_2->item(0)->setSelected(true);
    }
}

void MainWindow::on_pushButton_clicked()
{
    //this->testVLC();

    //Play Movie
    play = new QProcess();
    play->setProgram("C:/Program Files (x86)/VideoLAN/VLC/vlc.exe");
    play->setWorkingDirectory(mediaFolder[this->ui->listWidget->currentRow()]);
    play->setArguments(QString("--fullscreen," + mediaMovie[this->ui->listWidget->currentRow()]).split(","));
    connect(play,SIGNAL(finished(int)),this,SLOT(videoFinished()));
    play->start();
}

void MainWindow::videoFinished()
{
    QMessageBox *box = new QMessageBox();
    box->setText("Continue with next episode?");
    box->addButton(QMessageBox::Yes);
    box->addButton(QMessageBox::No);
    int ret = box->exec();

    if (ret == 16384)
    {
        this->userEndClick = false;
    }
    else if (ret == 65536)
    {
        this->userEndClick = true;
    }
}

void MainWindow::on_listWidget_2_currentRowChanged(int currentRow)
{
    //Shows
    this->ui->listWidget_2->blockSignals(true);
    this->ui->listWidget_3->blockSignals(true);

    QString show = this->ui->listWidget_2->item(currentRow)->text();
    this->ui->listWidget_4->clear();
    this->ui->listWidget_3->clear();

    QStringList selSeasons = seasonMap[show];
    QVector<QString> tmp;
    for (int i=0;i<selSeasons.size();i++)
    {
        tmp.append(selSeasons[i]);
    }
    std::sort(tmp.begin(),tmp.end());

    for (int i=0;i<tmp.size();i++)
    {
        this->ui->listWidget_3->addItem(new QListWidgetItem(tmp[i]));
    }

    this->ui->listWidget_2->blockSignals(false);
    this->ui->listWidget_3->blockSignals(false);

    this->ui->listWidget_3->setCurrentRow(0);
    on_listWidget_3_currentRowChanged(0);
}

void MainWindow::on_listWidget_3_currentRowChanged(int currentRow)
{
    this->ui->listWidget_3->blockSignals(true);
    this->ui->listWidget_4->clear();

    QString show = this->ui->listWidget_2->currentItem()->text();
    QString season = this->ui->listWidget_3->item(currentRow)->text();

    QStringList selEpisodes = episodeMap[show + "," + season];
    QVector<QString> tmp;
    for (int i=0;i<selEpisodes.size();i++)
    {
        tmp.append(selEpisodes[i]);
    }
    std::sort(tmp.begin(),tmp.end());

    for (int i=0;i<tmp.size();i++)
    {
        this->ui->listWidget_4->addItem(new QListWidgetItem(tmp[i]));
    }
    this->ui->listWidget_3->blockSignals(false);

    this->ui->listWidget_4->setCurrentRow(0);
}

void MainWindow::on_pushButton_4_clicked()
{
    //this->testVLC();

    //Play show
    QString show = this->ui->listWidget_2->currentItem()->text();
    QString season = this->ui->listWidget_3->currentItem()->text();
    QString episode = this->ui->listWidget_4->currentItem()->text();

    QString workDir = workDirMap[show + "," + season];

    play = new QProcess();
    play->setProgram("C:/Program Files (x86)/VideoLAN/VLC/vlc.exe");
    play->setWorkingDirectory(workDir);
    play->setArguments(QString("--fullscreen,--no-sub-autodetect-file," + episode).split(","));
    connect(play,SIGNAL(finished(int)),this,SLOT(videoFinished()));
    play->start();
}

void MainWindow::on_close()
{
    qDebug() << "TEST1";
}

void MainWindow::playMedia(QString thing)
{
    QStringList thingSplit = thing.split(",");

    QString show = thingSplit[0];
    QString season = thingSplit[1];
    QString episode = thingSplit[2];

    QString workDir = workDirMap[show + "," + season];

    play = new QProcess();
    play->setProgram("C:/Program Files (x86)/VideoLAN/VLC/vlc.exe");
    play->setWorkingDirectory(workDir);
    play->setArguments(QString("--fullscreen," + episode).split(","));
    connect(play,SIGNAL(finished(int)),this,SLOT(videoFinished()));
    play->start();
}

void MainWindow::playShow(QString show)
{
    userEndClick = false;
    QStringList seasons = this->seasonMap[show];
    for (int i=0;i<seasons.size();i++)
    {
        QStringList episodes = this->episodeMap[show + "," + seasons[i]];
        for (int j=0;j<episodes.size();j++)
        {
            if (!userEndClick)
            {
                playMedia(show + "," + seasons[i] + "," + episodes[j]);
                play->waitForFinished(-1);
            }
        }
    }
}

void MainWindow::playSeason(QString show,QString season)
{
    userEndClick = false;
    QStringList episodes = this->episodeMap[show + "," + season];
    for (int j=0;j<episodes.size();j++)
    {
        if (!userEndClick)
        {
            playMedia(show + "," + season + "," + episodes[j]);
            play->waitForFinished(-1);
        }
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Up)
        {
            moveIndex(-1);
        }
        else if (keyEvent->key() == Qt::Key_Down)
        {
            moveIndex(1);
        }
        else if (keyEvent->key() == Qt::Key_Right)
        {
            changeFocus(1);
        }
        else if (keyEvent->key() == Qt::Key_Left)
        {
            changeFocus(-1);
        }
        else if (keyEvent->key() == Qt::Key_M)
        {
            on_pushButton_clicked();
        }
        else if (keyEvent->key() == Qt::Key_P)
        {
            on_pushButton_4_clicked();
        }
        else if (keyEvent->key() == Qt::Key_S)
        {
            playShow(this->ui->listWidget_2->currentItem()->text());
        }
        else if (keyEvent->key() == Qt::Key_A)
        {
            playSeason(this->ui->listWidget_2->currentItem()->text(),this->ui->listWidget_3->currentItem()->text());
        }
        else if (keyEvent->key() == Qt::Key_E)
        {
            if (play->isReadable() && play->isOpen())
            {
                play->close();
            }
        }
        else
        {
            return true;
        }
    }
    return false;
}

void MainWindow::moveIndex(int move)
{

}

void MainWindow::changeFocus(int change)
{
    if (change == 1)
    {
        if (this->currentFocus == 1)
        {
            this->currentFocus = 2;
            this->ui->listWidget_2->setFocus();
        }
        else if (this->currentFocus == 2)
        {
            this->currentFocus = 3;
            this->ui->listWidget_3->setFocus();
        }
        else if (this->currentFocus == 3)
        {
            this->currentFocus = 4;
            this->ui->listWidget_4->setFocus();
        }
        else if (this->currentFocus == 4)
        {
            this->currentFocus = 1;
            this->ui->listWidget->setFocus();
        }
    }
    else if (change == -1)
    {
        if (this->currentFocus == 1)
        {
            this->currentFocus = 4;
            this->ui->listWidget_4->setFocus();
        }
        else if (this->currentFocus == 2)
        {
            this->currentFocus = 1;
            this->ui->listWidget->setFocus();
        }
        else if (this->currentFocus == 3)
        {
            this->currentFocus = 2;
            this->ui->listWidget_2->setFocus();
        }
        else if (this->currentFocus == 4)
        {
            this->currentFocus = 3;
            this->ui->listWidget_3->setFocus();
        }
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    userEndClick = true;
    play->close();
}
