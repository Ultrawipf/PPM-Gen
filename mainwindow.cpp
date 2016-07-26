#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qdebug.h>

#define CHANNELS 6 //Number of channels in a ppm frame


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setMouseTracking(true);
    inputTimer=new QTimer(this);
    inputValue= &curVal;
    connect(inputTimer,SIGNAL(timeout()),this,SLOT(inputTimeout()));
    this->ppmGen=new PPMGenerator(CHANNELS); //Channels in ppm frame
    this->thread()->setPriority(QThread::HighestPriority); //we are time critical.
    ppmGen->setOffset(ui->dataOffsetspinBox->value());
    updateSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateSettings(){
    THR=ui->chanThrspinBox->value()-1;
    ELE=ui->chanElespinBox->value()-1;
    AIL=ui->chanAilspinBox->value()-1;
    RUD=ui->chanRudspinBox->value()-1;
    AUX1=ui->chanAUXspinBox_1->value()-1;
    arrowMode=ui->arrowModecheckBox->checkState();


}

void MainWindow::wheelEvent(QWheelEvent *event){
    int amount=(event->delta()/8);
    if(ui->thrHighspinBox->value()+(amount*2) <= 2000 && ui->thrHighspinBox->value()+(amount*2)>=1000){
        ui->thrHighspinBox->setValue(ui->thrHighspinBox->value()+(amount*2));
        ui->thrCenterspinBox->setValue(ui->thrCenterspinBox->value()+amount);
    }

    if(wDown)
        ppmGen->setChannelData(THR,ui->thrHighspinBox->value());
    else if(!sDown)
        ppmGen->setChannelData(THR,ui->thrCenterspinBox->value());
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    if(event->isAutoRepeat())
        return;
    if(event->key()==Qt::Key_L && !active) //activate
    {
        this->on_startButton_clicked();

    }else if(event->key()==Qt::Key_K && active){ //stop
        setCursor( QCursor( Qt::ArrowCursor ) );
        this->releaseMouse();
        this->releaseKeyboard();
        ppmGen->setChannelData(THR,1000);
        inputTimer->stop();
    }else if(event->key()==Qt::Key_W){
        wDown=true;
        ppmGen->setChannelData(THR,ui->thrHighspinBox->value());
    }else if(event->key()==Qt::Key_S){
        sDown=true;
        ppmGen->setChannelData(THR,ui->thrLowspinBox->value());
    }else if(event->key()==Qt::Key_D){
        dDown=true;
        ppmGen->setChannelData(RUD,2000);
    }else if(event->key()==Qt::Key_A){
        aDown=true;
        ppmGen->setChannelData(RUD,1000);
    }else if(arrowMode && event->key()==Qt::Key_Up){
        upDown=true;
        ppmGen->setChannelData(ELE,2000);
    }else if(arrowMode && event->key()==Qt::Key_Down){
        downDown=true;
        ppmGen->setChannelData(ELE,1000);
    }else if(arrowMode && event->key()==Qt::Key_Left){
        leftDown=true;
        ppmGen->setChannelData(AIL,1000);
    }else if(arrowMode && event->key()==Qt::Key_Right){
        rightDown=true;
        ppmGen->setChannelData(AIL,2000);
    }else if(event->key()==Qt::Key_R){
        aux1Active=!aux1Active;
        ppmGen->setChannelData(AUX1,aux1Active?2000:1000);
        ui->auxLabel->setText("AUX1: "+QString(aux1Active?"on":"off"));
    }

}

void MainWindow::keyReleaseEvent(QKeyEvent *event){
    if(event->isAutoRepeat())
        return;
    if(event->key()==Qt::Key_W){
        if(!sDown)
            ppmGen->setChannelData(THR,ui->thrCenterspinBox->value());
        wDown=false;
    }else if(event->key()==Qt::Key_S){
        if(!wDown)
            ppmGen->setChannelData(THR,ui->thrCenterspinBox->value());
        sDown=false;
    }else if(event->key()==Qt::Key_A){
        if(!dDown)
            ppmGen->setChannelData(RUD,1500);
        aDown=false;
    }else if(event->key()==Qt::Key_D){
        if(!aDown)
            ppmGen->setChannelData(RUD,1500);
        dDown=false;
    }else if(event->key()==Qt::Key_K && active){ //stop
        active=false;
        ppmGen->stop();
    }else if(arrowMode && event->key()==Qt::Key_Up){
        upDown=false;
        if(!downDown)
            ppmGen->setChannelData(ELE,1500);
    }else if(arrowMode && event->key()==Qt::Key_Down){
        downDown=false;
        if(!upDown)
            ppmGen->setChannelData(ELE,1500);
    }else if(arrowMode && event->key()==Qt::Key_Left){
        leftDown=false;
        if(!rightDown)
            ppmGen->setChannelData(AIL,1500);
    }else if(arrowMode && event->key()==Qt::Key_Right){
        rightDown=false;
        if(!leftDown)
            ppmGen->setChannelData(AIL,1500);
    }

}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if(!active || arrowMode)
        return;
    bool differential=ui->differentialCheckBox->isChecked();

    QPoint center = (QWidget::mapToGlobal(ui->centralWidget->geometry().center()));
    QPoint curPos = event->pos()-QWidget::mapFromGlobal(center);

    if(!differential && abs(curVal.x()+curPos.x())>500)
        curPos.setX(0);
    if(!differential && abs(curVal.y()+curPos.y())>500)
        curPos.setY(0);

    curVal += curPos;
    QCursor::setPos(center);

}
void MainWindow::showStatus(){
    //ui->infoLabel->setText();
    ui->thrBar->setValue(ppmGen->getChannelData(THR));
    ui->ailBar->setValue(ppmGen->getChannelData(AIL));
    ui->eleBar->setValue(ppmGen->getChannelData(ELE));
    ui->rudBar->setValue(ppmGen->getChannelData(RUD));
}

void MainWindow::inputTimeout(){

    if(!arrowMode){

        bool differential=ui->differentialCheckBox->isChecked();
        inputValue=(differential? &diffVal: &curVal);

        QPointF newDiffVal=((curVal)-(lastVal)) * (ui->speedSpinBox->value()) + diffVal*0.05;

        diffVal=newDiffVal;
        if(diffVal.isNull() && differential)
            curVal=QPointF(0,0);

        ppmGen->setChannelData(ELE,1500-inputValue->y());
        ppmGen->setChannelData(AIL,inputValue->x()+1500);
        lastVal=curVal;
    }
    showStatus();
}

void MainWindow::on_startButton_clicked()
{
    this->grabKeyboard();
    updateSettings();
    aux1Active=false;
    ppmGen->setChannelData(AUX1,1000);
    ppmGen->setChannelData(THR,1000);
    ppmGen->start();
    setCursor( QCursor( Qt::BlankCursor ) );
    active=true;
    curVal = QPointF(0,0);
    this->grabMouse();
    QCursor::setPos(QWidget::mapToGlobal(ui->centralWidget->geometry().center()));
    inputTimer->start(20);
    showStatus();
}

void MainWindow::on_dataOffsetspinBox_valueChanged(int arg1)
{
    ppmGen->setOffset(arg1);
}
