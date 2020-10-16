


/***************************************
(C) JM Montolio Aranda Spain UE. 2020.
***************************************/


#include "adb.h"

/******* RECORD MARKS *******/
#define RECBMRK 0xBBBB
#define RECDMRK 0xDDDD
#define RECEMRK 0xEEEE

#define STRLX 96 /* STRING LEN */
#define HDLMIN 3 /* dep. OS+APLIC */
#define HDLMAX 9
#define HDLUNI 3
#define IOVERIFY TRUE
#define LRECU 128 
/***** main vect len ********************/
#define TABMAX 20    /* max tabs */
#define IXDBX 10000 /* max indexed rows  */
#define GROVX 10     /* max group rows */
/*********************************************/
#define MAXWORDS 10
#define GRAMWX 10 
#define I32LX 8
#define I32VX 99999999L
#define IDSZX 10
#define FILNAML 16

#define USEDCOLS 4
#define PRINCOLS 6
#define MAXCOLS 10

#define TIDLEN 6
#define COLIDLEN 4  
#define EXTLIN 2
#define GRLINX (GLNX+EXTLIN)

static char MABUF[STRLX];

typedef struct
  {
/** IN *******************/
  int NUMTOK;
  int TOKPAR[MAXWORDS];
  I32 TOKV32[MAXWORDS];
  char TOKS[MAXWORDS][IDSZX];
/****** IN ******************/
  int NUMCMD; /* es enum */
  char tid[TIDLEN+4];
  bool withtt,withkey,withcol,withint;
  int tt,ikey,vkey,icol,ncol,iint,vint;
/****** OUT *****************/
  int SQLCODE,QROWS;
  int allcnt,nrmcnt,delcnt;
  } TYPEUWCB;
static TYPEUWCB UWCB;
static TYPEUWCB *UOW=&UWCB;

/****** prototypes ***************/
static int findKWOt(const char *p);
static int findKWOc(const int p);
static void GRAMLENLIN(void);
static void howcols(void);
static void Fhow(void);
static void Fman(void);
static int inpMABUF(void);
static int parserbox(void);
static void Ffiles(void);
static void Fshdisk(void);
static void Fbulk(void);
static void Fmontecarlo(void);
static int exebox(void);

/************* basic API ********/
static void newl(void)
{printf("\n");
}

bool Bbreak(void)
{
char c;
printf("-- press b break -- ");c=getch();newl();
if(c EQ 'b')return TRUE;
return FALSE;
}

void exitenable(void)
{
printf("MSG NNsec q toexit \n");
if(0)sleep(3);
while(kbhit())if(getch() EQ 'q')exit(0);
}

static int mylen(const char *p)
{int r=strnlen(p,STRLX);
return r;
}

void hexdmp(void *p,long sz)
{unsigned char ch,*qq;
qq=p;
for(int i=0;i LT sz;i++)
  {ch=*qq;qq++;printf("%02X ",ch);
  }
}

static void todayMABUF(void)
{
char buf[STRLX];time_t tt,*pp=&tt;
time(pp);strncpy(buf,ctime(pp),20);
buf[20]=0;strcpy(MABUF,buf);
}

static bool allAZ(const char *p);
static bool allAZ(const char *p)
{
const char SAZ[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
if((int)strspn(p,SAZ) EQ (int)mylen(p))return TRUE;
return FALSE;
}

static bool all09(const char *p);
static bool all09(const char *p)
{
const char S09[]="0123456789";

if((int)strspn(p,S09) EQ (int)mylen(p))return TRUE;
return FALSE;
}
/**************** end basic API ****************/

/**** LINE SYNTAXIS ******************/
enum ENUMWCLAS /* must be negative */ 
{WA9=-32,WAZ,W09,WL4,WCT,WCF,WTID,WKEYV,WCOLID};

#define GLNX 35 /*** must match ****/
static I8 GRAMLEN[GLNX];
static bool GRNOSYM[GLNX];
static bool GRTTKEY[GLNX];
static bool GRTTSYM[GLNX];

/***** mustmatch (0..GLNX(  ***/
enum ENUMCMD /* 0,1,2, etc */ 
{QUIT=0,CREATEDB,DROPDB,OPENDB,CLOSEDB,BACKDB
,CREATA,RENATA,COLHP,COLTYP,COLFUN,DROPTA,TRUNTA,REORTA
,INSROW,UPDWHE,DLTWHE,DLTRANGE,DLTCOL
,SELFIR,SELRBA,SELWHE,SELKGE,SELCOL,SELRANGE,SELGRP
,DESTAB,SHFILES,SHTABS,CHECTA
,SHDISK,HHOW,MAN,TESTCODE,BULK};

static const I8 GRAMR[GLNX][GRAMWX]=
{{72,0,0,0,0,0,0,0,0,0} /* QUIT    */
,{1,7,0,0,0,0,0,0,0,0}  /* CREATE DB */
,{2,7,0,0,0,0,0,0,0,0}  /* DROP DB 0 */
,{4,7,0,0,0,0,0,0,0,0}  /* OPEN DB */
,{5,7,0,0,0,0,0,0,0,0}  /* CLOSE DB */

,{10,7,0,0,0,0,0,0,0,0} /* BACKUP DB */
,{1,8,WTID,0,0,0,0,0,0,0}  /* CREATE TAB t */
,{15,WTID,16,WTID,0,0,0,0,0,0} /* ALTAB WTID NAME WTID*/
,{15,WTID,66,WCOLID,16,WAZ,0,0,0,0} /* ALTAB COL NAME */
,{15,WTID,66,WCOLID,65,WCT,0,0,0,0} /* ALTAB COL TYP */
,{15,WTID,66,WCOLID,67,WCF,0,0,0,0} /* ALTAB COL FUN */
,{2,8,WTID,0,0,0,0,0,0,0}  /* DROP TAB L    */
,{18,8,WTID,0,0,0,0,0,0,0} /* TRUNCATE TAB L */
,{13,11,WTID,0,0,0,0,0,0,0} /* REORG TABIN t */
,{20,WTID,30,WKEYV,W09,W09,W09,0,0,0} /* INS T ROW .. */
,{24,WTID,46,WKEYV,25,WCOLID,W09,0,0,0} /* UPD T KEYEQ k SET c v */
,{26,WTID,46,WKEYV,0,0,0,0,0,0} /* Del t KEYEQ k  */

,{26,WTID,48,WKEYV,W09,0,0,0,0,0} /* del t range k k */
,{26,WTID,47,WCOLID,W09,0,0,0,0,0} /* DEL T COLEQ C V */
,{22,WTID,42,0,0,0,0,0,0,0} /* Sel T FIRST */
,{22,WTID,44,W09,0,0,0,0,0,0} /* Sel T RBAEQ k */
,{22,WTID,46,WKEYV,0,0,0,0,0,0} /* Sel T KEYEQ k */
,{22,WTID,45,WKEYV,0,0,0,0,0,0} /* Sel T KEYGE k */
,{22,WTID,47,WCOLID,W09,0,0,0,0,0} /* Sel t COLEQ col V */
,{22,WTID,48,WKEYV,W09,0,0,0,0,0}  /* SEL t RANGE k k*/
,{22,WTID,50,WCOLID,0,0,0,0,0,0}   /* SEL T GROUPBY c */
,{60,64,0,0,0,0,0,0,0,0} /* DES TABS */
,{56,62,0,0,0,0,0,0,0,0} /* show FILES */
,{56,63,0,0,0,0,0,0,0,0} /* show TABINS   */
,{12,63,0,0,0,0,0,0,0,0}  /* CHECK TABINS */
,{56,68,0,0,0,0,0,0,0,0} /* SHOW DISK */
,{69,WAZ,0,0,0,0,0,0,0,0} /* HOW waz  */
,{70,0,0,0,0,0,0,0,0,0}   /* MAN  */
,{71,0,0,0,0,0,0,0,0,0}   /* TESTCODE */

,{73,WTID,0,0,0,0,0,0,0,0}  /* BULK t */
};

typedef struct
  {
  char *t;int c;
  } TYPEKWOtc;


static const TYPEKWOtc KWOtc[]=
{{"NOP",0},{"CREATE",1},{"DROP",2},{"OPEN",4}
,{"CLOSE",5},{"DB",7},{"TAB",8},{"INDEX",9}
,{"BACKUP",10},{"TABIN",11}
,{"CHECK",12},{"REORG",13}
,{"ALTAB",15}
,{"NAME",16},{"TRUNCATE",18}
,{"INS",20},{"SEL",22},{"UPD",24},{"SET",25}
,{"DEL",26},{"ROW",30},{"FIRST",42},{"RBAEQ",44}
,{"KEYGE",45}
,{"KEYEQ",46},{"COLEQ",47},{"RANGE",48}
,{"GROUPBY",50},{"SHOW",56}
,{"DES",60},{"FILES",62},{"TABINS",63},{"TABS",64}
,{"TYP",65},{"COL",66},{"FUN",67}
,{"DISK",68},{"HOW",69},{"MAN",70}
,{"TESTCODE",71},{"QUIT",72},{"BULK",73},{NULL,0}};

#define CTLISX 4
static const char *CTLIST[]=
  {"NUM","C08","C16","C32",NULL};
#define CFLISX 3
static const char *CFLIST[]=
  {"DATA","XKEY","YDAY",NULL};
/******** END GRAMATIC ****************/

/****** SYSDIN ************************/
typedef struct
  {
  bool created,opened;
  bool ronly;
  } TYPEDBDIN;
static TYPEDBDIN DBDIN;

typedef struct
  {
  char fname[FILNAML],iname[FILNAML];
  bool created,ireated;
  char tid[TIDLEN+2];  /* 6+2 */
  long fleng,trows,SEQ;

  I8 ct[MAXCOLS];
  char colhp[MAXCOLS][COLIDLEN+2];
  I8 cf[MAXCOLS];

  } TYPTADIN;
static TYPTADIN TADIN[TABMAX];

typedef struct
  {
/**** HDR  **/
  U16 bmark;
  U8 st;I8 tt;
  I32 indate,SEQ;
/***** DATA *******/
  I32 C[MAXCOLS];
  I32 dmark;
  char L[68]; 
/***************** Trailer 4B *****/
  U16 crc;U16 emark;
  } TYPRECUNI;  /* exact LRECU */
static TYPRECUNI RECUNI[TABMAX];

/********** IO API  **************/

int dbstate(void)
{
int st;
st=2*(int)DBDIN.created+(int)DBDIN.opened;
assert(st NE 1);return st;
}

void hdlcheck(void)
{
int hopened=0;
for(int hi=HDLMIN;hi LE HDLMAX;hi++)
  {
  if(tell(hi) GE hopened)
    printf("ERROR hdlcheck hdl=%d \n",hi);
  }
for(int hi=HDLMIN;hi LE HDLMAX;hi++)
  {
  assert(tell(hi) LT hopened);
  }
}

static bool existfile(char *p)
{
int stio;bool b;
stio=access(p,(F_OK|R_OK|W_OK));
b=(stio EQ 0);return b;
}

/******** open length close API **************/
static int HDLU;

void myclose(void)
{
if(HDLU LT HDLMIN)return;
if(HDLU GT HDLMAX)return;
if(0)_commit(HDLU);
close(HDLU);HDLU=-1;
}

void HDLUopen(char *p,int m)
{
HDLU=open(p,m);
printf("MSG OPEN /%s/ m %X HDLU %d \n",p,m,HDLU);
assert(HDLU EQ HDLUNI);
assert(tell(HDLU) GE 0);
}

void openTAB(int tt,int m)
{
long posl;

HDLUopen(TADIN[tt].fname,m);
posl=filelength(HDLU);assert(posl GE 0);
TADIN[tt].fleng=posl;assert((posl%LRECU) EQ 0);
TADIN[tt].trows=posl/LRECU;
}

void truncateINDEX(int tt)
{
HDLUopen(TADIN[tt].iname,OTRUNCA);
myclose();
}

/***************** API IO tt *****************/
void createfile(char *p,int m)
{
int h;bool a,d;

a=existfile(p);
h=creat(p,m);close(h);
d=existfile(p);
if(!a AND d)
  printf("Created %s MODE %04X \n",p,m);
}
static void createtable(const int tt)
{
createfile(TADIN[tt].fname,SIRRW);
TADIN[tt].created=TRUE;
TADIN[tt].SEQ=1;
createfile(TADIN[tt].iname,SIRRW);
TADIN[tt].ireated=TRUE;
}
void removefile(char *p)
{
bool a,d;

a=existfile(p);remove(p);d=existfile(p);
if(a AND !d)printf("Removed %s \n",p);
}
void removetable(const int tt)
{
removefile(TADIN[tt].fname);
TADIN[tt].created=FALSE;
removefile(TADIN[tt].iname);
TADIN[tt].ireated=FALSE;
}
void Fcontainer(int p)
{
int stio;

if(p EQ 1)
  {
  stio=mkdir(DIRDATA);
  if(stio EQ 0)printf("Created DATA container \n");
  stio=mkdir(DIRINDEX);
  if(stio EQ 0)printf("Created INDEX container \n");
  }
else if(p EQ 2)
  {
  stio=rmdir(DIRDATA);
  if(stio EQ 0)printf("Removed DATA container \n");
  stio=rmdir(DIRINDEX);
  if(stio EQ 0)printf("Removed INDEX container \n");
  }
}

/***** API REC *****************************/
enum RECST {NRMST=0x0A,DLTST=0x0D};

void printHDR(const int tt)
{
TYPRECUNI *p=&RECUNI[tt];
printf("HDR st %04X tt %02d SEQ %06d \n"
,p->st,p->tt,p->SEQ);
}

void setRECmarks(const int tt)
{
RECUNI[tt].bmark=RECBMRK;RECUNI[tt].dmark=RECDMRK;
RECUNI[tt].emark=RECEMRK;
}
bool checkmarks(const int tt)
{
bool b=( (RECUNI[tt].bmark EQ RECBMRK)
AND (RECUNI[tt].dmark EQ RECDMRK)
AND (RECUNI[tt].emark EQ RECEMRK));
if(!b)printHDR(tt);
return b;
}

static void setRECdflt(const int tt)
{
TYPRECUNI *p=&RECUNI[tt];
memset(p,0,LRECU);setRECmarks(tt);
p->st=NRMST;p->tt=tt;
p->SEQ=TADIN[tt].SEQ;TADIN[tt].SEQ++;
}

static void newpage(int tt)
{
char buf[STRLX];

for(int c=0;c LT PRINCOLS;c++)
  printf("%8s ",TADIN[tt].colhp[c]);
newl();
strcpy(buf,"-------- ");
for(int c=0;c LT PRINCOLS;c++)printf("%s",buf);
newl();
}

void printREC(const int tt)
{
TYPRECUNI *p=&RECUNI[tt];
for(int c=0;c LT PRINCOLS;c++)printf("%8d ",p->C[c]);
newl();
}

void setCRC(const int tt)
{
const int nw=LRECU/2;
U16 buf[nw],crc=0;

RECUNI[tt].crc=0;
memmove(&buf,&RECUNI[tt],LRECU);
crc=0;
for(int i=0;i LT nw;i++)crc=crc bitxor buf[i];
RECUNI[tt].crc=crc;
}

U16 evalCRC(const int tt)
{
const int nw=LRECU/2;
U16 buf[nw],crc=0;

memmove(&buf,&RECUNI[tt],LRECU);crc=0;
for(int i=0;i LT nw;i++)crc=crc bitxor buf[i];
return crc;
}

static void readREC(const int tt,const long posl)
{
int stio;TYPRECUNI *p;
long posz=(posl+LRECU-1);
long fleng=filelength(HDLU);

p=&RECUNI[tt];memset(p,0,LRECU);
assert(posl GE 0);assert(posz LT fleng);
assert(tell(HDLU) GE 0);
lseek(HDLU,posl,SEEK_SET);assert(tell(HDLU) EQ posl);
stio=read(HDLU,&RECUNI[tt],LRECU);assert(stio EQ LRECU);
assert(checkmarks(tt));assert(evalCRC(tt) EQ 0);
assert(RECUNI[tt].tt EQ tt);
}

static void writeREC(const int tt,const long posl)
{
int stio;

assert(posl GE 0);assert(RECUNI[tt].tt EQ tt);
setRECmarks(tt);setCRC(tt);
assert(tell(HDLU) GE 0);
lseek(HDLU,posl,SEEK_SET);assert(tell(HDLU) EQ posl);
stio=write(HDLU,&RECUNI[tt],LRECU);
if(0)_commit(HDLU);
if(stio NE LRECU)
  {
  printf("ERROR st %4d io %4d str %s \n"
  ,stio,errno,strerror(errno));
  }
assert(stio EQ LRECU);
}
/***** END API's ***************************/

/************** IXDB **************/
typedef struct
  {
  I16 tt,st;
  I32 key;
  long RBA;
  } TYPIXDB;
static TYPIXDB IXDB[IXDBX];

/****** IXDIN *******************/
typedef struct
  {
  bool inmem;
  int tt;
  int qix;
  bool mofied,sorted;
  } TYPIXDIN;
static TYPIXDIN IXDIN;

/***** SYSDIN API ************/
int getfreetable(void)
{
for(int t=0;t LT TABMAX;t++)
  if(!TADIN[t].created)return t;
return -1;
}

static void createSYSDIN(void)
{
char buf[STRLX];
for(int tt=0;tt LT TABMAX;tt++)
  {
  memset(&TADIN[tt],0,sizeof(TADIN[tt]));

  sprintf(buf,"%s%02d.TSP",FPREFIX,tt);
  assert(mylen(buf) LT FILNAML);
  strcpy(TADIN[tt].fname,buf);
  TADIN[tt].created=existfile(TADIN[tt].fname);

  sprintf(buf,"%s%02d.TSP",IPREFIX,tt);
  assert(mylen(buf) LT FILNAML);
  strcpy(TADIN[tt].iname,buf);
  TADIN[tt].ireated=existfile(TADIN[tt].iname);
  }
}

static void freshSYSDIN(void)
{
char buf[STRLX];
for(int tt=0;tt LT TABMAX;tt++)
  {
  sprintf(buf,"%s%02d.TSP",FPREFIX,tt);
  assert(mylen(buf) LT FILNAML);
  strcpy(TADIN[tt].fname,buf);
  TADIN[tt].created=existfile(TADIN[tt].fname);

  sprintf(buf,"%s%02d.TSP",IPREFIX,tt);
  assert(mylen(buf) LT FILNAML);
  strcpy(TADIN[tt].iname,buf);
  TADIN[tt].ireated=existfile(TADIN[tt].iname);
  }
}

void writeSYSDIN(void)
{
int s1;
HDLUopen(SYSDINfname,OSYSTW);
s1=(int)sizeof(TADIN);
write(HDLU,&TADIN,s1);
myclose();
printf("SYSDIN Stored \n");
}
void readSYSDIN(void)
{
int stio,s1;
s1=(int)sizeof(TADIN);
HDLUopen(SYSDINfname,OSYSRO);
stio=read(HDLU,&TADIN,s1);assert(stio EQ s1);
myclose();
printf("SYSDIN Loaded \n");
}
static void matchSYSDIN(void)
{
for(int tt=0;tt LT TABMAX;tt++)
  {
  assert(TADIN[tt].created EQ existfile(TADIN[tt].fname));
  assert(TADIN[tt].ireated EQ existfile(TADIN[tt].iname));
  assert(TADIN[tt].created EQ TADIN[tt].ireated); 
  }
}

/******** IXDB API **********************/
long trowsINDEX(int tt)
{
long sz,trix;

HDLUopen(TADIN[tt].iname,OSYSRO);
sz=filelength(HDLU);trix=sz/sizeof(IXDB[0]);
myclose();return trix;
}

int sortfcmp1(void *a,void *b)
{
TYPIXDB *c,*d;
c=a;d=b;
if(c->tt LT d->tt)return -1;
if(c->tt GT d->tt)return +1;
if(c->key LT d->key)return -1;
if(c->key GT d->key)return +1;
return 0;
}
int sortfcmp2(void *a,void *b)
{
TYPIXDB *c,*d;
c=a;d=b;
if(c->tt LT d->tt)return -1;
if(c->tt GT d->tt)return +1;
if(c->RBA LT d->RBA)return -1;
if(c->RBA GT d->RBA)return +1;
return 0;
}
void sortIXDB(int p)
{
void *pf;

printf("MSG sortIXDB OPT=%d \n",p);
pf=&sortfcmp1;if(p EQ 2)pf=&sortfcmp2;
qsort(IXDB,(size_t)IXDIN.qix
,(size_t)sizeof(IXDB[0]),pf);
if(p EQ 1)IXDIN.sorted=TRUE;
}

void writeIXDB(void)
{
int s2;
int tt=IXDIN.tt;

assert(IXDIN.inmem);
HDLUopen(TADIN[tt].iname,OSYSTW);
s2=IXDIN.qix*(sizeof(IXDB[0]));
write(HDLU,&IXDB[0],s2);myclose();
IXDIN.mofied=FALSE;
printf("MSG INDEXwrite %02d \n",tt);
}


void readIXDB(int p)
{
int tt,stio,s2;long s3;

tt=p;
assert(!IXDIN.inmem);assert(TADIN[tt].ireated);
HDLUopen(TADIN[tt].iname,OSYSRO);
s3=filelength(HDLU);s2=(int)s3;
stio=read(HDLU,&IXDB[0],s2);assert(stio EQ s2);
s3=s3/sizeof(IXDB[0]);myclose();
IXDIN.qix=s3;IXDIN.tt=tt;
IXDIN.inmem=TRUE;IXDIN.mofied=FALSE;
printf("MSG INDEXread \n");
sortIXDB(1);
}

static int findkey(const int opt12
,const int xtt,const I32 xkey)
__attribute__((hot));
static int findkey(const int opt12
,const int xtt,const I32 xkey)
{
assert(IXDIN.inmem);assert(IXDIN.sorted);
for(int i=0;i LT IXDIN.qix;i++)
  {
  if(IXDB[i].tt NE xtt)continue; 
  if(IXDB[i].st NE NRMST)continue; 
  if((opt12 EQ 1)AND(IXDB[i].key NE xkey))continue;
  else if((opt12 EQ 2)AND(IXDB[i].key LT xkey))continue;
  return i; 
  }
return -1;
}

static int findslot(void)
{
assert(IXDIN.inmem);
for(int i=0;i LT IXDIN.qix;i++)
  {
  if(IXDB[i].st NE DLTST)continue;
  return i; 
  }
return -1;
}

void printix(int ii)
{
printf("IXDB %4d: st %2X tt %2d key %8d RBA %8lX. \n"
,ii,IXDB[ii].st,IXDB[ii].tt,IXDB[ii].key,IXDB[ii].RBA);
}

/********************************/
void Fduplicates(void)
{
int a,b,nerr=0;

assert(IXDIN.inmem);
sortIXDB(2);
for(int ii=1;ii LT IXDIN.qix;ii++)
  {
  a=ii-1;b=ii;
  if(IXDB[a].st NE NRMST)continue;
  if(IXDB[b].st NE NRMST)continue;
  if(IXDB[a].RBA NE IXDB[b].RBA)continue;
  printf("ERROR duplicateRBA \n");
  printix(a);printix(b);nerr++;
  }
sortIXDB(1);
for(int ii=1;ii LT IXDIN.qix;ii++)
  {
  a=ii-1;b=ii;
  if((IXDB[a].st NE NRMST)
    OR (IXDB[b].st NE NRMST))continue;
  if(IXDB[a].key NE IXDB[b].key)continue;
  printf("ERROR duplicatekey \n");
  printix(a);printix(b);nerr++;
  }
IXDIN.sorted=TRUE;
if(nerr EQ 0)printf("MSG Indexnotduplicates \n");
}

/************* INDEXFULL ************/
void matchttii(int tt,int ii)
{
bool bok=(RECUNI[tt].tt EQ IXDB[ii].tt)
  AND (RECUNI[tt].st EQ IXDB[ii].st);
if(!bok)
  {printHDR(tt);printREC(tt);printix(ii);
  }
assert(RECUNI[tt].st EQ IXDB[ii].st);
assert(RECUNI[tt].tt EQ IXDB[ii].tt);
}

static void indexfull(void)
{
const int cmd=UOW->NUMCMD;
I32 kmin,kmax,tmp;
int tt=-1,ikey=-1,qrow=0;
int firt,last;

if(UOW->withtt)tt=UOW->tt;

printf("MSG indexfull %02d \n",tt);

assert(IXDIN.inmem);
if(UOW->withkey)ikey=UOW->ikey;
kmin=kmax=0;

if((cmd EQ SELRANGE) OR (cmd EQ DLTRANGE))
  {
  assert(UOW->withtt);assert(UOW->withkey);
  assert(tell(HDLU) GE 0);
  kmin=UOW->TOKV32[ikey];kmax=UOW->TOKV32[ikey+1];
  if(kmax LT kmin)
    {tmp=kmin;kmin=kmax;kmax=tmp;}
  }

/** Fduplicates(); **/
sortIXDB(1);
firt=0;last=IXDIN.qix;

for(int ii=firt;ii LT last;ii++)
  {
  const int tti=IXDB[ii].tt;
  const long posli=IXDB[ii].RBA;
  switch(cmd)
    {
    case SELRANGE:
      {
      assert(UOW->withtt);assert(UOW->withkey);
      if(IXDB[ii].st NE NRMST)continue;
      if(tti NE tt)continue;
      if(IXDB[ii].key LT kmin)continue;
      if(IXDB[ii].key GT kmax)continue;
      readREC(tt,posli);
      assert(RECUNI[tt].C[0] EQ IXDB[ii].key);
      printREC(tt);
      }
      break;
    case DLTRANGE:
      {
      if(IXDB[ii].st NE NRMST)continue;
      if(tti NE tt)continue;
      if(IXDB[ii].key LT kmin)continue;
      if(IXDB[ii].key GT kmax)continue;
      readREC(tt,posli);
      RECUNI[tt].st=DLTST;writeREC(tt,posli);
      IXDB[ii].st=DLTST;qrow++;
      }
      break;
    default:
      printf("cmd %d \n",cmd); 
      assert(0 EQ 2);
      break;
    }
  }
myclose();
if(qrow GT 0)
  printf("MSG NUMROWS%04d \n",qrow);
}


static void Fselwherba(void)
{
const int tt=UOW->tt;
int numr;

numr=UOW->TOKV32[TOK3];if(numr LT 0)return;
openTAB(tt,OTABRO);
if(numr LT TADIN[tt].trows)
  {
  long posl=numr*LRECU;
  readREC(tt,posl);
  printf("RBA %08lX \n",posl);printHDR(tt);
  newpage(tt);printREC(tt);
  }
myclose();
}

/***** 1 dbpage *************/
#define PAGEsz 4096
#define PAGEnrx (PAGEsz/LRECU)
typedef union
  {
  char buf[PAGEsz];
  TYPRECUNI R[PAGEnrx];
  } TYPEPAGE64K;
static TYPEPAGE64K KPAGE;

/***** cmd GROUPby  ********/
typedef struct
  {I32 cv;int q;
  } TYPESELGRP;
static TYPESELGRP VGRP[GROVX];


/********** TABFULLSCAN ******/
static void tabfullscan(void)
__attribute__((hot));
static void tabfullscan(void)
{
const int LIMIT= 10;
/***** UOW IN **********/
const int cmd=UOW->NUMCMD;
const int tt=UOW->tt;
const int ncol=UOW->ncol;
/*****************************
const int vint=UOW->vint;
const int icol=UOW->icol;
const int iint=UOW->iint;
*****************************/
I32 nowkey;bool stopscan=FALSE;
int rio,stio;
long endblk,begblk,posl;
int qrows=0;
/***** Vars out any cmd *****/
int allcnt,nrmcnt,delcnt;
/******** only GROUP cmd ****/
int qgrp;
/****************************/

/***** init **********************/

printf("MSG tabfullscan %02d \n",tt);
allcnt=delcnt=nrmcnt=0;

if(cmd EQ SELGRP)
  {
  qgrp=0;
  for(int i=0;i LT GROVX;i++)
    {VGRP[i].cv=-1;VGRP[i].q=-1;}
  }

if(cmd EQ DLTCOL)openTAB(tt,OINDEXW);
else openTAB(tt,OFULTAB);

do/**************** while ********/
  {
  begblk=tell(HDLU);assert(begblk GE 0);
  if(eof(HDLU) EQ 1)break;
  memset(&KPAGE,0,PAGEsz);
  stio=read(HDLU,&KPAGE,PAGEsz);
  assert(stio GE LRECU);assert((stio%LRECU) EQ 0);
  endblk=tell(HDLU);rio=stio/LRECU;
  for(int i=0;(i LT rio) AND NOT stopscan;i++)
    {
    posl=begblk+i*LRECU;assert(posl GE 0);

    memmove(&RECUNI[tt],&KPAGE.R[i],LRECU);
    assert(checkmarks(tt));
    assert(RECUNI[tt].tt EQ tt);
    assert(evalCRC(tt) EQ 0);

/** REC PROCESS START **************************/
    allcnt++;
    if(RECUNI[tt].st EQ DLTST)delcnt++;
    if(RECUNI[tt].st NE NRMST)continue;
    nrmcnt++;nowkey=RECUNI[tt].C[0];
    switch(cmd)
      {
      case SELFIR:
        {
        printREC(tt);
        /** SEL FIRST nrm rec **/
        if(nrmcnt GE LIMIT)stopscan=TRUE;
        }
        break;
      case SELGRP:
        {
        int colv,iigrp;
        colv=RECUNI[tt].C[ncol];iigrp=-1;
        for(int i=0;i LT GROVX;i++)
          if(VGRP[i].cv EQ colv){iigrp=i;break;}

        if(iigrp GE 0)VGRP[iigrp].q++;
        else if((iigrp LT 0) AND (qgrp LT GROVX))
          {VGRP[qgrp].cv=colv;VGRP[qgrp].q=1;qgrp++;
          }
        } /****END GROUP cmd *************/
        break;
      case SELCOL:
        {
        if(RECUNI[tt].C[ncol] EQ UOW->vint)printREC(tt);
        }
        break;       
      case DLTCOL:
        {
        int iiii;
        if(RECUNI[tt].C[ncol] NE UOW->vint)break;
        iiii=findkey(1,tt,nowkey);
        assert(iiii GE 0);
        assert(IXDB[iiii].RBA EQ posl);
        RECUNI[tt].st=DLTST;writeREC(tt,posl);
        qrows++;IXDB[iiii].st=DLTST;
        }
        break;       
      case CHECTA:
        {
        int ii=findkey(1,tt,nowkey);
        assert(ii GE 0);
        assert(IXDB[ii].RBA EQ posl);
        qrows++;matchttii(tt,ii);
        assert(nowkey EQ IXDB[ii].key);
        }
        break;       
      default:
        assert(0 EQ 5); 
        break;
      }
    } /* rio RECs in PAGE */
  lseek(HDLU,endblk,SEEK_SET);
  assert(tell(HDLU) EQ endblk);
  }while(NOT stopscan); /* forall pages on table */
myclose();

/******************************/
UOW->QROWS=qrows;
/******* UOW. 6 OUT, any cmd *****/
UOW->allcnt=allcnt;UOW->nrmcnt=nrmcnt;
UOW->delcnt=delcnt;
}

/******* tabfullscan clients *******/

void Fchecktabins(void)
{

for(int tt=0;tt LT TABMAX;tt++)
  {
  if(existfile(TADIN[tt].fname))
    {
    printf("tt %02d \n",tt);
    IXDIN.inmem=FALSE;
    readIXDB(tt);Fduplicates();
    UOW->tt=tt;
    tabfullscan();
    IXDIN.inmem=FALSE;
    newl();
    }
  }
}


static void Fselfirst(void)
{
int tt=UOW->tt;
newpage(tt);tabfullscan();
}

static void Fselgroup(void)
{
int qsub,qrec;

tabfullscan();qsub=qrec=0;
printf("      VALUE    COUNT \n");
for(int i=0;i LT GROVX;i++)
  {
  if(VGRP[i].cv LE 0)continue;
  printf("%8d %8d \n",VGRP[i].cv,VGRP[i].q);
  qsub++;qrec+=VGRP[i].q;
  }
printf("TOTAL %8d %8d \n",qsub,qrec);
}
/**** end tabfullscan clients ***/


/*********** Freorgtab ************/
int reofirtixdb(int tt)
{
for(int i=0;i LT IXDIN.qix;i++)
  if(IXDB[i].tt EQ tt)return i;
return -1;
}
int reolastixdb(int tt)
{
int last=-1;
for(int i=0;i LT IXDIN.qix;i++)
  if(IXDB[i].tt EQ tt)last=i;
return last;
}
void reorgswap(int tt,int ee,int ff)
{
long eel,ffl;
I32 eek,ffk;

assert(IXDB[ee].st EQ DLTST);
assert(IXDB[ff].st EQ NRMST);
assert(IXDB[ee].tt EQ IXDB[ff].tt);
eek=IXDB[ee].key;eel=IXDB[ee].RBA;
ffk=IXDB[ff].key;ffl=IXDB[ff].RBA;
if(0)assert(eek NE ffk);
assert(eel NE ffl);

readREC(tt,ffl);
RECUNI[tt].st=NRMST;
RECUNI[tt].C[0]=ffk;
IXDB[ee].st=NRMST;
IXDB[ee].key=RECUNI[tt].C[0];
IXDB[ee].RBA=eel;
writeREC(tt,eel);

RECUNI[tt].st=DLTST;
RECUNI[tt].C[0]=eek;
IXDB[ff].st=RECUNI[tt].st;
IXDB[ff].key=RECUNI[tt].C[0];
IXDB[ff].RBA=ffl;
writeREC(tt,ffl);
}
static void Freorgtab(void)
{
const int tt=UOW->tt;
int aa,bb,ee,ff,uu,qq;
long flen;int trec;
sortIXDB(2);
IXDIN.mofied=TRUE;
IXDIN.sorted=FALSE;
aa=reofirtixdb(tt);if(aa LT 0)return;
bb=reolastixdb(tt);if(bb LT 0)return;
if(bb EQ aa)return;
assert(bb GT aa);
printf("TABNUM %2d IXDB[%d,%d]slots \n",tt,aa,bb);
openTAB(tt,OINDEXW);
flen=TADIN[tt].fleng;trec=TADIN[tt].trows;
ee=aa;ff=bb;uu=-1;qq=0;
while(ee LT ff)
  {do
    {
    assert(IXDB[ee].tt EQ tt);
    if(IXDB[ee].st EQ DLTST)break;
    ee++;if(ee GE ff)break;
    }while(1);
  if(ee GE ff)break;
  assert(IXDB[ee].st EQ DLTST);
  do
    {
    assert(IXDB[ff].tt EQ tt);
    if(IXDB[ff].st EQ NRMST)break;
    ff--;if(ff LE ee)break;
    }while(1);
  if(ff LE ee)break;
  assert(IXDB[ff].st EQ NRMST);
  reorgswap(tt,ee,ff);
  uu=ff;qq++;
  }

if(uu GE 0)
  {
  int bu1=bb-uu+1;
  printf("MSG RECS %d Moved %d. \n",trec,qq);
  printf("MSG TABEND NOW %d RECSLOTS \n",bu1);
  printf(
    "MSG FLEN %lX UUPOINT %lX BBEND %lX) \n"
    ,flen,IXDB[uu].RBA,IXDB[bb].RBA);

  if(0)printf("MSG IXDBSlots [%d,%d][%d; %d,%d] %d. \n"
    ,ee,ff,aa,uu,bb,(bb-uu+1));

  for(int i=uu;i LE bb;i++)
    {assert(IXDB[i].st EQ DLTST);
    }
  }
myclose();
IXDIN.mofied=TRUE;IXDIN.sorted=FALSE;
}

/****************************************/
static void Finsrow(void)
{
const int tt=UOW->tt,ikey=UOW->ikey;
I32 xkey=UOW->TOKV32[ikey];
long posl;int ii;

ii=findkey(1,tt,xkey);
if(ii GE 0)
  {printf("MSG existingkey \n");return; 
  }
setRECdflt(tt);
for(int c=0;c LT MAXCOLS;c++)
  {
  RECUNI[tt].C[c]=c;
  }
for(int c=0;c LT USEDCOLS;c++)
  {
  RECUNI[tt].C[c]=UOW->TOKV32[ikey+c];
  }

ii=findslot();
if(ii GE 0)
  {
  posl=IXDB[ii].RBA;
  openTAB(tt,OINSLOT);writeREC(tt,posl);
  assert(IXDIN.qix LT IXDBX);
  IXDB[ii].st=NRMST;IXDB[ii].key=xkey;
  matchttii(tt,ii);
/*** verify write verify **/
  if(IOVERIFY)
    {
    readREC(tt,posl);
    assert(RECUNI[tt].C[0] EQ IXDB[ii].key); 
    }
  myclose();
  printf("MSG rowwrited IX %d RBA %08lX \n",ii,posl);
  }
else
  {
  assert(IXDIN.qix LT IXDBX);
  openTAB(tt,OINSEND);posl=TADIN[tt].fleng;
  writeREC(tt,posl);
  ii=IXDIN.qix;
  IXDB[ii].tt=tt;IXDB[ii].st=NRMST;
  IXDB[ii].key=xkey;IXDB[ii].RBA=posl;
  matchttii(tt,ii);
/***********************************/
  IXDIN.qix++;
/****** write verify ***/
  if(IOVERIFY)
    {
    readREC(tt,posl);
    assert(RECUNI[tt].C[0] EQ IXDB[ii].key);
    }
/*****************************************/
  myclose();
  printf("MSG rowappend IX %d RBA %08lX \n",ii,posl);
  }
IXDIN.mofied=TRUE;IXDIN.sorted=FALSE;
}

static void Fselwhekey(int opt12)
{
const int tt=UOW->tt,ikey=UOW->ikey;
const int xkey=UOW->TOKV32[ikey];
int ii;long posl;

ii=findkey(opt12,tt,xkey);
if(ii LT 0)
  {printf("MSG keynotfound \n");return;
  }
posl=IXDB[ii].RBA;
openTAB(tt,OINDEXR);readREC(tt,posl);
matchttii(tt,ii);myclose();
newpage(tt);printREC(tt);
printf("MSG rowselect %08lX \n",posl);
}
static void Fupdwhe(void)
{
int tt=UOW->tt;
I32 xkey=UOW->TOKV32[UOW->ikey];
I32 xval=UOW->TOKV32[UOW->icol+1];
int ii;long posl;

ii=findkey(1,tt,xkey);
if(ii LT 0)
  {printf("MSG Keynotfound \n");return;
  }
posl=IXDB[ii].RBA;
openTAB(tt,OINDEXW);readREC(tt,posl);
matchttii(tt,ii);
assert(RECUNI[tt].C[0] EQ xkey);
RECUNI[tt].C[UOW->ncol]=xval;
writeREC(tt,posl);myclose();
matchttii(tt,ii);
printf("MSG rowupdated RBA %08lX \n",posl);
} 

static void Fdltwhe(void)
{
const int tt=UOW->tt;
const int ikey=UOW->ikey;
int ii;
long posl;
I32 xkey;

xkey=UOW->TOKV32[ikey];
ii=findkey(1,tt,xkey);
if(ii LT 0)
  {printf("MSG keynotfound \n");return;
  }
assert(IXDB[ii].key EQ xkey);
posl=IXDB[ii].RBA;
openTAB(tt,OINDEXW);readREC(tt,posl);
matchttii(tt,ii);
assert(RECUNI[tt].C[0] EQ IXDB[ii].key);
RECUNI[tt].st=DLTST;writeREC(tt,posl);
myclose();
IXDB[ii].st=DLTST;

IXDIN.mofied=TRUE;IXDIN.sorted=FALSE;
printf("MSG rowdeleted RBA %08lX \n",posl);
}
/*******************************/

void Fdescribe(void)
{
for(int t=0;t LT TABMAX;t++)
  {
  if(!TADIN[t].created)continue;

  printf("%6s: \n",TADIN[t].tid);
  for(int c=0;c LT MAXCOLS;c++)printf("%4s ",TADIN[t].colhp[c]);
  newl();
  for(int c=0;c LT MAXCOLS;c++)
    {int k=TADIN[t].ct[c];assert(k LT CTLISX);
    printf("%4s ",CTLIST[k]);
    }
  newl();
  for(int c=0;c LT MAXCOLS;c++)
    {int k=TADIN[t].cf[c];assert(k LT CFLISX);
    printf("%4s ",CFLIST[k]);
    }
  newl();
  newl();
  }
howcols();
}


static void Fshowtabins(void)
{
long trix;

printf(
"TABxxx TABSEQxx TABROWSx INDROWSx \n");
printf(
"------ -------- -------- -------- \n");
for(int tt=0;tt LT TABMAX;tt++)
  {
  if(existfile(TADIN[tt].fname))
    {
    openTAB(tt,OSYSRO);myclose();
    trix=trowsINDEX(tt);

    printf("%6s %8ld %08ld %08ld \n"
    ,TADIN[tt].tid,TADIN[tt].SEQ
    ,TADIN[tt].trows,trix);

    assert(TADIN[tt].trows EQ trix);
    }
  }
}
/**^^*******************************/

/******* API del texto a simbolo ***/
int TIDtoTT(char *p)
{
for(int t=0;t LT TABMAX;t++)
  {
  if(TADIN[t].created AND 
    (strcmp(TADIN[t].tid,p) EQ 0))return t;
  }
return -1;
}

int COLIDtoNCOL(int tt,char *p)
{
int st;
for(int c=0;c LT MAXCOLS;c++)
  {
  st=strncmp(TADIN[tt].colhp[c],p,STRLX);
  if(st EQ 0)return c;
  }
return -1;
}

int WCTtoNUM(char *p)
{
int k=-1;
for(int i=0;(i LT CTLISX) AND (CTLIST[i] NE NULL);i++)
  if(strcmp(CTLIST[i],p) EQ 0)k=i;
return k;
}
int WCFtoNUM(char *p)
{
int k=-1;
for(int i=0;(i LT CFLISX) AND (CFLIST[i] NE NULL);i++)
  if(strcmp(CFLIST[i],p) EQ 0)k=i;
return k;
}
/**************************************/

/**************** PrepBOX *************/
static int Prepnosym(void)
{
const int cmd=UOW->NUMCMD;
int situ;bool g1,g2,g3;

g1=(cmd EQ MAN);if(g1)return +1;
situ=dbstate();
if(cmd EQ CREATEDB)
  {if(situ EQ 0)return +1;else return -105;}
g3=(cmd EQ OPENDB) OR (cmd EQ BACKDB);
if(g3){if(situ EQ 2)return +1; else return -107;}
g2=(cmd EQ QUIT) OR (cmd EQ SHFILES) 
OR (cmd EQ SHDISK) OR (cmd EQ DROPDB); 
if(g2){if(situ NE 3)return +1;else return -109;}
if(cmd EQ CLOSEDB)
  {if(situ EQ 3)return +1;else return -111;}

if(situ EQ 3)return +1;
return -113;
}

static int Preptt(void)
{
int situ;

UOW->withtt=FALSE;UOW->tt=-1;
situ=dbstate();if(situ NE 3)return -217;
for(int i=0;i LT UOW->NUMTOK;i++)
  {
  if(UOW->TOKPAR[i] EQ WTID)
    {
    assert(!UOW->withtt);
    strcpy(UOW->tid,UOW->TOKS[i]);
    UOW->tt=TIDtoTT(UOW->tid);
    UOW->TOKV32[i]=UOW->tt;
    UOW->withtt=TRUE;
    }
  } /** end for **/
assert(UOW->withtt);
if(UOW->NUMCMD EQ CREATA)
  {
  if(UOW->tt LT 0)
    {
    int tt=getfreetable();
    if(tt LT 0)return -215;
    else {UOW->tt=tt;return +1;}
    }
  else return -217;
  }
if(UOW->tt LT 0)return -219; 
return +1;
}

static int Prepttkey(void)
{
int situ;

UOW->withtt=UOW->withkey=FALSE;
UOW->tt=UOW->ikey=UOW->vkey=-1;
for(int i=0;i LT UOW->NUMTOK;i++)
  {
  if(UOW->TOKPAR[i] EQ WTID)
    {
    assert(!UOW->withtt);
    strcpy(UOW->tid,UOW->TOKS[i]);
    UOW->tt=TIDtoTT(UOW->tid);
    UOW->TOKV32[i]=UOW->tt;
    UOW->withtt=TRUE;
    }
  else if(UOW->TOKPAR[i] EQ WKEYV)
    {
    assert(!UOW->withkey);
    UOW->ikey=i;UOW->vkey=UOW->TOKV32[i];
    UOW->withkey=TRUE;
    }
  } /** end for **/

if(UOW->tt LT 0)return -319;
situ=dbstate();if(situ NE 3)return -321;
return +1;
}

static int Prepdflt(void)
{
const int cmd=UOW->NUMCMD;
int situ;

UOW->tt=-1;
UOW->withtt=FALSE;
for(int i=0;i LT UOW->NUMTOK;i++)
  {
  if(UOW->TOKPAR[i] EQ WTID)
    {
    if(!UOW->withtt)
      {
      strcpy(UOW->tid,UOW->TOKS[i]);
      UOW->tt=TIDtoTT(UOW->tid);
      UOW->TOKV32[i]=UOW->tt;
      UOW->withtt=TRUE;
      }
    }
  else if(UOW->TOKPAR[i] EQ WKEYV)
    {
    if(!UOW->withkey)
      {
      UOW->ikey=i;UOW->vkey=UOW->TOKV32[i];
      UOW->withkey=TRUE;
      }
    }
  else if(UOW->TOKPAR[i] EQ WCOLID)
    {
    if(!UOW->withcol)
      {
      UOW->icol=i;
      UOW->ncol=COLIDtoNCOL(UOW->tt,UOW->TOKS[i]);
      UOW->TOKV32[i]=UOW->ncol;
      UOW->withcol=TRUE;
      }
    }
  else if(UOW->TOKPAR[i] EQ W09)
    {
    if(!UOW->withint)
      {
      UOW->iint=i;
      UOW->vint=UOW->TOKV32[i];
      UOW->withint=TRUE;
      }
    }
  else if(UOW->TOKPAR[i] EQ WCT)
    {
    UOW->TOKV32[i]=WCTtoNUM(UOW->TOKS[i]);
    }
  else if(UOW->TOKPAR[i] EQ WCF)
    {
    UOW->TOKV32[i]=WCFtoNUM(UOW->TOKS[i]);
    }

  } /** end for **/

if(cmd EQ HHOW)return +1;
situ=dbstate();
if(cmd EQ RENATA)
  {
  int v,n;
  if(situ NE 3)return -401;
  v=TIDtoTT(UOW->TOKS[TOK1]);
  n=TIDtoTT(UOW->TOKS[TOK3]);
  UOW->TOKV32[TOK1]=v;UOW->TOKV32[TOK3]=n;
  if((v GE 0) AND (n LT 0))return +1;
  else return -403;
  }

if(cmd EQ COLTYP)
  {
  if(situ NE 3)return -405;
  if(UOW->tt LT 0)return -406;
  if(!UOW->withcol)return -407;
  if(UOW->TOKV32[TOK3] LT 0)return -408;
  if(UOW->TOKV32[TOK5] LT 0)return -409;
  }

if(cmd EQ COLHP)
  {
  if(situ NE 3)return -411;
  if(UOW->tt LT 0)return -413;
  if(!UOW->withcol)return -443;
  if(UOW->TOKV32[TOK3] LT 0)return -445;
  if(mylen(UOW->TOKS[TOK5]) NE 4)return -449;
  if(COLIDtoNCOL(UOW->tt,UOW->TOKS[TOK5])
     GE 0)return -447;
  }

if(UOW->withcol AND (UOW->ncol LT 0) )return -415;
if(UOW->withtt)
  {
  if(situ NE 3)return -417;
  if(UOW->tt GE 0)return +1;
  else return -419; 
  }
if(situ NE 3)return -421;
return +1;
}

int prepbox(void)
{
int cmd=UOW->NUMCMD;
int st;

if(GRNOSYM[cmd])st=Prepnosym();
else if(GRTTSYM[cmd])st=Preptt();
else if(GRTTKEY[cmd])st=Prepttkey();
else st=Prepdflt();
return st;
}
/**********************************/
/********* MAIN *******************/

/*** Montecarlo testcode *********/
#define MALBUX 100000
static char MALBU[MALBUX][STRLX];
int qmalbux;

static void userexit(void)
{
if(!DBDIN.opened)return;
writeSYSDIN();
}


static void initmain(void)
{
srand( (unsigned int) clock() );
GRAMLENLIN();
memset(&DBDIN,0,sizeof(DBDIN));
DBDIN.created=existfile(SYSDINfname);
createSYSDIN();
IXDIN.qix=0;IXDIN.mofied=FALSE;
IXDIN.sorted=TRUE;
atexit(userexit);
}

static bool checkPAK(const int pak)
{
int a,b;
printf("Checking PAK.. \n");if(pak LE 1000)return FALSE;
a=pak%100;b=(pak-a)/100;if(b LT 40)return FALSE;
if((a+b) NE 87)return FALSE;
return TRUE;
}

static void newUWCB(void)
{
memset(UOW,0,sizeof(UWCB));
}

#define ERDESX 1000
static char ERDES[ERDESX][STRLX];
void printerdes(int nega)
{
int st=-nega;

if(st LT 0)return;
if(st GE ERDESX)return;
for(int i=0;i LT ERDESX;i++)strset(ERDES[i],0);

strcpy(ERDES[113],"gral. prepare error");
strcpy(ERDES[499],"gral. syntax error");
if(mylen(ERDES[st]) GT 0)
  {
  printf("ERROR %04d %s \n",st,ERDES[st]);
  }
}



int main(const int argc,const char *argv[])
{
FILE *FEVENLOG;
int pak;bool okpak,sactive;

if(0)printf("szREC %d LRECU %d \n"
  ,(int)sizeof(RECUNI[0]),LRECU);
assert(sizeof(RECUNI[0]) EQ LRECU);
/***************************************/

todayMABUF();
pak=0;okpak=FALSE;
if(argc EQ 2)
  {pak=atoi(argv[1]);okpak=checkPAK(pak);
  }
FEVENLOG=fopen(FEVEFNAME,"w");
fprintf(FEVENLOG
,"EXEC %d /%s %s/ %s %d %d \n"
,argc,argv[0],argv[1],MABUF,pak,okpak);
fclose(FEVENLOG);
/******************************************/

if(argc NE 2)
  {
  printf("ERROR Please append your PAK License \n");
  printf("Use: <EXE> <PAK-ID> \n");
  }
if(!okpak)
  {
  printf("ERROR unverifiedpak \n");
  }  
if(!okpak)exit(0);
/*****************************************/

printf("atomDB 0.5  (C)JMMA \n");
printf("%s \n",MABUF);
printf("MSG pakverified  \n");

/*****************************************/

initmain();sactive=TRUE;qmalbux=0;
while(sactive)
  {
  int pha,st;

  pha=0;
  while(pha LT 6)
    {
    switch(pha)
      {
      case 0:
        if(qmalbux GT 0)
        {
        strncpy(MABUF,MALBU[qmalbux-1],STRLX);
        qmalbux--;st=+1;
        printf("now--> %4d /%s/ \n",qmalbux,MABUF);
        }
        else st=inpMABUF();
        if(st EQ +1)pha++;
        break;

      case 1:
        newUWCB();
        st=parserbox();
        if(st LT 0)
        {
          printf("ERROR PARSECODE%04d \n",-st);
          printerdes(st);pha=0;
        }
        else pha++;
        break;

      case 2:
        st=prepbox();
        if(st LT 0)
        {
        printf("ERROR PREPARECODE%04d \n",-st);
        printerdes(st);pha=0;
        }
        else pha++;
        break;

      case 3:
        hdlcheck();
        st=exebox();
        if(st LT 0)
        {
        st=st-800;
        printf("ERROR EXECODE%04d \n",-st);
        printerdes(st);pha=0;
        }
        else 
        {
        if(UOW->NUMCMD EQ QUIT)sactive=FALSE;
        pha++;
        }
        break;
      case 4:
        hdlcheck();pha++;
        break;
      case 5:
        newl();
        /** dar msg aplazados linea texto */
        if(qmalbux GT 0)
        {exitenable();newl();newl();
        }
        pha++;
        break;
      default: break;
      }
    }
  } /*** END MAIN LOOP *****/

return +1;
}
/*******************************/
/** under MAIN code ************/
/*******************************/

static int findKWOt(const char *p)
{
int st;
for(int i=0;KWOtc[i].t NE NULL;i++)
  {st=strcmp(KWOtc[i].t,p);if(st EQ 0)return i;
  }
return -1;
}
static int findKWOc(const int p)
{
for(int i=0;KWOtc[i].t NE NULL;i++)
  if(KWOtc[i].c EQ p)return i;
return -1;
}
/*********************************/

/****** PARSERBOX ****************/
static int parserbox(void)
{
char LEXBUF[STRLX],SQLBUF[STRLX],PARBUF[STRLX];
int SEPI[MAXWORDS],TOKINI[MAXWORDS];
int TOKLEN[MAXWORDS],TOKLEX[MAXWORDS];
const char VALIDSET[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";

int NSEP,sblen,szt,ksep,sum,st,k,xlen;
char SPACE=' ';char cp;
int imatch=-449;

strncpy(SQLBUF,strupr(MABUF),STRLX);
szt=(int)strspn(SQLBUF,VALIDSET);
xlen=mylen(SQLBUF);
if(szt NE xlen)return -701;

k=0;cp=SPACE;strnset(LEXBUF,0,STRLX);
for(int i=0;i LT xlen;i++)
  {
  char cb=SQLBUF[i];
  if((cb EQ SPACE) AND (cp EQ SPACE))continue;
  LEXBUF[k++]=cb;cp=cb;
  }
if((k GE 2) AND (LEXBUF[k-1] EQ SPACE))k--;
LEXBUF[k]=0;

strncpy(PARBUF,LEXBUF,STRLX);
sblen=mylen(PARBUF);

/********** NSEP y sepi ***************/
ksep=0;
for(int i=0;i LT sblen;i++)
  {char ch=PARBUF[i];
  if(ch EQ SPACE)
    {SEPI[ksep++]=i;if(ksep GE MAXWORDS)return -703;
    }
  }
NSEP=ksep;if(NSEP GE MAXWORDS)return -705;

/**** NUMTOK y TOKINI *********************/
UOW->NUMTOK=0;TOKINI[UOW->NUMTOK++]=0;
for(int i=0;i LT NSEP;i++)
  {
  TOKINI[UOW->NUMTOK++]=SEPI[i]+1;
  if(UOW->NUMTOK GE MAXWORDS)return -707;
  }
/*******  toklen ************************/
TOKLEN[UOW->NUMTOK-1]=sblen-TOKINI[UOW->NUMTOK-1];
for(int i=0;i LE (UOW->NUMTOK-2);i++)
  {TOKLEN[i]=TOKINI[i+1]-TOKINI[i]-1;}

for(int i=0;i LT UOW->NUMTOK;i++)
  {if(TOKLEN[i] GE IDSZX)return -709;}

/*********** Check ******/
sum=NSEP;
for(int i=0;i LT UOW->NUMTOK;i++)sum+=TOKLEN[i];
assert(sum EQ sblen);

for(int i=0;i LT UOW->NUMTOK;i++)
  {
  strset(UOW->TOKS[i],0);
  strncpy(UOW->TOKS[i],&PARBUF[TOKINI[i]],TOKLEN[i]);
  UOW->TOKV32[i]=-1;
  }
for(int i=0;i LT UOW->NUMTOK;i++)
  {
  bool stAZ,st09;

  TOKLEX[i]=WA9;
  stAZ=allAZ(UOW->TOKS[i]);
  if(stAZ)TOKLEX[i]=WAZ;
  if(TOKLEX[i] NE WA9)continue;
  st09=all09(UOW->TOKS[i]);
  if(st09)
    {
    long v;
    if(TOKLEN[i] GT I32LX)return -711;
    v=atol(UOW->TOKS[i]);if(v LT 0)return -713;
    if(v GT I32VX)return -715;
    TOKLEX[i]=W09;UOW->TOKV32[i]=v;
    }
  }
if(UOW->NUMTOK GT GRAMWX)return -751;
if(UOW->NUMTOK LE 0)return -753;
for(int ii=0;ii LT GLNX;ii++)
  {
  int ml=GRAMLEN[ii];
  bool fmatch;
  if(ml NE UOW->NUMTOK)continue;
  fmatch=TRUE;
  for(int j=0;j LT ml;j++)
    {
    const int vv=UOW->TOKV32[j];
    const int gij=GRAMR[ii][j];
    const int rr=TOKLEX[j];
    const int ll=TOKLEN[j];
  
    if(gij GE 0) /** gij reserved word **/
      {
      if(rr EQ WAZ)
        {
        st=findKWOt(UOW->TOKS[j]);
        if(st LT 0)fmatch=FALSE;
        else if(KWOtc[st].c NE gij)fmatch=FALSE;
        }
      else fmatch=FALSE;
      }
    else if(gij EQ W09)
      {if(rr NE W09)fmatch=FALSE;
      if(vv LT 0)fmatch=FALSE;
      }
    else if(gij EQ WKEYV)
      {
      if(rr NE W09)fmatch=FALSE;
      if(vv LE 0)fmatch=FALSE;
      }
    else if(gij EQ WTID)
      {
      if(rr NE WAZ)fmatch=FALSE;
      if(ll NE TIDLEN)fmatch=FALSE;
      }
    else if(gij EQ WCOLID)
      {
      if(ll NE COLIDLEN)fmatch=FALSE;
      }
    else if(gij EQ WAZ)
      {
      if(rr NE WAZ)fmatch=FALSE;
      }
    else if(gij EQ W09)
      {
      if(rr NE W09)fmatch=FALSE;
      }
    else if(gij EQ WCT)
      {
      if(ll NE 3)fmatch=FALSE;
      }
    else if(gij EQ WCF)
      {
      if(ll NE 4)fmatch=FALSE;
      }
    else assert(0 EQ 9);
    }
  if(fmatch)
    {
    if(imatch GE 0)return -759;
    imatch=ii;
    }
  }
UOW->NUMCMD=imatch;
if(imatch GE 0)
  {
  for(int j=0;j LT GRAMLEN[imatch];j++)
    {UOW->TOKPAR[j]=GRAMR[imatch][j];}
  }
return imatch;
}

static char GRAMLIN[GRLINX][STRLX];

static void GRAMLENLIN(void)
{ 
int qlt,qwtid,qwkey,k;
char buf[STRLX],puf[STRLX];

for(int i=0;i LT GLNX;i++)
  {
  GRAMLEN[i]=0;qlt=qwtid=qwkey=0;
  for(int j=0;j LT GRAMWX;j++)
    {
    int gij=GRAMR[i][j];
    if(gij EQ 0)break;
    GRAMLEN[i]++;
    if(gij LT 0)
      {
      qlt++;
      if(gij EQ WTID)qwtid++;
      else if(gij EQ WKEYV)qwkey++;
      }
    }
  GRNOSYM[i]=(qlt EQ 0);
  GRTTSYM[i]=(qlt EQ 1)AND(qwtid EQ 1);
  GRTTKEY[i]=(qlt EQ 2)AND(qwtid EQ 1)
    AND(qwkey EQ 1);
  }

for(int i=0;i LT GLNX;i++)
  {
  strset(buf,0);
  for(int j=0;j LT GRAMLEN[i];j++)
    {
    int v=GRAMR[i][j];int st;
    strset(puf,0);
    if(v GE 0)
      {
      st=findKWOc(v);assert(st GE 0);
      sprintf(puf,"%s ",KWOtc[st].t);
      }
    else if(v EQ WTID)  sprintf(puf,"tid6 ");
    else if(v EQ W09)   sprintf(puf,"num ");
    else if(v EQ WKEYV)  sprintf(puf,"key ");
    else if(v EQ WCOLID) sprintf(puf,"col4 ");
    else if(v EQ WAZ)    sprintf(puf,"waz ");
    else if(v EQ WA9)    sprintf(puf,"wA9 ");
    else if(v EQ WCT)    sprintf(puf,"wct ");
    else if(v EQ WCF)    sprintf(puf,"wcf ");
    strcat(buf,puf);
    }
  strcpy(GRAMLIN[i],buf);
  }

k=GLNX+0;strset(GRAMLIN[k],0);
strcpy(GRAMLIN[k],"COL TYP: ");
for(int i=0;i LT CTLISX;i++)
  {strcat(GRAMLIN[k],CTLIST[i]);
  strcat(GRAMLIN[k]," ");
  }
k=GLNX+1;strset(GRAMLIN[k],0);
strcpy(GRAMLIN[k],"COL FUN: ");
for(int i=0;i LT CFLISX;i++)
  {strcat(GRAMLIN[k],CFLIST[i]);
  strcat(GRAMLIN[k]," ");
  }
}

static int inpMABUF(void)
{
char buf[STRLX];
char SPACE=' ';
int sl;
printf("adb> ");strset(buf,0);fgets(buf,STRLX,stdin);
sl=mylen(buf);if(sl LE 0)return -1;
if(buf[sl-1] LT SPACE)buf[sl-1]=0;
buf[sl]=0;
sl=mylen(buf);if(sl LE 0)return -1;
strncpy(MABUF,buf,STRLX);return +1;
}

/********** FILES AND DISK *****/
long mystat(char *p)
{
struct stat SST,*q;int stio;long fsz;

fsz=0;q=&SST;memset(q,0,sizeof(SST));
stio=stat(p,q);if(stio NE 0)return fsz;
fsz=(long)SST.st_size;
return fsz;
}

static void Ffiles(void)
{
long tsz,isz;

printf("DB FILES \n");
printf("Filenamexxxxxxxx SIZExxxx INDSIZEx  \n");
printf("---------------- -------- --------  \n");

tsz=mystat(SYSDINfname);
printf("%-16s %08lX \n",SYSDINfname,tsz);

for(int tt=0;tt LT TABMAX;tt++)
  {
  tsz=mystat(TADIN[tt].fname);
  isz=mystat(TADIN[tt].iname);
  printf("%-16s %08lX %08lX \n"
  ,TADIN[tt].fname,tsz,isz);
  }
}

static void Fshdisk(void)
{
struct _diskfree_t E,*p;
char buf[STRLX];
int vd,st,k64=64,tmg,fmg,clsz;

vd=_getdrive();getcwd(buf,STRLX);
printf("DEVICE [%c:] DIR=[%s] \n",k64+vd,buf);
printf("DEVICE Free/Total \n");
for(vd=3;vd LT 16;vd++)
  {
  p=&E;st=_getdiskfree(vd,p);
  if(st NE 0)continue;
  clsz=p->sectors_per_cluster*p->bytes_per_sector;      
  tmg=(p->total_clusters*clsz)/MB1;
  fmg=(p->avail_clusters*clsz)/MB1;
  printf("%-6c %8dMB %8dMB CLSZ: %6d \n"
,(k64+vd),fmg,tmg,clsz);
  }
}
/*************************************/

/****** Montecarlo test/code ****/
void Fbulk(void)
{
const int tt=UOW->tt;
const int bulkx=20;
long posli,posl;
int ii,qrows=0;
I32 xkey;

sortIXDB(1);
openTAB(tt,OINSEND);
posli=TADIN[tt].fleng;
for(int qin=0;qin LT bulkx;qin++)
  {
  xkey=1000+rand()%20;
  ii=findkey(1,tt,xkey);if(ii GE 0)continue;

  assert(IXDIN.qix LT IXDBX);
  posl=posli;setRECdflt(tt);
  for(int c=0;c LT MAXCOLS;c++)
    RECUNI[tt].C[c]=xkey +c*(rand()%2);

  writeREC(tt,posl);
  ii=IXDIN.qix;IXDB[ii].tt=tt;IXDB[ii].st=NRMST;
  IXDB[ii].key=xkey;IXDB[ii].RBA=posl;
  matchttii(tt,ii);
/****************************************/
  IXDIN.qix++;posli+=LRECU;qrows++;
  }
myclose();
printf("MSG BULKROWS: %04d \n",qrows);
IXDIN.mofied=TRUE;IXDIN.sorted=FALSE;
}
void buildSQL(int nc)
{
char buf[STRLX],pbuf[STRLX];

strset(buf,0);
for(int i=0;i LT GRAMLEN[nc];i++)
  {
  int token=GRAMR[nc][i];int valor;

  strset(pbuf,0);
  switch(token)
    {
    case WAZ:
      valor=65+rand()%8;
      sprintf(pbuf,"%c ",valor);
      break;
    case WCT:
      strcpy(pbuf,"NUM ");
      break;
    case WCF:
      strcpy(pbuf,"DATA");
      break;
    case WTID:
      {
      valor=65+rand()%TABMAX;
      sprintf(pbuf,"ALEA%c%c ",valor,valor);
      pbuf[6]=0;
      }
      break;
    case W09: 
      valor=1000+rand()%20;
      sprintf(pbuf,"%d ",valor);
      break;
    case WKEYV:
      valor=1000+rand()%20;
      sprintf(pbuf,"%d ",valor);
      break;
    case WCOLID:
      {
      valor=1+rand()%3;
      sprintf(pbuf,"COL%1d",valor);
      pbuf[4]=0;
      }
      break;
    default:
      if(token GE 0)
      {
      valor=findKWOc(token);
      assert(valor GE 0);
      sprintf(pbuf,"%s ",KWOtc[valor].t);
      }
      else assert(0 EQ 11);
      break;
    }
  strcat(buf,pbuf);
  strcat(buf," ");
  }
strncpy(MABUF,buf,STRLX);
}

void Fmontecarlo(void)
{
enum ENUMCMD c;bool b;

for(int q=0;q LT MALBUX;q++)
  {
  do{
    c=rand()%GLNX;
    b=   (c EQ CREATA) OR (c EQ DROPTA)
      OR (c EQ TRUNTA) OR (c EQ INSROW)
      OR (c EQ UPDWHE) OR (c EQ DLTWHE)
      OR (c EQ DLTRANGE) OR (c EQ DLTCOL)
      OR (c EQ SELWHE) OR (c EQ SELKGE) 
      OR (c EQ SELCOL) OR (c EQ SELRANGE)
      OR (c EQ SHTABS) OR (c EQ CHECTA)
      OR (c EQ SELRBA) OR (c EQ BULK);
    }while(!b);
  buildSQL(c);strcpy(MALBU[q],MABUF);
  }
qmalbux=MALBUX;
}

void howcols(void)
{
printf("COL TYP: ");
for(int i=0;i LT CTLISX;i++)printf("%s ",CTLIST[i]);
newl();
printf("COL FUN: ");
for(int i=0;i LT CFLISX;i++)printf("%s ",CFLIST[i]);
newl();
}

static void Fhow(void)
{
char buf[STRLX]; 
char *p;

for(int i=0;i LT GRLINX;i++)
  {
  strncpy(buf,GRAMLIN[i],STRLX);
  strupr(buf);p=strstr(buf,UOW->TOKS[TOK1]);
  if(p NE NULL)printf("%s \n",GRAMLIN[i]);
  }
howcols();
}

static void Fman(void)
{ 
for(int i=0;i LT GRLINX;i++)
  {
  printf("%s \n",GRAMLIN[i]);
  if((i GT 0) AND (i%12 EQ 0))if(Bbreak())break;
  }
}
/**** EXEBOX **********************/
void Fcdoc(void)
{
switch(UOW->NUMCMD)
  {
  case SHDISK: Fshdisk();break;
  case SHFILES: Ffiles();break;
  case CREATEDB:
    Fcontainer(1);
    createfile(SYSDINfname,SIRRW);
    DBDIN.created=TRUE;DBDIN.opened=FALSE;
    createSYSDIN();
    writeSYSDIN();

    IXDIN.qix=0;IXDIN.sorted=TRUE;
    IXDIN.mofied=FALSE;
    break;

  case DROPDB:
    freshSYSDIN();
    for(int t=0;t LT TABMAX;t++)
      {
      if(TADIN[t].created)removetable(t);
      }

    IXDIN.inmem=FALSE;
    DBDIN.opened=FALSE;
    removefile(SYSDINfname);
    DBDIN.created=FALSE;
    Fcontainer(2);
    break;

  case OPENDB:
    readSYSDIN();
    DBDIN.opened=TRUE;

    IXDIN.inmem=FALSE;
    IXDIN.sorted=FALSE;
    break;
  case CLOSEDB:
    writeSYSDIN();
    assert(!IXDIN.inmem);
    DBDIN.opened=FALSE;
    break;
  default: assert(0 EQ 3);break;
  }
}

void Fcdtcr(void)
{
int tt=UOW->tt;

matchSYSDIN();
switch(UOW->NUMCMD)
  {
  case CREATA:
    strcpy(TADIN[tt].tid,UOW->tid);
    for(int c=0;c LT MAXCOLS;c++)
      {
      sprintf(TADIN[tt].colhp[c],"COL%1X",c);
      TADIN[tt].colhp[c][4]=0;
      TADIN[tt].ct[c]=0;TADIN[tt].cf[c]=0;
      }
    createtable(tt);
    printf("TAB & INDEX, Created \n");

    writeSYSDIN();
    break;
  case DROPTA: 
    removetable(tt);
    strset(TADIN[tt].tid,0);
    printf("TAB+IX deleted \n");
    writeSYSDIN();
    break;
  case TRUNTA:
    openTAB(tt,OTRUNCA);myclose();
    truncateINDEX(tt);
    printf("TAB+INDEX truncated \n");
    TADIN[tt].SEQ=1;
    break;
  case CHECTA: Fchecktabins();break;
  case REORTA: Freorgtab();break;
  default: assert(0 EQ 4);break;
  }
matchSYSDIN();
}

static int exebox(void)
{
int tt=UOW->tt;
int cmd=UOW->NUMCMD;
bool ixneed,ixmod;


/************************************ 
CREATEDB,DROPDB,OPENDB,CLOSEDB
,CREATA,DROPTA,TRUNTA,REORTA
************************************/

ixneed=(cmd EQ REORTA)
OR (cmd EQ INSROW)   OR (cmd EQ UPDWHE)
OR (cmd EQ DLTWHE)   OR (cmd EQ DLTRANGE)
OR (cmd EQ DLTCOL)
OR (cmd EQ SELWHE)   OR (cmd EQ SELKGE)
OR (cmd EQ SELRANGE) OR (cmd EQ BULK);

ixmod=(cmd EQ INSROW) OR (cmd EQ BULK)
   OR (cmd EQ DLTWHE) OR (cmd EQ DLTRANGE)
   OR (cmd EQ DLTCOL);

if(ixneed)
  {
  if(!DBDIN.opened)return -1;
  if(!UOW->withtt)return -1;
  if(!TADIN[tt].ireated)return -1;
  }
IXDIN.inmem=FALSE;
if(ixneed)
  {readIXDB(tt);sortIXDB(1);
  }

switch(UOW->NUMCMD)
  {
  case SHDISK: Fcdoc();break;
  case SHFILES: Fcdoc();break;
  case CREATEDB:
    Fcdoc();
    break;
  case DROPDB:
    if(DBDIN.opened)return -1;
    Fcdoc();
    break;
  case OPENDB:
    if(!DBDIN.created)return -2; 
    if(DBDIN.opened)return -1; 
    Fcdoc();
    break;
  case CLOSEDB:
    if(!DBDIN.opened)return -1; 
    Fcdoc();
    break;
  case BACKDB:
    if(!DBDIN.created)return -2; 
    if(DBDIN.opened)return -1; 
    break;

/************ table cmds      ******/
  case CREATA:
    if(!DBDIN.opened)return -1; 
    if(NOT UOW->withtt)return -5;
    Fcdtcr();
    break;
  case RENATA:
    if(!DBDIN.opened)return -1; 
    if(NOT UOW->withtt)return -5;
    if(!TADIN[tt].created)return -6;
    strcpy(TADIN[tt].tid,UOW->TOKS[TOK3]);
    break;

  case COLHP:
    if(!DBDIN.opened)return -1; 
    if(!TADIN[tt].created)return -6;
    {
    char buf[STRLX];int col=UOW->TOKV32[TOK3];
    strcpy(buf,UOW->TOKS[TOK5]);
    strcat(buf,"xxxxxx");buf[TOK4]=0;
    strcpy(TADIN[tt].colhp[col],buf);
    }
    break;
  case COLTYP:
    if(!DBDIN.opened)return -1; 
    if(!TADIN[tt].created)return -6;
    {
    int c=UOW->TOKV32[TOK3];
    TADIN[tt].ct[c]=(int)UOW->TOKV32[TOK5];
    }
    break;
  case COLFUN:
    if(!DBDIN.opened)return -1; 
    if(!TADIN[tt].created)return -6;
    {
    int c=UOW->TOKV32[TOK3];
    TADIN[tt].cf[c]=(int)UOW->TOKV32[TOK5];
    }
    break;

  case DROPTA: 
    if(!DBDIN.opened)return -1; 
    if(NOT UOW->withtt)return -5;
    if(!TADIN[tt].created)return -6;
    Fcdtcr();
    break;
  case TRUNTA:
    if(!DBDIN.opened)return -1; 
    if(NOT UOW->withtt)return -5;
    if(!TADIN[tt].created)return -6;
    Fcdtcr();
    break;
  case CHECTA:
    if(!DBDIN.opened)return -1; 
    Fcdtcr();
    break;
  case REORTA:
    if(!DBDIN.opened)return -1; 
    if(NOT UOW->withtt)return -5;
    if(!TADIN[tt].created)return -6;
    Fcdtcr();
    break;

/*************************************/
  case SELRBA:
    if(NOT UOW->withtt)return -1;
    if(!TADIN[tt].created)return -6;
    Fselwherba();
    break;
  case SELWHE:
    if(NOT UOW->withtt)return -1;
    if(NOT UOW->withkey)return -5;
    if(!TADIN[tt].created)return -6;
    Fselwhekey(1);
    break;
  case SELKGE:
    if(NOT UOW->withtt)return -5;
    if(NOT UOW->withkey)return -8;
    if(!TADIN[tt].created)return -6;
    Fselwhekey(2);
    break;

  case INSROW:
    if(NOT UOW->withtt)return -5;
    if(NOT UOW->withkey)return -8;
    if(NOT UOW->withint)return -9;
    if(!TADIN[tt].created)return -6;
    Finsrow();
    break;
  case UPDWHE:
    if(NOT UOW->withtt)return -5;
    if(NOT UOW->withkey)return -8;
    if(NOT UOW->withcol)return -11;
    if(NOT UOW->withint)return -9;
    if(!TADIN[tt].created)return -6;
    Fupdwhe();
    break;
  case DLTWHE:
    if(NOT UOW->withtt)return -5;
    if(NOT UOW->withkey)return -8;
    if(!TADIN[tt].created)return -6;
    Fdltwhe();
    break;
/****************************************/
  case SELRANGE:
    if(NOT UOW->withtt)return -5;
    if(NOT UOW->withkey)return -8;
    if(NOT UOW->withint)return -1;
    if(!TADIN[tt].created)return -6;
    newpage(tt);openTAB(tt,OINDEXR);
    indexfull();myclose();
    break;
  case DLTRANGE:
    if(NOT UOW->withtt)return -5;
    if(NOT UOW->withkey)return -8;
    if(NOT UOW->withint)return -1;
    if(!TADIN[tt].created)return -6;

    openTAB(tt,OINDEXW);indexfull();myclose();
    IXDIN.mofied=TRUE;IXDIN.sorted=FALSE;
    break;

  case SELFIR:
    if(NOT UOW->withtt)return -5;
    if(!TADIN[tt].created)return -6;
    Fselfirst();
    break;
  case SELCOL:
    if(NOT UOW->withtt)return -5;
    if(NOT UOW->withcol)return -1;
    if(NOT UOW->withint)return -1;
    if(!TADIN[tt].created)return -6;
    newpage(tt);tabfullscan();
    break;
  case DLTCOL:
    if(NOT UOW->withtt)return -5;
    if(NOT UOW->withcol)return -1;
    if(NOT UOW->withint)return -1;
    if(!TADIN[tt].created)return -6;
    tabfullscan();
    printf("MSG DLTROWS:%04d \n",UOW->QROWS);
    break;
  case SELGRP:
    if(NOT UOW->withtt)return -5;
    if(NOT UOW->withcol)return -1;
    if(!TADIN[tt].created)return -6;
    Fselgroup();
    break;
/***********************************/
  case DESTAB: Fdescribe();break;
  case SHTABS: Fshowtabins();break;
/**************************************/
  case TESTCODE: Fmontecarlo();break;
  case BULK: 
    if(!DBDIN.opened)return -1; 
    if(NOT UOW->withtt)return -5;
    if(!TADIN[tt].created)return -6;
    Fbulk();break;
  case HHOW: Fhow();break;
  case MAN: Fman();break;
  case QUIT:
    if(DBDIN.opened)return -1;
    break;
  default: 
    printf("MSG UNDEFCMD%04d \n",UOW->NUMCMD);
    assert(0 EQ 7);
    return -1;
    break;
  }

if(IXDIN.inmem)
  {
  if(ixmod OR IXDIN.mofied)writeIXDB();
  IXDIN.inmem=FALSE;
  }

return +1;
}


/******************************************/
/******* END SOURCE CODE ***************/
/******************************************/


