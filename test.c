#include <stdio.h>
#include <string.h>
#include "tycoonclient.h"

//char *key="new";
//char *value="test";
char *host="127.0.0.1";
char *port="1978";
int xt=500;
int sock;
char *key;
char *value;

int main(void)
{

int k;

value = malloc(8192);
for(k=0;k<8192;k++){
value[k]='1';
}

key = malloc(8192);
for(k=0;k<8192;k++){
key[k]='2';
}

sock = tycoon_connect(host, port);
kt_timer_start();
tycoon_set(sock, key, value, xt);
kt_timer_stop("set");

free(value);
free(key);

}
