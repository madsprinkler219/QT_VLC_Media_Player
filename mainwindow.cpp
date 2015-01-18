#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QKeyEvent>
#include <vlc/vlc.h>
#include <QMessageBox>
#include <QSettings>
#include "settings.h"
#include "mediacontrolsite.h"
#include "serverthread.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    t = new ServerThread(this);
    t->run();

    play = new QProcess();

    QSettings settings("KevinPC");

    this->currentFocus = 1;
    this->ui->listWidget->installEventFilter(this);
    this->ui->listWidget_2->installEventFilter(this);
    this->ui->listWidget_3->installEventFilter(this);
    this->ui->listWidget_4->installEventFilter(this);
    this->ui->listWidget_5->installEventFilter(this);

    QList<QVariant> paths = settings.value("mediaPaths").toList();
    for (int i=0;i<paths.size();i++)
    {
        addMediaFolder(paths[i].toString());
    }

    addMedia();
}

void MainWindow::userDataWritten()
{
    QString letter = this->t->site->socket->readAll();

    qDebug() << letter;

    if (letter == "\r\n")
    {
        runSiteCommand(this->siteCommands);
        this->siteCommands.clear();
    }
    else
    {
        this->siteCommands = this->siteCommands + letter;
    }
}

void MainWindow::runSiteCommand(QString command)
{
    if (command == "up")
    {
        QObject *obj = new QObject();
        QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress,Qt::Key_Up,Qt::NoModifier);
        eventFilter(obj,(QEvent*) keyEvent);
    }
    else if (command == "down")
    {
        QObject *obj = new QObject();
        QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress,Qt::Key_Down,Qt::NoModifier);
        eventFilter(obj,(QEvent*) keyEvent);
    }
    else if (command == "right")
    {
        changeFocus(1);
    }
    else if (command == "left")
    {
        changeFocus(-1);
    }
}

void MainWindow::addMediaFolder(QString path)
{
    QVector<QString> mediaExtensions;
    mediaExtensions.append(".m4v");
    mediaExtensions.append(".mkv");
    mediaExtensions.append(".mp4");
    mediaExtensions.append(".avi");

    searchPaths.clear();
    searchPaths.append(path);

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

void MainWindow::playMovie(QString path)
{
    QStringList pathSplit = path.split("/",QString::SkipEmptyParts);
    QString playPath;
    if (path.left(2) == "//")
    {
        playPath = "//";
    }
    else if (path.left(1) == "/")
    {
        playPath = "/";
    }

    QString movieName;
    for (int i=0;i<pathSplit.size()-1;i++)
    {
        playPath = playPath + pathSplit[i] + "/";
    }
    movieName = pathSplit.last();

    play = new QProcess();
    play->setProgram(this->vlcLocation);
    play->setWorkingDirectory(playPath);

    play->setArguments(QString(movieName).split(","));
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

void MainWindow::on_close()
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

void MainWindow::playMedia(QString thing)
{
    QStringList thingSplit = thing.split(",");

    QString show = thingSplit[0];
    QString season = thingSplit[1];
    QString episode = thingSplit[2];

    QString workDir = workDirMap[show + "," + season];

    play = new QProcess();
    play->setProgram(vlcLocation);
    play->setWorkingDirectory(workDir);
    play->setArguments((episode).split(","));
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
                this->ui->listWidget_5->addItem(episodes[j]);
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
            this->ui->listWidget_5->addItem(episodes[j]);
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
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Down)
        {
            moveIndex(1);
            return true;
        }
        else if (keyEvent->key() == Qt::Key_U)
        {
            if (this->currentFocus == 5 && this->ui->listWidget_5->currentRow() > 0)
            {
                QString curIndex = this->ui->listWidget_5->currentItem()->text();
                QString newIndex = this->ui->listWidget_5->item(this->ui->listWidget_5->currentRow()-1)->text();
                this->ui->listWidget_5->currentItem()->setText(newIndex);
                this->ui->listWidget_5->item(this->ui->listWidget_5->currentRow()-1)->setText(curIndex);
                this->ui->listWidget_5->setCurrentRow(this->ui->listWidget_5->currentRow()-1);
            }
        }
        else if (keyEvent->key() == Qt::Key_D)
        {
            if (this->currentFocus == 5 && this->ui->listWidget_5->currentRow() < this->ui->listWidget_5->count()-1)
            {
                QString curIndex = this->ui->listWidget_5->currentItem()->text();
                QString newIndex = this->ui->listWidget_5->item(this->ui->listWidget_5->currentRow()+1)->text();
                this->ui->listWidget_5->currentItem()->setText(newIndex);
                this->ui->listWidget_5->item(this->ui->listWidget_5->currentRow()+1)->setText(curIndex);
                this->ui->listWidget_5->setCurrentRow(this->ui->listWidget_5->currentRow()+1);
            }
        }
        else if (keyEvent->key() == Qt::Key_A)
        {
            on_pushButton_6_clicked();
        }
        else if (keyEvent->key() == Qt::Key_C)
        {
            on_pushButton_8_clicked();
        }
        else if (keyEvent->key() == Qt::Key_P)
        {
            on_pushButton_7_clicked();
        }
        else if (keyEvent->key() == Qt::Key_Right)
        {
            changeFocus(1);
        }
        else if (keyEvent->key() == Qt::Key_Left)
        {
            changeFocus(-1);
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
    if (this->currentFocus == 1)
    {
        this->ui->listWidget->setCurrentRow(this->ui->listWidget->currentRow()+move);
    }
    else if (this->currentFocus == 2)
    {
        this->ui->listWidget_2->setCurrentRow(this->ui->listWidget_2->currentRow()+move);
    }
    else if (this->currentFocus == 3)
    {
        this->ui->listWidget_3->setCurrentRow(this->ui->listWidget_3->currentRow()+move);
    }
    else if (this->currentFocus == 4)
    {
        this->ui->listWidget_4->setCurrentRow(this->ui->listWidget_4->currentRow()+move);
    }
    else if (this->currentFocus == 5)
    {
        this->ui->listWidget_5->setCurrentRow(this->ui->listWidget_5->currentRow()+move);
    }
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
            this->currentFocus = 5;
            this->ui->listWidget_5->setFocus();
        }
        else if (this->currentFocus == 5)
        {
            this->currentFocus = 1;
            this->ui->listWidget->setFocus();
        }
    }
    else if (change == -1)
    {
        if (this->currentFocus == 1)
        {
            this->currentFocus = 5;
            this->ui->listWidget_5->setFocus();
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
        else if (this->currentFocus == 5)
        {
            this->currentFocus = 4;
            this->ui->listWidget_4->setFocus();
        }
    }
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionSettings_triggered()
{
    Settings *set = new Settings();
    set->show();
}

void MainWindow::on_pushButton_6_clicked()
{
    //Add to playlist

    if (this->currentFocus == 1)
    {
        this->ui->listWidget_5->addItem(this->ui->listWidget->currentItem()->text());
        this->playlistPath.append(mediaPath[this->ui->listWidget->currentIndex().row()]);
        this->ui->listWidget->setFocus();
    }
    else if (this->currentFocus == 2)
    {
        QStringList seasons = this->seasonMap[this->ui->listWidget_2->currentItem()->text()];
        for (int i=0;i<seasons.size();i++)
        {
            QStringList episodes = this->episodeMap[this->ui->listWidget_2->currentItem()->text() + "," + seasons[i]];
            for (int j=0;j<episodes.size();j++)
            {
                this->ui->listWidget_5->addItem(episodes[j]);
                this->playlistPath.append(this->ui->listWidget_2->currentItem()->text() + "," + seasons[i] + "," + episodes[j]);
            }
        }
        this->ui->listWidget_2->setFocus();
    }
    else if (this->currentFocus == 3)
    {
        QStringList episodes = this->episodeMap[this->ui->listWidget_2->currentItem()->text() + "," + this->ui->listWidget_3->currentItem()->text()];
        for (int j=0;j<episodes.size();j++)
        {
            QString show = this->ui->listWidget_2->currentItem()->text();
            QString season = this->ui->listWidget_3->currentItem()->text();
            this->ui->listWidget_5->addItem(episodes[j]);
            this->playlistPath.append(show + "," + season + "," + episodes[j]);
        }
    }
    else if (this->currentFocus == 4)
    {
        QString show = this->ui->listWidget_2->currentItem()->text();
        QString season = this->ui->listWidget_3->currentItem()->text();
        QString episode = this->ui->listWidget_5->currentItem()->text();
        this->ui->listWidget_4->addItem(episode);
        this->playlistPath.append(show + "," + season + "," + episode);
    }

    this->ui->listWidget_5->setCurrentRow(0);
}

void MainWindow::on_pushButton_8_clicked()
{
    //Clear Playlist
    this->ui->listWidget_5->clear();
    this->playlistPath.clear();
}

void MainWindow::on_pushButton_7_clicked()
{
    //Play Playlist

    QSettings settings("KevinPC");
    this->vlcLocation = settings.value("vlcLocation").toString();

    this->userEndClick = false;

    while (this->playlistPath.size() > 0 && !this->userEndClick)
    {
        if (this->playlistPath.first().split(",").size() == 1)
        {
            if (!this->userEndClick)
            {
                playMovie(this->playlistPath.first());
                play->waitForFinished(-1);
                this->playlistPath.pop_front();
            }
        }
        else
        {
            if (!this->userEndClick)
            {
                playMedia(this->playlistPath.first());
                play->waitForFinished(-1);
                this->playlistPath.pop_front();
            }
        }
        this->ui->listWidget_5->clear();
        for (int i=0;i<this->playlistPath.size();i++)
        {
            this->ui->listWidget_5->addItem(this->playlistPath[i].split(",").last());
            this->ui->listWidget_5->setCurrentRow(0);
        }
    }
}
