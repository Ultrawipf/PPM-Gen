#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QTime>
#include <QTimer>
#include <QGraphicsScene>
#include <QPoint>
#include <QpointF>
#include "ppmgenerator.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QPointF* inputValue;


private:

    //Channel numbers
    int THR;
    int ELE;
    int AIL;
    int RUD;

    bool arrowMode;

    void updateSettings();
    Ui::MainWindow *ui;
    bool active=false;
    QPoint lastCursorPos = QPoint(0,0);
    QTimer *inputTimer;
    void showStatus();
    QGraphicsScene *scene;
    QPointF lastVal = QPoint(0,0);
    QPointF curVal= QPoint(0,0);
    QPointF diffVal= QPoint(0,0);
    PPMGenerator *ppmGen;

    bool wDown=false;
    bool sDown=false;
    bool aDown=false;
    bool dDown=false;
    bool upDown=false;
    bool downDown=false;
    bool leftDown=false;
    bool rightDown=false;
private slots:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void inputTimeout();
    void on_startButton_clicked();
    void on_dataOffsetspinBox_valueChanged(int arg1);
};

#endif // MAINWINDOW_H
