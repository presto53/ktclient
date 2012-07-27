#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_LEN 8192

int intsock;
char errbuf[5];
char *magicbuf;
char *readbuf;
char buf[BUF_LEN];
struct sockaddr_in sock_in;
struct hostent *phe;
int offset = 0x00;

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

void tycoon_write(int ktsock,char *data) {
        int k;
        offset = 0x00;
	memset(buf,0,BUF_LEN);
        for( k=0; k<(strlen(data)/BUF_LEN)+1; k++) {
                if( strlen(data+offset) < BUF_LEN ) {
                        memcpy(buf,data+offset,strlen(data+offset));
                        write(ktsock, buf, strlen(data+offset));
                }
                else {
                        memcpy(buf,data+offset,BUF_LEN);
                        write(ktsock, buf, BUF_LEN);
                }
                offset+=BUF_LEN;
        }
}

char* tycoon_read(int ktsock) {
	uint8_t resp_magic;
	uint32_t hits;
	uint16_t dbidx;
	uint32_t ksiz;
	uint32_t vsiz;
	int64_t xt;
	uint32_t magicbufsize = sizeof(resp_magic) + sizeof(hits) + sizeof(dbidx) + sizeof(ksiz) + sizeof(vsiz) + sizeof(xt);
	offset = 0x00;

	magicbuf = (char*)malloc(magicbufsize);
        memset(magicbuf,0,magicbufsize);

	read(ktsock, magicbuf, magicbufsize);

	offset = sizeof(resp_magic) + sizeof(hits) + sizeof(dbidx);
	memcpy(&ksiz,magicbuf+offset,sizeof(ksiz));
	ksiz=ntohl(ksiz);
	
	offset = sizeof(resp_magic) + sizeof(hits) + sizeof(dbidx) + sizeof(ksiz);
	memcpy(&vsiz,magicbuf+offset,sizeof(vsiz));
	vsiz=ntohl(vsiz);
	
	readbuf = (char*)realloc(readbuf, ksiz);
	read(ktsock, readbuf, ksiz);
	readbuf = realloc(readbuf, vsiz);
	read(ktsock, readbuf, vsiz);		
	free(magicbuf);
	return readbuf;
}

int tycoon_set(int ktsock, char *skey, char *svalue, uint64_t sxt) {
	
	uint8_t kt_set_magic = 0xB8;
	uint32_t flags = 0x00;
	uint32_t rnum = 0x01;
	uint16_t dbidx = 0x00;
	uint32_t ksiz = strlen(skey);
	uint32_t vsiz = strlen(svalue);
	uint32_t magicbufsize = sizeof(kt_set_magic) + sizeof(flags) + sizeof(rnum) + sizeof(dbidx) + sizeof(ksiz) + sizeof(vsiz) + sizeof(sxt);
	offset = 0x00;

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
		free(magicbuf);
		return -1;
        }
        else {
		write(ktsock, magicbuf, magicbufsize);

		tycoon_write(ktsock,skey);
		tycoon_write(ktsock,svalue);
                free(magicbuf);

                if (read(ktsock, &errbuf, sizeof(errbuf))>0){
                        if ((unsigned char)errbuf[0] != (unsigned char)kt_set_magic) {
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

char* tycoon_get(int ktsock, char *gkey) {
	uint8_t kt_get_magic = 0xBA;
        uint32_t flags = 0x00;
        uint32_t rnum = 0x01;
        uint16_t dbidx = 0x00;
        uint32_t ksiz = strlen(gkey);
        uint32_t magicbufsize = sizeof(kt_get_magic) + sizeof(flags) + sizeof(rnum) + sizeof(dbidx) + sizeof(ksiz);
	char *value;
	char *error="0x00";
        offset = 0x00;

        flags = htonl(flags);
        rnum = htonl(rnum);
        dbidx = htonl(dbidx);
        ksiz = htonl(ksiz);

        magicbuf = (char*)malloc(magicbufsize);
        memset(magicbuf,0,magicbufsize);

        memcpy(magicbuf,&kt_get_magic,sizeof(kt_get_magic));
        offset=sizeof(kt_get_magic);

        memcpy(magicbuf+offset,&flags,sizeof(flags));
        offset+=sizeof(flags);

        memcpy(magicbuf+offset,&rnum,sizeof(rnum));
        offset+=sizeof(rnum);

        memcpy(magicbuf+offset,&dbidx,sizeof(dbidx));
        offset+=sizeof(dbidx);

        memcpy(magicbuf+offset,&ksiz,sizeof(ksiz));

	if(connect(ktsock, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0) {
		free(magicbuf);
                return error;
        }
        else {
                write(ktsock, magicbuf, magicbufsize);
		tycoon_write(ktsock,gkey);
		free(magicbuf);
		value = tycoon_read(ktsock);
		return value;	
	}
}

int tycoon_remove(int ktsock, char *dkey) {
        uint8_t kt_del_magic = 0xB9;
        uint32_t flags = 0x00;
        uint32_t rnum = 0x01;
        uint16_t dbidx = 0x00;
        uint32_t ksiz = strlen(dkey);
        uint32_t magicbufsize = sizeof(kt_del_magic) + sizeof(flags) + sizeof(rnum) + sizeof(dbidx) + sizeof(ksiz);
        offset = 0x00;

        flags = htonl(flags);
        rnum = htonl(rnum);
        dbidx = htonl(dbidx);
        ksiz = htonl(ksiz);

        magicbuf = (char*)malloc(magicbufsize);
        memset(magicbuf,0,magicbufsize);

        memcpy(magicbuf,&kt_del_magic,sizeof(kt_del_magic));
        offset=sizeof(kt_del_magic);

        memcpy(magicbuf+offset,&flags,sizeof(flags));
        offset+=sizeof(flags);

        memcpy(magicbuf+offset,&rnum,sizeof(rnum));
        offset+=sizeof(rnum);

        memcpy(magicbuf+offset,&dbidx,sizeof(dbidx));
        offset+=sizeof(dbidx);

        memcpy(magicbuf+offset,&ksiz,sizeof(ksiz));

        if(connect(ktsock, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0) {
		free(magicbuf);
                return -1;
        }
        else {
                write(ktsock, magicbuf, magicbufsize);
                tycoon_write(ktsock,dkey);
                free(magicbuf);

		if (read(ktsock, &errbuf, sizeof(errbuf))>0){
                        if ((unsigned char)errbuf[0] != (unsigned char)kt_del_magic) {
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

int tycoon_connect(char *thost, char *tport) {
	memset(&sock_in, 0, sizeof(sock_in));
        sock_in.sin_family = AF_INET;
        sock_in.sin_port = htons((unsigned short)atoi(tport));
        if(phe = gethostbyname(thost)) memcpy(&sock_in.sin_addr, phe->h_addr, phe->h_length);
        intsock = socket(AF_INET, SOCK_STREAM, 0);
	readbuf=malloc(BUF_LEN);
	return intsock;
}

int tycoon_close(int s) {
	free(readbuf);
	close(s);
}
