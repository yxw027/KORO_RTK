#ifndef NAVIMAIN_H
#define NAVIMAIN_H
#include <QWidget>
#include <QTimer>
#include <QDebug>
#include "rtklib.h"
#include <QLabel>

#define PRGNAME     "KORO_RTK"            /* program name */

#define TRACEFILE   "koro_rtk_%Y%m%d%h%M.trace" // debug trace file
#define STATFILE    "koro_rtk_%Y%m%d%h%M.stat"  // solution status file


#define MAXSTR      1024                /* max length of a stream */


#define PROGNAME    "navi"              /* program name */
#define MAXFILE     16                  /* max number of input files */




QT_BEGIN_NAMESPACE
namespace Ui { class NaviMain; }
QT_END_NAMESPACE

class NaviMain : public QWidget
{
    Q_OBJECT

public:
    NaviMain(QWidget *parent = nullptr);
    ~NaviMain();


    QLabel *label;

    QTimer Timer;



    void outputsol();
    void svrstart();



public slots:
        void outPutSol();




private:
    Ui::NaviMain *ui;


};
#endif // NAVIMAIN_H
