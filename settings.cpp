#include "settings.h"
#include "ui_settings.h"
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QDir>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    QSettings settings("KevinPC");
    this->ui->lineEdit->setText(settings.value("vlcLocation").toString());
    this->ui->lineEdit_2->setText(settings.value("configFile").toString());

    this->ui->tableWidget->setColumnCount(1);

    QList<QVariant> paths = settings.value("mediaPaths").toList();
    for (int i=0;i<paths.size();i++)
    {
        this->ui->tableWidget->setRowCount(i+1);
        this->ui->tableWidget->setRowHeight(i,18);
        this->ui->tableWidget->setItem(i,0,new QTableWidgetItem(paths[i].toString()));
        this->ui->tableWidget->setColumnWidth(0,350);
    }

    bool fullScreen = settings.value("fullScreen").toBool();
    bool subs  = settings.value("subtitles").toBool();

    if (fullScreen)
    {
        this->ui->checkBox->setChecked(true);
    }
    else
    {
        this->ui->checkBox->setChecked(false);
    }

    if (subs)
    {
        this->ui->checkBox_2->setChecked(true);
    }
    else
    {
        this->ui->checkBox_2->setChecked(false);
    }
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_pushButton_clicked()
{
    QSettings settings("KevinPC");
    settings.setValue("vlcLocation",this->ui->lineEdit->text());
    settings.setValue("configFile",this->ui->lineEdit_2->text());

    QList<QString> paths;
    for (int i=0;i<this->ui->tableWidget->rowCount();i++)
    {
        if (this->ui->tableWidget->item(i,0)->text() != "")
        {
            paths.append(this->ui->tableWidget->item(i,0)->text());
        }
    }

    settings.setValue("mediaPaths",(QVariant) paths);
    settings.setValue("fullScreen",(QVariant) this->ui->checkBox->isChecked());
    settings.setValue("subtitles", (QVariant) this->ui->checkBox_2->isChecked());

    this->close();

    QString saveSettings;
    QFile testFile(this->ui->lineEdit_2->text());
    if (testFile.exists())
    {
        testFile.open(QIODevice::ReadOnly);
        QTextStream file(&testFile);
        while (!file.atEnd())
        {
            QString line = file.readLine();
            if (line.contains("fullscreen="))
            {
                if (settings.value("fullScreen").toBool())
                {
                    saveSettings = saveSettings + "fullscreen=1" + "\n";
                }
                else
                {
                    saveSettings = saveSettings + "fullscreen=0" + "\n";
                }
            }
            else if (line.contains("spu=") && line.size() < 7)
            {
                if (settings.value("subtitles").toBool())
                {
                    saveSettings = saveSettings + "spu=1" + "\n";
                }
                else
                {
                    saveSettings = saveSettings + "spu=0" + "\n";
                }
            }
            else if (line.size() == 0)
            {
                saveSettings = saveSettings + "\n";
            }
            else
            {
                saveSettings = saveSettings + line + "\n";
            }
        }
    }
    testFile.close();

    QFile file(settings.value("configFile").toString());
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << saveSettings;

    file.close();
}

void Settings::on_pushButton_2_clicked()
{
    this->close();
}

void Settings::on_pushButton_3_clicked()
{
    this->ui->tableWidget->setRowCount(this->ui->tableWidget->rowCount()+1);
    this->ui->tableWidget->setItem(this->ui->tableWidget->rowCount()-1,0,new QTableWidgetItem(""));
    this->ui->tableWidget->setRowHeight(this->ui->tableWidget->rowCount()-1,18);
    this->ui->tableWidget->setColumnWidth(0,350);
}
