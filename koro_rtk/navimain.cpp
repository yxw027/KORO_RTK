#include "navimain.h"
#include "ui_navimain.h"



rtksvr_t svr;
stream_t moni;


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


    int strtype[8]={                  /* stream types */
       STR_SERIAL,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE
   };
    char strpath[8][MAXSTR]={"","","","","","","",""}; /* stream paths */
    int strfmt[5]={                   /* stream formats */
       STRFMT_UBX,STRFMT_RTCM3,STRFMT_SP3,SOLF_LLH,SOLF_NMEA
   };
    int svrcycle     =10;            /* server cycle (ms) */

    int nmeacycle    =5000;          /* nmea request cycle (ms) */
    int buffsize     =32768;         /* input buffer size (bytes) */
    int navmsgsel    =0;             /* navigation mesaage select */

    int nmeareq      =0;             /* nmea request type (0:off,1:lat/lon,2:single) */
    double nmeapos[3] ={0,0,0};       /* nmea position (lat/lon/height) (deg,m) */


    char errmsg[2048]="";
    //初始化
    qDebug()<<"init";
    char *paths[8]={
        strpath[0],strpath[1],strpath[2],strpath[3],strpath[4],strpath[5],
        strpath[6],strpath[7]
    };

    //设置处理间隔  最小1
    svrcycle=10;
    //设置buffsize 最小4096
    buffsize=32768;



    //设置处理间隔  最小1
    svrcycle=10;
    //设置buffsize 最小4096
    buffsize=32768;
    //输入输出流设置 in：0-2  out：3-4   log：5-8
    //设置输入输出类型  strtype[x] =STR_XXXX
    //设置输入输出路径  paths[x]   =
    //设置输入输出格式  strfmt[x]  =STRFMT_XXXX
    strtype[0]=STR_NTRIPCLI;
    char path0[MAXSTR]="chdc01:123456@114.55.137.33/TQ01";
    paths[0]=path0;
    strfmt[0]=STRFMT_RTCM3;

    strtype[1]=STR_NTRIPCLI;
    char path1[MAXSTR]="chdc01:123456@114.55.137.33/TQ02";
    paths[1]=path1;
    strfmt[1]=STRFMT_RTCM3;

    strtype[2]=STR_NTRIPCLI;
    char path2[MAXSTR]="chdc01:123456@114.55.137.33/NAVM";
    paths[2]=path2;
    strfmt[2]=STRFMT_RTCM3;


    strtype[3]=STR_FILE;
    char path3[MAXSTR]="G:/resultB34.txt";paths[3]=path3;
    strfmt[3]=SOLF_LLH;


    //设置导航信息选择 navigation mesaage select 0:all 1:rover 2:base  3:correct
    navmsgsel=0;

    prcopt_t prcopt=prcopt_default;
    solopt_t solopt=solopt_default;
    filopt_t filopt={};


    //cmd和cmd_periodic  3个输入流
    char*cmds[3]={0,0,0};
    char*cmds_periodic[3]={0,0,0};


    //接收机相关设置和RTCM相关设置 3个流  char opt[256]
    char *ropts[3]={"","",""};

    //设置nmeacycle 最小1000
    //设置nmeareq 0：no  1：nmeapos  2：single sol
    //设置nmeapos (lat/lon/height) (deg,m)
    nmeacycle=5000;
    nmeareq=0;
    nmeapos[0]=0;
    nmeapos[1]=0;
    nmeapos[2]=0;


    const char* optfile="G:/RTK.conf";
     qDebug()<<"loadopt";
    resetsysopts();//初始化
    loadopts(optfile,sysopts);//将conf文件读入到 sysopts（prcopt_ solopt_ filopt_ 等）


    //getsysopts(&prcopt,&solopt,1,&filopt);//将prcopt_ solopt_ filopt_等赋值给prcopt,solopt,filopt

    prcopt_t *PRCOPT=&prcopt;
    solopt_t *SOLOPT=&solopt;
    filopt_t *FILEOPT=&filopt;

    getsysopts(PRCOPT,SOLOPT,FILEOPT);


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



