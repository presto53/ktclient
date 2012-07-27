#include <stdio.h>
#include <string.h>
#include "tycoonclient.h"

char *key="new";
char *value="test";
char *host="127.0.0.1";
char *port="1978";
int xt=500;
int sock;
char *key;
char *value;

int main(void)
{

int k;

/*
value = malloc(8192);
for(k=0;k<8192;k++){
value[k]='1';
}

key = malloc(8192);
for(k=0;k<8192;k++){
key[k]='2';
}
*/

sock = tycoon_connect(host, port);
kt_timer_start();
//tycoon_set(sock, key, value, xt);
//tycoon_get(sock, key);
tycoon_remove(sock, key);
tycoon_close(sock);
kt_timer_stop("set");

/*
free(value);
free(key);
*/
/*
	p = 1024;
	c = 18342;
	printf("P: %i\nP2: %i\n", p/8192, p%8192);
	printf("C: %i\nC2: %i\n", c/8192, c%8192);
	return 0;
*/
}
