#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>

#include "MfErrNo.h"
#include "Core.h"
#include "Sw_Device.h"
#include "Sw_Mf_Classic.h"
#include "Sw_Poll.h"
#include "Sw_ISO14443A-3.h"
#include "TypeDefs.h"
#include "Tools.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("TP MIFARE CLASSIC");
    this->setStyleSheet("background-color: #373737;");
}

MainWindow::~MainWindow()
{
    delete ui;
}

ReaderName MonLecteur;
int count = 0;

void MainWindow::on_btn_connect_clicked()
{

    count++;
    if(count & 1) //Si impair = connexion
    {
        int16_t status = MI_OK;
        MonLecteur.Type = ReaderCDC;
        MonLecteur.device = 0;

        status = OpenCOM(&MonLecteur);
        qDebug() << "OpenCOM" << status;
        RF_Power_Control(&MonLecteur, TRUE, 0);

        status = Version(&MonLecteur);
        ui->label_version->setText(MonLecteur.version);
        ui->label_version->update();

        ui->btn_connect->setStyleSheet("border-image: url(:on.png)");

    }
    else //Si pair = deconnexion
    {
        int16_t status = MI_OK;
        RF_Power_Control(&MonLecteur, FALSE, 0);
        status = LEDBuzzer(&MonLecteur, LED_OFF);
        status = CloseCOM(&MonLecteur);
        ui->btn_connect->setStyleSheet("border-image: url(:off.png)");
    }
}


void MainWindow::on_btn_quitter_clicked()
{
    int16_t status = MI_OK;
    RF_Power_Control(&MonLecteur, FALSE, 0);
    status = LEDBuzzer(&MonLecteur, LED_OFF);
    status = CloseCOM(&MonLecteur);
    qApp->quit();
}

void MainWindow::on_btn_ledON_clicked()
{
    int16_t status = MI_OK;
    status = LEDBuzzer(&MonLecteur, LED_RED_ON);
}

void MainWindow::on_btn_ledOFF_clicked()
{
    int16_t status = MI_OK;
    status = LEDBuzzer(&MonLecteur, LED_RED_OFF);
}

void MainWindow::on_btn_ledONyellow_clicked()
{
    int16_t status = MI_OK;
    status = LEDBuzzer(&MonLecteur, LED_YELLOW_ON);
}

void MainWindow::on_btn_ledONred_clicked()
{
    int16_t status = MI_OK;
    status = LEDBuzzer(&MonLecteur, LED_GREEN_ON);
}


void MainWindow::on_btn_lire_clicked()
{
    uint8_t data[240] = {0};
    int16_t status = 0;
    uint8_t offset;
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;
    int blockNom = 2;
    int blockPrenom = 1;

    status =ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);

    if(status == MI_OK){
        qDebug() << "UID: " << uid;

        status = Mf_Classic_Read_Sector(&MonLecteur, TRUE, 2, data, AuthKeyA, 2);

        if(status == MI_OK){

            QString nom = "";

            qDebug() << "Status: " << status;

            for (offset = 0; offset < 16; offset++){
                if(data[16 * blockNom + offset] != 0)
                    nom += (char)data[16 * blockNom + offset];
            }
            qDebug() << "Nom: " << nom;
            ui->displayLastname->setText(nom);

            QString prenom = "";

            for (offset = 0; offset < 16; offset++){
                if(data[16 * blockPrenom + offset] != 0)
                    prenom += (char)data[16 * blockPrenom + offset];
            }
            qDebug() << "Prenom: " << prenom;
            ui->displayName->setText(prenom);

            notifSuccess();

        }
        else{
            ui->error->setText("Erreur lecture identité: " + QString::number(status));
            LEDBuzzer(&MonLecteur, LED_GREEN_ON);
        }

    }
    else{
        ui->error->setText("Erreur authentification: " + QString::number(status));
        LEDBuzzer(&MonLecteur, LED_GREEN_ON);
    }

    int blocCounter = 14;
    uint32_t value;

    status = Mf_Classic_Read_Value(&MonLecteur, TRUE, blocCounter, &value, AuthKeyA, 3);

    if(status == MI_OK) {
        ui->compteur->setValue(value);
        notifSuccess();
    }
    else
        {
            ui->error->setText("Erreur lecture compteur: " + QString::number(status));
            LEDBuzzer(&MonLecteur, LED_GREEN_ON);
        }
}


void MainWindow::on_btn_buzzer_released()
{
    int16_t status = MI_OK;
    status = LEDBuzzer(&MonLecteur, LED_GREEN_ON);
}

void MainWindow::on_btn_buzzer_pressed()
{
    int16_t status = MI_OK;
    status = LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_GREEN_ON);
}


void MainWindow::on_btn_update_clicked()
{
    uint8_t data[240] = {0};
    uint8_t data2[240] = {0};
    int16_t status = 0;
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;

    if(nom !="" && prenom != "") {

        status = ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);

        if(status == MI_OK){
            qDebug() << "UID: " << status;

            QString nom = ui->displayLastname->toPlainText();
            QString prenom = ui->displayName->toPlainText();

            QByteArray a = nom.toUtf8() ;
            QByteArray b = prenom.toUtf8() ;


            for(int i=0; i < 16; i++){
                data[i] = a[i];
                data2[i] = b[i];
            }

            status = Mf_Classic_Write_Block(&MonLecteur, TRUE, 10, data, AuthKeyB, 2);

            if(status == MI_OK){
                qDebug() << status ;
                notifSuccess();
            }
            else{
                ui->error->setText("Erreur ecriture: " + QString::number(status));
                qDebug() << status ;
                LEDBuzzer(&MonLecteur, LED_GREEN_ON);
            }

            status = Mf_Classic_Write_Block(&MonLecteur, TRUE, 9, data2, AuthKeyB, 2);

            if(status == MI_OK){

                qDebug() << status ;
                notifSuccess();
            }
            else{
                ui->error->setText("Erreur ecriture: " + QString::number(status));
                LEDBuzzer(&MonLecteur, LED_GREEN_ON);
            }
        }
        else{
            ui->error->setText("Erreur authentification: " + QString::number(status));
            LEDBuzzer(&MonLecteur, LED_GREEN_ON);
        }
    }
    else
        ui->error->setText("Le nom et le prénom ne doivent pas être vide !");

}

void MainWindow::on_compteur_valueChanged(int arg1)
{
    uint32_t value = arg1;

    int16_t status = Mf_Classic_Write_Value(&MonLecteur, TRUE, 14, value, AuthKeyB,3);
    if(status != MI_OK)
        ui->error->setText("Erreur ecriture compteur: " + QString::number(status));
    else
        notifSuccess();

}


void MainWindow::on_btn_ledONall_clicked()
{
    LEDBuzzer(&MonLecteur, LED_ON);
}

void MainWindow::on_increment_btn_clicked()
{
    int step = ui->spinBox_Step->value();

    int16_t status = Mf_Classic_Increment_Value(&MonLecteur, TRUE, 14, step, 13, AuthKeyB, 3);

    if(status != MI_OK){
        ui->error->setText("Erreur incrémentation compteur: " + QString::number(status));
        LEDBuzzer(&MonLecteur, LED_GREEN_ON);
    }
    else{
        ui->error->setText("");
        notifSuccess();
    }

    status = Mf_Classic_Restore_Value(&MonLecteur, TRUE, 13, 14, AuthKeyB, 3);

    if(status != MI_OK){
        ui->error->setText("Erreur restore lors de l'incrémentation du compteur: " + QString::number(status));
        LEDBuzzer(&MonLecteur, LED_GREEN_ON);
    }

    else {
        this->on_btn_lire_clicked();
        ui->error->setText("");
        notifSuccess();
    }
}



void MainWindow::on_decrement_btn_clicked()
{
    int step = ui->spinBox_Step->value();

    int16_t status = Mf_Classic_Decrement_Value(&MonLecteur, TRUE, 14, step, 13, AuthKeyA, 3);

    if(status != MI_OK){
        ui->error->setText("Erreur décrémentation compteur: " + QString::number(status));
        LEDBuzzer(&MonLecteur, LED_GREEN_ON);
    }
    else{
        notifSuccess();
        ui->error->setText("");
    }

    status = Mf_Classic_Restore_Value(&MonLecteur, TRUE, 13, 14, AuthKeyA, 3);

    if(status != MI_OK){
        ui->error->setText("Erreur restore lors de la décrémentation du compteur: " + QString::number(status));
        LEDBuzzer(&MonLecteur, LED_GREEN_ON);
    }

    else {
        this->on_btn_lire_clicked();
        ui->error->setText("");
        notifSuccess();
    }
}



void MainWindow::notifSuccess() {
    LEDBuzzer(&MonLecteur, LED_RED_ON);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_GREEN_ON);
    QThread::msleep(100);
    LEDBuzzer(&MonLecteur, LED_GREEN_OFF);
}
