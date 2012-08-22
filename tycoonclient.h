/* File tycoonclient.h */
#include <netdb.h>
#define KT_BUF_LEN 8192

int intsock;
char kterrbuf[5];
char *ktmagicbuf;
char *ktreadbuf;
struct sockaddr_in sock_in;
struct hostent *phe;
int ktoffset = 0x00;

/* timer var */
struct timeval ktstart, ktend;
float ktmtime;
long ktseconds, ktuseconds;

/* Timer start function */
void kt_timer_start();

/* Timer stop function */
float kt_timer_stop(char*);

/* Internal function to write data to KT */
void tycoon_write(int, char*);

/* Internal function to read data from KT */
char* tycoon_read(int);

/* Function to set data to KT */
int tycoon_set(int, char*, char*, uint64_t);

/* Function to get data from KT */
int tycoon_get(int, char*);

/* Function to remove data from KT */
int tycoon_remove(int, char*);

/* Function to connect to KT */
int tycoon_connect(char*, char*);

/* Function to close connection to KT */
void tycoon_close(int);

