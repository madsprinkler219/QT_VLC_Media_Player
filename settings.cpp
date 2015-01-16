#include "settings.h"
#include "ui_settings.h"
#include <QSettings>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    QSettings settings("KevinPC");
    this->ui->lineEdit->setText(settings.value("vlcLocation").toString());

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
    if (subs)
    {
        this->ui->checkBox_2->setChecked(true);
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
