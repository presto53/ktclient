#include <stdio.h>
#include <string.h>
#include "tycoonclient.h"

char *key="new";
//char *value="test";
char *host="87.242.75.49";
//char *port="31300";
char *port="1978";
int xt=500;
int sock;
char *key;
char *value;
char *result;

int main(void)
{

int k;

/**/
value = malloc(100*8192);
for(k=0;k<100*8192;k++){
value[k]='1';
}

/*
key = malloc(1024*8192);
for(k=0;k<1024*8192;k++){
key[k]='2';
}
*/
/*
sock = tycoon_connect(host, port);
kt_timer_start();
tycoon_set(sock, key, value, xt);
//tycoon_get(sock, key);
//tycoon_remove(sock, key);
tycoon_close(sock);
kt_timer_stop("set");
*/
sock = tycoon_connect(host, port);
kt_timer_start();
tycoon_get(sock, key);
//printf("String: %s\n", readbuf);

tycoon_close(sock);
kt_timer_stop("get");

/**/
free(value);
//free(key);
/**/
}
