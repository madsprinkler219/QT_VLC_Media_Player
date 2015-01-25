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
    else if (command == "add")
    {
        on_pushButton_6_clicked();
    }
    else if (command == "clear")
    {
        on_pushButton_8_clicked();
    }
    else if (command == "play")
    {
        on_pushButton_7_clicked();
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
                movieList.insert(files[j],searchPaths[i] + "/" + files[j]);
                workDirMap.insert(searchPaths[i] + "/" + files[j],searchPaths[i]);
                fileNameMap.insert(searchPaths[i] + "/" + files[j],files[j]);
            }
        }
        for (int j=0;j<subDirs.size();j++)
        {
            QDir showFolder = QDir(searchPaths[i] + "/" + subDirs[j]);
            QStringList showList = showFolder.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
            QVector<QString> showVector;
            for (int k=0;k<showList.size();k++)
            {
                showVector.append(showList[k]);
                QDir seasonFolder = QDir(searchPaths[i] + "/" + subDirs[j] + "/" + showList[k]);
                QStringList seasonList = seasonFolder.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
                QVector<QString> seasonVector;
                for (int m=0;m<seasonList.size();m++)
                {
                    seasonVector.append(seasonList[m]);
                    QDir episodes = QDir(seasonFolder.path() + "/" + seasonList[m]);
                    QStringList episodeList = episodes.entryList(QDir::Files | QDir::NoDotAndDotDot);
                    QVector<QString> episodeVector;
                    for (int p=0;p<episodeList.size();p++)
                    {
                        if (mediaExtensions.contains(episodeList[p].right(4)))
                        {
                            episodeVector.append(episodeList[p]);
                            tvShows.insert(showList[k] + "," + seasonList[m] + "," + episodeList[p],episodes.path() + "/" + episodeList[p]);
                            workDirMap.insert(episodes.path() + "/" + episodeList[p],episodes.path());
                            fileNameMap.insert(episodes.path() + "/" + episodeList[p],episodeList[p]);
                        }
                    }
                    if (episodeVector.size() > 0)
                    {
                        this->episodeMap.insert(showList[k] + "," + seasonList[m],episodeVector);
                    }
                }
                if (seasonVector.size() > 0)
                {
                    this->seasonMap.insert(showList[k],seasonVector);
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
    QVector<QString> movies;
    QList<QString> tmp = this->movieList.keys();
    for (int i=0;i<tmp.size();i++)
    {
        movies.append(tmp[i]);
    }
    std::sort(movies.begin(),movies.end());

    for (int i=0;i<movies.size();i++)
    {
        this->ui->listWidget->addItem(new QListWidgetItem(movies[i]));
        this->ui->listWidget->item(0)->setSelected(true);
    }

    QVector<QString> shows;
    QList<QString> tmp1 = this->seasonMap.keys();
    for (int i=0;i<tmp1.size();i++)
    {
        QString item = tmp1[i];
        shows.append(item);
    }
    std::sort(shows.begin(),shows.end());

    for (int i=0;i<shows.size();i++)
    {
        this->ui->listWidget_2->addItem(new QListWidgetItem(shows[i]));
        this->ui->listWidget_2->item(0)->setSelected(true);
    }
}

void MainWindow::videoFinished()
{
    if (this->playlistPath.size() > 1)
    {
        QMessageBox *box = new QMessageBox();
        QString name = this->playlistPath[1];

        box->setText("Continue with next item in playlist:\n\n" + name);

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
}

void MainWindow::on_listWidget_2_currentRowChanged(int currentRow)
{
    //Shows
    this->ui->listWidget_2->blockSignals(true);
    this->ui->listWidget_3->blockSignals(true);

    QString show = this->ui->listWidget_2->item(currentRow)->text();
    this->ui->listWidget_4->clear();
    this->ui->listWidget_3->clear();

    QVector<QString> selSeasons = seasonMap.value(show);
    std::sort(selSeasons.begin(),selSeasons.end());

    for (int i=0;i<selSeasons.size();i++)
    {
        this->ui->listWidget_3->addItem(new QListWidgetItem(selSeasons[i]));
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

    QVector<QString> selEpisodes = episodeMap.value(show + "," + season);
    std::sort(selEpisodes.begin(),selEpisodes.end());

    for (int i=0;i<selEpisodes.size();i++)
    {
        this->ui->listWidget_4->addItem(new QListWidgetItem(selEpisodes[i]));
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

void MainWindow::playMedia(QString path)
{
    QString workDir = workDirMap.value(path);

    play = new QProcess();
    play->setProgram(vlcLocation);
    play->setWorkingDirectory(workDir);
    play->setArguments(fileNameMap.value(path).split(","));
    connect(play,SIGNAL(finished(int)),this,SLOT(videoFinished()));

    play->start();
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

                int selRow = this->ui->listWidget_5->currentRow();
                int moveRow = this->ui->listWidget_5->currentRow()-1;

                QString selItem = this->playlistPath[selRow];
                QString moveItem = this->playlistPath[moveRow];

                this->playlistPath[selRow] = moveItem;
                this->playlistPath[moveRow] = selItem;

                this->ui->listWidget_5->setCurrentRow(this->ui->listWidget_5->currentRow()-1);
            }
            return true;
        }
        else if (keyEvent->key() == Qt::Key_D)
        {
            if (this->currentFocus == 5 && this->ui->listWidget_5->currentRow() < this->ui->listWidget_5->count()-1)
            {
                QString curIndex = this->ui->listWidget_5->currentItem()->text();
                QString newIndex = this->ui->listWidget_5->item(this->ui->listWidget_5->currentRow()+1)->text();
                this->ui->listWidget_5->currentItem()->setText(newIndex);
                this->ui->listWidget_5->item(this->ui->listWidget_5->currentRow()+1)->setText(curIndex);

                int selRow = this->ui->listWidget_5->currentRow();
                int moveRow = this->ui->listWidget_5->currentRow()+1;

                QString selItem = this->playlistPath[selRow];
                QString moveItem = this->playlistPath[moveRow];

                this->playlistPath[selRow] = moveItem;
                this->playlistPath[moveRow] = selItem;

                this->ui->listWidget_5->setCurrentRow(this->ui->listWidget_5->currentRow()+1);
            }
            return true;
        }
        else if (keyEvent->key() == Qt::Key_A)
        {
            on_pushButton_6_clicked();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_C)
        {
            on_pushButton_8_clicked();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_P)
        {
            on_pushButton_7_clicked();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_R)
        {
            on_pushButton_clicked();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Right)
        {
            changeFocus(1);
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Left)
        {
            changeFocus(-1);
            return true;
        }
        else if (keyEvent->key() == Qt::Key_E)
        {
            if (play->isReadable() && play->isOpen())
            {
                play->close();
            }
            return true;
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
    Settings *set = new Settings(this);
    connect(set,SIGNAL(settingsChanged()),this,SLOT(on_settingsChanged()));
    set->show();
}

void MainWindow::on_settingsChanged()
{
    this->ui->listWidget->clear();
    this->ui->listWidget_2->clear();
    this->ui->listWidget_3->clear();
    this->ui->listWidget_4->clear();
    this->ui->listWidget_5->clear();

    seasonMap.clear();
    episodeMap.clear();
    workDirMap.clear();
    searchPaths.clear();

    QSettings settings("KevinPC");

    QList<QVariant> paths = settings.value("mediaPaths").toList();
    for (int i=0;i<paths.size();i++)
    {
        addMediaFolder(paths[i].toString());
    }

    addMedia();
}

void MainWindow::on_pushButton_6_clicked()
{
    //Add to playlist

    if (this->currentFocus == 1)
    {
        this->ui->listWidget_5->addItem(this->ui->listWidget->currentItem()->text());
        this->playlistPath.append(movieList.value(this->ui->listWidget->currentItem()->text()));
        this->ui->listWidget->setFocus();
    }
    else if (this->currentFocus == 2)
    {
        QString show = this->ui->listWidget_2->currentItem()->text();
        QVector<QString> seasons = this->seasonMap[show];
        for (int i=0;i<seasons.size();i++)
        {
            QVector<QString> episodes = this->episodeMap[show + "," + seasons[i]];
            for (int j=0;j<episodes.size();j++)
            {
                this->ui->listWidget_5->addItem(episodes[j]);
                this->playlistPath.append(this->tvShows.value(show + "," + seasons[i] + "," + episodes[j]));
            }
        }
        this->ui->listWidget_2->setFocus();
    }
    else if (this->currentFocus == 3)
    {
        QString show = this->ui->listWidget_2->currentItem()->text();
        QString season = this->ui->listWidget_3->currentItem()->text();
        QVector<QString> episodes = this->episodeMap[show + "," + season];
        for (int j=0;j<episodes.size();j++)
        {
            this->ui->listWidget_5->addItem(episodes[j]);
            this->playlistPath.append(this->tvShows.value(show + "," + season + "," + episodes[j]));
        }
    }
    else if (this->currentFocus == 4)
    {
        QString show = this->ui->listWidget_2->currentItem()->text();
        QString season = this->ui->listWidget_3->currentItem()->text();
        QString episode = this->ui->listWidget_4->currentItem()->text();
        this->ui->listWidget_5->addItem(episode);
        this->playlistPath.append(this->tvShows.value(show + "," + season + "," + episode));
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
        if (!this->userEndClick)
        {
            playMedia(this->playlistPath.first());
            play->waitForFinished(-1);
            this->playlistPath.pop_front();
        }
        this->ui->listWidget_5->clear();
        for (int i=0;i<this->playlistPath.size();i++)
        {
            this->ui->listWidget_5->addItem(this->fileNameMap.value(playlistPath[i]));
            this->ui->listWidget_5->setCurrentRow(0);
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    if (this->ui->listWidget_5->count() > 0)
    {
        int curItem = this->ui->listWidget_5->currentRow();

        if (this->currentFocus == 5)
        {
            this->playlistPath.remove(this->ui->listWidget_5->currentRow());

            this->ui->listWidget_5->clear();
            for (int i=0;i<this->playlistPath.size();i++)
            {
                if (this->playlistPath[i].split(",").size() > 1)
                {
                    this->ui->listWidget_5->addItem(this->fileNameMap.value(this->playlistPath[i]));
                }
                else
                {
                    this->ui->listWidget_5->addItem(this->fileNameMap.value(this->playlistPath[i]));
                }
                if (this->ui->listWidget_5->count() > curItem)
                {
                    this->ui->listWidget_5->setCurrentRow(curItem);
                }
                else if (this->ui->listWidget_5->count() > 0)
                {
                    this->ui->listWidget_5->setCurrentRow(curItem-1);
                }
            }
        }
    }
}
