

/********************************/
/********************************/
#pragma pack(16)

#include <stdio.h>
#include <conio.h>
#include <crtdefs.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include <string.H>
#include <mem.h>
#include <stdbool.h>
#define FALSE 0
#define TRUE 1

#include <limits.h>
#include <sys/stat.h>
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <share.h>
#include <process.h>
#include <search.h>
#include <intrin.H>

#define EQ ==
#define NE !=
#define GE >=
#define LE <=
#define GT >
#define LT <
 
#define AND &&
#define OR ||
#define NOT !

#define bitand &
#define bitor |
#define bitxor ^
#define bitnot ~

typedef int8_t I8; 
typedef uint8_t U8; 
typedef int16_t I16; 
typedef uint16_t U16; 
typedef int32_t I32; 
typedef uint32_t U32;
typedef int64_t I64; 
typedef uint64_t U64; 

#define MB16 16777216
#define MB1  1048576
#define K64  65536
#define SIRRW (S_IFREG|S_IREAD|S_IWRITE)
#define OACR O_CREATED|O_TRUNC
#define OARO O_EXCL|O_BINARY|O_RDONLY
#define OARW O_EXCL|O_BINARY|O_RDWR
#define OASEQ O_SEQUENTIAL
#define OARAN O_RANDOM

#define OCREATE (OACR|OARW|OARAN)
#define OTRUNCA (O_TRUNC|OARW)
#define OSYSRO  (OARO|OASEQ)
#define OSYSTW  (O_TRUNC|OARW|OASEQ)
#define OTABRO  (OARO|OARAN)
#define OINSLOT (OARW|OARAN)
#define OINSEND (O_APPEND|OARW|OARAN)
#define OFULTAB (OARO|OASEQ)
#define OINDEXR (OARO|OARAN)
#define OINDEXW (OARW|OARAN)
/* O_SHORT_LIVED:.O_TEMPORARY: close es delete */
#define TOK0 0
#define TOK1 1 
#define TOK2 2
#define TOK3 3
#define TOK4 4
#define TOK5 5
#define TOK6 6
#define TOK7 7
#define TOK8 8
#define TOK9 9

/******** filenames *************/
#define SYSDINfname "DATA\\SYS$DD.TSP"
#define FPREFIX     "DATA\\TB"
#define IPREFIX     "INDEX\\IX"

#define DIRDATA     ".\\DATA"
#define DIRINDEX    ".\\INDEX"
#define FEVEFNAME   "EVNT$.LOG"

