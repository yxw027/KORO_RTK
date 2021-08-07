#include "navimain.h"
#include "ui_navimain.h"



rtksvr_t svr;
stream_t moni;


// receiver options table ---------------------------------------------------
static int strtype[]={                  /* stream types */
    STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE
};
static char strpath[8][MAXSTR]={""};    /* stream paths */
static int strfmt[]={                   /* stream formats */
    STRFMT_RTCM3,STRFMT_RTCM3,STRFMT_SP3,SOLF_LLH,SOLF_NMEA,0,0,0
};
static int svrcycle     =10;            /* server cycle (ms) */
static int timeout      =10000;         /* timeout time (ms) */
static int reconnect    =10000;         /* reconnect interval (ms) */
static int nmeacycle    =5000;          /* nmea request cycle (ms) */
static int fswapmargin  =30;            /* file swap marign (s) */
static int buffsize     =32768;         /* input buffer size (bytes) */
static int navmsgsel    =0;             /* navigation mesaage select */
static int nmeareq      =0;             /* nmea request type (0:off,1:lat/lon,2:single) */
static double nmeapos[] ={0,0};         /* nmea position (lat/lon) (deg) */
static char proxyaddr[MAXSTR]="";       /* proxy address */


#define TIMOPT  "0:gpst,1:utc,2:jst,3:tow"
#define CONOPT  "0:dms,1:deg,2:xyz,3:enu,4:pyl"
#define FLGOPT  "0:off,1:std+2:age/ratio/ns"
#define ISTOPT  "0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,6:ntripcli,7:ftp,8:http"
#define OSTOPT  "0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,5:ntripsvr,9:ntrcaster"
#define FMTOPT  "0:rtcm2,1:rtcm3,2:oem4,3:oem3,4:ubx,5:ss2,6:hemis,7:skytraq,8:javad,9:nvs,10:binex,11:rt17,12:spt,13:rnx,14:sp3,15:clk,16:sbas,17:nmea"
#define NMEOPT  "0:off,1:latlon,2:single"
#define SOLOPT  "0:llh,1:xyz,2:enu,3:nmea"
#define MSGOPT  "0:all,1:rover,2:base,3:corr"

static opt_t rcvopts[]={
    {"inpstr1-type",    3,  (void *)&strtype[0],         ISTOPT },
    {"inpstr2-type",    3,  (void *)&strtype[1],         ISTOPT },
    {"inpstr3-type",    3,  (void *)&strtype[2],         ISTOPT },
    {"inpstr1-path",    2,  (void *)strpath [0],         ""     },
    {"inpstr2-path",    2,  (void *)strpath [1],         ""     },
    {"inpstr3-path",    2,  (void *)strpath [2],         ""     },
    {"inpstr1-format",  3,  (void *)&strfmt [0],         FMTOPT },
    {"inpstr2-format",  3,  (void *)&strfmt [1],         FMTOPT },
    {"inpstr3-format",  3,  (void *)&strfmt [2],         FMTOPT },
    {"inpstr2-nmeareq", 3,  (void *)&nmeareq,            NMEOPT },
    {"inpstr2-nmealat", 1,  (void *)&nmeapos[0],         "deg"  },
    {"inpstr2-nmealon", 1,  (void *)&nmeapos[1],         "deg"  },
    {"outstr1-type",    3,  (void *)&strtype[3],         OSTOPT },
    {"outstr2-type",    3,  (void *)&strtype[4],         OSTOPT },
    {"outstr1-path",    2,  (void *)strpath [3],         ""     },
    {"outstr2-path",    2,  (void *)strpath [4],         ""     },
    {"outstr1-format",  3,  (void *)&strfmt [3],         SOLOPT },
    {"outstr2-format",  3,  (void *)&strfmt [4],         SOLOPT },
    {"logstr1-type",    3,  (void *)&strtype[5],         OSTOPT },
    {"logstr2-type",    3,  (void *)&strtype[6],         OSTOPT },
    {"logstr3-type",    3,  (void *)&strtype[7],         OSTOPT },
    {"logstr1-path",    2,  (void *)strpath [5],         ""     },
    {"logstr2-path",    2,  (void *)strpath [6],         ""     },
    {"logstr3-path",    2,  (void *)strpath [7],         ""     },

    {"misc-svrcycle",   0,  (void *)&svrcycle,           "ms"   },
    {"misc-timeout",    0,  (void *)&timeout,            "ms"   },
    {"misc-reconnect",  0,  (void *)&reconnect,          "ms"   },
    {"misc-nmeacycle",  0,  (void *)&nmeacycle,          "ms"   },
    {"misc-buffsize",   0,  (void *)&buffsize,           "bytes"},
    {"misc-navmsgsel",  3,  (void *)&navmsgsel,          MSGOPT },
    {"misc-proxyaddr",  2,  (void *)proxyaddr,           ""     },
    {"misc-fswapmargin",0,  (void *)&fswapmargin,        "s"    },

    {"",0,NULL,""}
};



NaviMain::NaviMain(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NaviMain)
{
    ui->setupUi(this);

    QLabel *labl=new QLabel(this);
    labl->setText("eeeeeemmmmmmm");

    label=labl;

    connect(&Timer,SIGNAL(timeout()),this,SLOT(outPutSol()));

    qDebug()<<"start";
    svrstart();

    Timer.setInterval(100);
    Timer.setSingleShot(false);
    Timer.start();

}

NaviMain::~NaviMain()
{
    delete ui;
}


void NaviMain::svrstart()
{

    //conf文件读取路径
    const char* optfile="G:/RTKTEST.conf";

    char errmsg[2048]="";
    //初始化 流路径
    qDebug()<<"init";
    char *paths[8]={
        strpath[0],strpath[1],strpath[2],strpath[3],strpath[4],strpath[5],
        strpath[6],strpath[7]
    };

    //定义opt
    prcopt_t prcopt=prcopt_default;
    solopt_t solopt=solopt_default;
    filopt_t filopt={};

    //cmd和cmd_periodic  3个输入流的cmd指令
    char*cmds[3]={0,0,0};
    char*cmds_periodic[3]={0,0,0};

    //接收机相关设置和RTCM相关设置 3个流  char opt[256]  命令？
    char *ropts[3]={"","",""};


     qDebug()<<"loadopt";

    //初始化 opt全局变量
    resetsysopts();

    //将conf文件读入到 sysopts（prcopt_ solopt_ filopt_ 等）
    loadopts(optfile,sysopts);
    //
    loadopts(optfile,rcvopts);



    //将prcopt_ solopt_ filopt_等赋值给prcopt,solopt,filopt
    getsysopts(&prcopt,&solopt,&filopt);


    qDebug()<<"svrinit";
    rtksvrinit(&svr);
    strinit(&moni);



    sprintf(solopt.prog ,"%s ver.%s %s",PROGNAME,VER_RTKLIB,PATCH_LEVEL);
    sprintf(filopt.trace,"%s.trace",PROGNAME);


    if (!prcopt.navsys) {
        prcopt.navsys=SYS_GPS|SYS_GLO;
    }

    qDebug()<<"svrstart";
    rtksvrstart(&svr,svrcycle,buffsize,strtype,paths,strfmt,navmsgsel,
                         cmds,cmds_periodic,ropts,nmeacycle,nmeareq,nmeapos,&prcopt,
                         &solopt,&moni,errmsg);

}




//void NaviMain::svrstop()
//{
//    rtksvrstop( &svr, &cmds);
//}

void NaviMain::outputsol()
{

    sol_t *sol;
    int i,update=0;

    rtksvrlock(&svr);

    for (i=0;i<svr.nsol;i++) {
        sol=svr.solbuf+i;
        qDebug()<<"time:"<<sol->time.time<<"stat:"<<sol->stat<<"xyz:"<<sol->rr[0]<<sol->rr[1]<<sol->rr[2]<<sol->rr[3]<<sol->rr[4]<<sol->rr[5]<<"ns:"<<sol->ns<<"ratio"<<sol->ratio<<sol->thres;
        //update=1;

        //QString str;


        //label->setText(str);
      }







      svr.nsol=0;

      rtksvrunlock(&svr);
}

void NaviMain::outPutSol()
{
    outputsol();
}

static int abortf=0;

int showmsg(char *format,...)
{

        va_list arg;
        QString str;
        static QString buff2;
        char buff[1024],*p;

        va_start(arg,format);
        vsprintf(buff,format,arg);
        va_end(arg);

        qDebug()<<buff;

        return abortf;
}

int debug_svr(rtksvr_t svr)
{




    system("pause");

    return 0;

}



