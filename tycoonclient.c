#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "tycoonclient.h"

/* timer var */
struct timeval ktstart, ktend;
float ktmtime;
long ktseconds, ktuseconds;

/* Timer start function */
void kt_timer_start() {
        gettimeofday(&ktstart, NULL);
}

/* Timer stop function */
float kt_timer_stop(char *operation) {
        gettimeofday(&ktend, NULL);
        ktseconds  = ktend.tv_sec  - ktstart.tv_sec;
        ktuseconds = ktend.tv_usec - ktstart.tv_usec;
        ktmtime = ((ktseconds) * 1000 + ktuseconds/1000.0);
        //printf("Pid %d %s elapsed time: %.3lf milliseconds\n", getpid(), operation, ktmtime);
	return ktmtime;
}

/* Internal function to write data to KT */
void tycoon_write(int ktsock, char *data) {
        int k;
	char buf[KT_BUF_LEN];
	int datasize;
        ktoffset = 0x00;
	memset(buf, 0, KT_BUF_LEN);
        for( k=0; k < (strlen(data) / KT_BUF_LEN)+1; k++) {
                if( strlen(data + ktoffset) < KT_BUF_LEN ) {
			datasize = strlen(data + ktoffset);
                        memcpy(buf, data + ktoffset, datasize);
                        write(ktsock, buf, datasize);
                }
                else {
                        memcpy(buf, data + ktoffset, KT_BUF_LEN);
                        write(ktsock, buf, KT_BUF_LEN);
                }
                ktoffset += KT_BUF_LEN;
        }
}

/* Internal function to read data from KT */
char* tycoon_read(int ktsock) {
	uint8_t resp_magic;
	uint32_t hits;
	uint16_t dbidx;
	uint32_t ksiz;
	uint32_t vsiz;
	int64_t xt;
	uint32_t ktmagicbufsize = sizeof(resp_magic) + sizeof(hits) + sizeof(dbidx) + sizeof(ksiz) + sizeof(vsiz) + sizeof(xt);
	ktoffset = 0x00;
	char *err = 0;
	int btmp;

	ktmagicbuf = (char*)malloc(ktmagicbufsize);
        memset(ktmagicbuf, 0, ktmagicbufsize);

	// read header with response magic and other information from socket
	while( ktmagicbufsize > 0 ) {
                btmp = read(ktsock, ktmagicbuf + ktoffset, ktmagicbufsize);
                if( btmp < 0 ) return err;
                else if ( btmp == 0 ) break;
                ktmagicbufsize -= btmp;
                ktoffset += btmp;
        }
	
	// get key size
	ktoffset = sizeof(resp_magic) + sizeof(hits) + sizeof(dbidx);
	memcpy(&ksiz, ktmagicbuf + ktoffset, sizeof(ksiz));
	ksiz=ntohl(ksiz);
	
	// get value size
	ktoffset = sizeof(resp_magic) + sizeof(hits) + sizeof(dbidx) + sizeof(ksiz);
	memcpy(&vsiz, ktmagicbuf + ktoffset, sizeof(vsiz));
	vsiz=ntohl(vsiz);
	
	free(ktreadbuf);
	ktreadbuf = calloc(ksiz,1);
	
	// read key from socket
	ktoffset = 0x00;
        while( ksiz > 0 ) {
                btmp = read(ktsock, ktreadbuf + ktoffset, ksiz);
                if( btmp < 0 ) return err;
                else if ( btmp == 0 ) break;
                ksiz -= btmp;
                ktoffset += btmp;
        }

	// drop buf with key and realloc for value
	free(ktreadbuf);
	ktreadbuf = calloc(vsiz,1);

	// get value from socket
	ktoffset = 0x00;
	while( vsiz > 0 ) {
		btmp = read(ktsock, ktreadbuf + ktoffset, vsiz);
		if( btmp < 0 ) return err;
		else if ( btmp == 0 ) break;
		vsiz -= btmp;
		ktoffset += btmp;
	}

	free(ktmagicbuf);
	return ktreadbuf;
}

/* Function to set data to KT */
int tycoon_set(int ktsock, char *skey, char *svalue, uint64_t sxt) {
	
	uint8_t kt_set_magic = 0xB8;
	uint32_t flags = 0x00;
	uint32_t rnum = 0x01;
	uint16_t dbidx = 0x00;
	uint32_t ksiz = strlen(skey);
	uint32_t vsiz = strlen(svalue);
	uint32_t ktmagicbufsize = sizeof(kt_set_magic) + sizeof(flags) + sizeof(rnum) + sizeof(dbidx) + sizeof(ksiz) + sizeof(vsiz) + sizeof(sxt);
	ktoffset = 0x00;

	flags = htonl(flags);
	rnum = htonl(rnum);
	dbidx = htonl(dbidx);
	ksiz = htonl(ksiz);
	vsiz = htonl(vsiz);
	sxt = htonl(sxt);

	if(connect(ktsock, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0) {
		return -1;
        }
        else {
		ktmagicbuf = (char*)malloc(ktmagicbufsize);
        	memset(ktmagicbuf, 0, ktmagicbufsize);
		
		// Create magic header with set magic, key lenght, value lenght, expiration timeout and other.
	        memcpy(ktmagicbuf, &kt_set_magic, sizeof(kt_set_magic));
	        ktoffset = sizeof(kt_set_magic);
        	memcpy(ktmagicbuf + ktoffset, &flags, sizeof(flags));
	        ktoffset += sizeof(flags);
	        memcpy(ktmagicbuf + ktoffset, &rnum, sizeof(rnum));
	        ktoffset += sizeof(rnum);
	        memcpy(ktmagicbuf + ktoffset, &dbidx, sizeof(dbidx));
	        ktoffset += sizeof(dbidx);
	        memcpy(ktmagicbuf + ktoffset, &ksiz, sizeof(ksiz));
	        ktoffset += sizeof(ksiz);
	        memcpy(ktmagicbuf + ktoffset, &vsiz, sizeof(vsiz));
	        ktoffset += sizeof(vsiz);
	        memcpy(ktmagicbuf + ktoffset, &sxt, sizeof(sxt));
		
		// Write magic header to socket
		write(ktsock, ktmagicbuf, ktmagicbufsize);

		// Write key and value to KT
		tycoon_write(ktsock, skey);
		tycoon_write(ktsock, svalue);
                free(ktmagicbuf);

                if (read(ktsock, &kterrbuf, sizeof(kterrbuf))>0){
                        if ((unsigned char)kterrbuf[0] != (unsigned char)kt_set_magic) {
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

	return -1;
}

/* Function to get data from KT */
int tycoon_get(int ktsock, char *gkey) {
	uint8_t kt_get_magic = 0xBA;
        uint32_t flags = 0x00;
        uint32_t rnum = 0x01;
        uint16_t dbidx = 0x00;
        uint32_t ksiz = strlen(gkey);
        uint32_t ktmagicbufsize = sizeof(kt_get_magic) + sizeof(flags) + sizeof(rnum) + sizeof(dbidx) + sizeof(ksiz);
        ktoffset = 0x00;

        flags = htonl(flags);
        rnum = htonl(rnum);
        dbidx = htonl(dbidx);
        ksiz = htonl(ksiz);

	if(connect(ktsock, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0) {
                return -1;
        }
        else {
	        ktmagicbuf = (char*)malloc(ktmagicbufsize);
        	memset(ktmagicbuf, 0, ktmagicbufsize);

		// Create magic header with get magic, key lenght and other.
	        memcpy(ktmagicbuf, &kt_get_magic, sizeof(kt_get_magic));
	        ktoffset = sizeof(kt_get_magic);
	        memcpy(ktmagicbuf + ktoffset, &flags, sizeof(flags));
	        ktoffset += sizeof(flags);
	        memcpy(ktmagicbuf + ktoffset, &rnum, sizeof(rnum));
	        ktoffset += sizeof(rnum);
	        memcpy(ktmagicbuf + ktoffset, &dbidx, sizeof(dbidx));
	        ktoffset += sizeof(dbidx);
	        memcpy(ktmagicbuf + ktoffset, &ksiz, sizeof(ksiz));

		// Write magic header to socket
                write(ktsock, ktmagicbuf, ktmagicbufsize);

		// Write key to KT
		tycoon_write(ktsock, gkey);

		free(ktmagicbuf);
		
		// Read response
		if (tycoon_read(ktsock) == 0) {
			return -1;
		}
	}

	return 0;
}

/* Function to remove data from KT */
int tycoon_remove(int ktsock, char *dkey) {
        uint8_t kt_del_magic = 0xB9;
        uint32_t flags = 0x00;
        uint32_t rnum = 0x01;
        uint16_t dbidx = 0x00;
        uint32_t ksiz = strlen(dkey);
        uint32_t ktmagicbufsize = sizeof(kt_del_magic) + sizeof(flags) + sizeof(rnum) + sizeof(dbidx) + sizeof(ksiz);
        ktoffset = 0x00;

        flags = htonl(flags);
        rnum = htonl(rnum);
        dbidx = htonl(dbidx);
        ksiz = htonl(ksiz);

        if(connect(ktsock, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0) {
                return -1;
        }
        else {
		ktmagicbuf = (char*)malloc(ktmagicbufsize);
	        memset(ktmagicbuf, 0, ktmagicbufsize);
	        memcpy(ktmagicbuf, &kt_del_magic, sizeof(kt_del_magic));
	        ktoffset = sizeof(kt_del_magic);
	        memcpy(ktmagicbuf + ktoffset, &flags, sizeof(flags));
	        ktoffset += sizeof(flags);
	        memcpy(ktmagicbuf + ktoffset, &rnum, sizeof(rnum));
	        ktoffset += sizeof(rnum);
	        memcpy(ktmagicbuf + ktoffset, &dbidx, sizeof(dbidx));
	        ktoffset += sizeof(dbidx);
	        memcpy(ktmagicbuf + ktoffset, &ksiz, sizeof(ksiz));

                write(ktsock, ktmagicbuf, ktmagicbufsize);
                tycoon_write(ktsock, dkey);
                free(ktmagicbuf);

		if (read(ktsock, &kterrbuf, sizeof(kterrbuf))>0){
                        if ((unsigned char)kterrbuf[0] != (unsigned char)kt_del_magic) {
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


/* Function to connect to KT */
/* Return -1 if fail and socket id otherwise */
int tycoon_connect(char *thost, char *tport) {
	memset(&sock_in, 0, sizeof(sock_in));
        sock_in.sin_family = AF_INET;
        sock_in.sin_port = htons((unsigned short)atoi(tport));
        if(phe = gethostbyname(thost)) memcpy(&sock_in.sin_addr, phe->h_addr, phe->h_length);
        intsock = socket(AF_INET, SOCK_STREAM, 0);
	ktreadbuf = malloc(KT_BUF_LEN);
	return intsock;
}

/* Function to close connection to KT */
void tycoon_close(int s) {
	free(ktreadbuf);
	close(s);
}
