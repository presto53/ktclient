#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_LEN 8192
#define MAX_KEY_LEN 1024

int intsock;
char errbuf[BUF_LEN];
char *magicbuf;
char buf[BUF_LEN];
struct sockaddr_in sock_in;
struct hostent *phe;

// timer var
struct timeval start, end;
float mtime;
long seconds, useconds;

void kt_timer_start() {
        gettimeofday(&start, NULL);
}

float kt_timer_stop(char *operation) {
        gettimeofday(&end, NULL);
        seconds  = end.tv_sec  - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        mtime = ((seconds) * 1000 + useconds/1000.0);
        printf("Pid %d %s elapsed time: %.3lf milliseconds\n", getpid(), operation, mtime);
	return mtime;
}

int tycoon_set(int ktsock, char *skey, char *svalue, uint64_t sxt) {
	
	uint8_t kt_set_magic = 0xB8;
	uint32_t flags = 0x00;
	uint32_t rnum = 0x01;
	uint16_t dbidx = 0x00;
	uint32_t ksiz = strlen(skey);
	uint32_t vsiz = strlen(svalue);
	uint32_t magicbufsize = sizeof(kt_set_magic) + sizeof(flags) + sizeof(rnum) + sizeof(dbidx) + sizeof(ksiz) + sizeof(vsiz) + sizeof(sxt);
	int offset = 0x00;

	flags = htonl(flags);
	rnum = htonl(rnum);
	dbidx = htonl(dbidx);
	ksiz = htonl(ksiz);
	vsiz = htonl(vsiz);
	sxt = htonl(sxt);

	magicbuf = (char*)malloc(magicbufsize);
	memset(magicbuf,0,magicbufsize);
	
	memcpy(magicbuf,&kt_set_magic,sizeof(kt_set_magic));
	offset=sizeof(kt_set_magic);

	memcpy(magicbuf+offset,&flags,sizeof(flags));
	offset+=sizeof(flags);

	memcpy(magicbuf+offset,&rnum,sizeof(rnum));
	offset+=sizeof(rnum);

	memcpy(magicbuf+offset,&dbidx,sizeof(dbidx));
	offset+=sizeof(dbidx);

	memcpy(magicbuf+offset,&ksiz,sizeof(ksiz));
        offset+=sizeof(ksiz);

        memcpy(magicbuf+offset,&vsiz,sizeof(vsiz));
        offset+=sizeof(vsiz);

	memcpy(magicbuf+offset,&sxt,sizeof(sxt));
	
	if(connect(ktsock, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0) {
		return -1;
        }
        else {
		write(ktsock, magicbuf, magicbufsize);

		int k;
		offset = 0x00;
		for( k=0; k<(strlen(skey)/BUF_LEN)+1; k++) {
			if( strlen(skey+offset) < BUF_LEN ) {
				memcpy(buf,skey+offset,strlen(skey+offset));
				write(ktsock, buf, strlen(skey+offset));
			}
			else {
				memcpy(buf,skey+offset,BUF_LEN);
				write(ktsock, buf, BUF_LEN);
			}
			offset+=BUF_LEN;
		}

		offset = 0x00;
                for( k=0; k<(strlen(svalue)/BUF_LEN)+1; k++) {
			if( strlen(svalue+offset) < BUF_LEN ) {
                                memcpy(buf,svalue+offset,strlen(svalue+offset));
				write(ktsock, buf, strlen(svalue+offset));
                        }
                        else {
                                memcpy(buf,svalue+offset,BUF_LEN);
				write(ktsock, buf, BUF_LEN);
                        }
                        offset+=BUF_LEN;
		}

		free(magicbuf);

                if (read(ktsock, &errbuf, sizeof(errbuf))>0){
                        if ((int)errbuf[0] != kt_set_magic) {
				return -1;
			}
			else {
				return 0;
			}
                }
                else {
                        return -1;
                }
        }
}

int tycoon_get(unsigned char *gkey) {

}

int tycoon_remove(unsigned char *rkey) {

}

int tycoon_connect(char *thost, char *tport) {
	memset(&sock_in, 0, sizeof(sock_in));
        sock_in.sin_family = AF_INET;
        sock_in.sin_port = htons((unsigned short)atoi(tport));
        if(phe = gethostbyname(thost)) memcpy(&sock_in.sin_addr, phe->h_addr, phe->h_length);
        intsock = socket(AF_INET, SOCK_STREAM, 0);
	return intsock;
}

int tycoon_close(int s) {

}
