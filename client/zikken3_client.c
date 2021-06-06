/*
 * print-http-request.c -- HTTPの要求を画面に表示する(引き通付き、枠組みだけ)
 * ~yas/syspro/ipc/print-http-request.c
 */

#include <stdio.h>
#include <stdlib.h>	/* exit() */
#include <string.h>	/* memset(), memcpy() */
#include <sys/types.h>	/* socket() */
#include <sys/socket.h>	/* socket() */
#include <netinet/in.h>	/* struct sockaddr_in */
#include <netdb.h>	/* getaddrinfo() */
#include <string.h>	/* strlen() */
#include <unistd.h>	/* close() */
#define BUFFERSIZE 1024

extern	int tcp_connect( char *server, int portno );
extern  int fdopen_sock( int sock, FILE **inp, FILE **outp );
extern  int http_send_request( char *host, int portno,char *file,char *command,FILE *out );
extern  int http_client_one( char *host, int portno, char *file,char *command);
extern  int http_receive_reply( FILE *in, char buf[], int size );
extern  int get_reply(FILE *in,char buf[], int size,char *file);
extern  int put_reply(FILE *out,char buf[], int size,char *file);
extern  int dir_reply(FILE *in,char buf[],int size);

int main( int argc, char *argv[] )
{
	char *host, *file;
	int portno;
    char *command;
	if( argc != 5 &&argc!=4) {
		fprintf(stderr,"Usage: %s host port Command file or host port DIR\n",argv[0] );
		exit( 1 );
	}
	host = argv[1];
    portno=strtol( argv[2],0,10);
    command =argv[3];
	file = argv[4];
	int err=http_client_one(host,portno,file,command);
}

int
http_client_one( char *host, int portno, char *file,char *command)
{
	int sock ;
	FILE *in, *out ;
	char rbuf[BUFFERSIZE];
	int res;

	sock = tcp_connect(host,portno);
	if( sock<0 )
		return( 1 );
	if( fdopen_sock(sock,&in,&out) < 0 )
	{
		fprintf(stderr,"fdooen()\n");
		close( sock );
		return( 1 );
	}
	res = http_send_request(host,portno,file,command,out);
	if( res < 0 )
	{
		fprintf(stderr,"fprintf()\n");
		fclose( in );
		fclose( out );
		return( 1 );
	}

    if(strcmp(command,"GET")==0)
        res= get_reply(in,rbuf,BUFFERSIZE,file);
    if(strcmp(command,"PUT")==0)
        res=put_reply(out,rbuf,BUFFERSIZE,file);
    if(strcmp(command,"DIR")==0)
        res=dir_reply(in,rbuf,BUFFERSIZE);
	/*res = http_receive_reply( in, rbuf, BUFFERSIZE );
	if( res < 0 )
	{
		fprintf(stderr,"fprintf()\n");
		fclose( in );
		fclose( out );
		return( 1 );
	}
    */
	fclose( in );
	fclose( out );
	return( 0 );
}

int get_reply(FILE *in,char buf[],int size,char *file)
{
    FILE *fp;
    fp=fopen(file,"w");

    while((fgets(buf,BUFFERSIZE,in)) != NULL)
    {
		fprintf(fp,"%s",buf);
	}
    fclose(fp);
	return strlen(buf);

}
int put_reply(FILE *out,char buf[],int size,char *file)
{
    FILE *fp;
    fp=fopen(file,"r");

    while((fgets(buf,BUFFERSIZE,fp)) != NULL)
    {
		fprintf(out,"%s",buf);
	}
    fclose(fp);


	return strlen(buf);

}

int dir_reply(FILE *in,char buf[],int size)
{
    
	char *res;
	while((res = fgets( buf, size, in ))!=NULL){
		fprintf(stdout,"%s",buf);
	  /* receive a reply message */
	}
	return( strlen(buf) );
}


int
http_send_request( char *host, int portno,char *file,char *command,FILE *out ) {

    int res=fprintf(out,"%s %s\r\n\r\n",command,file);
		return (res);

}

int
http_receive_reply( FILE *in, char buf[], int size )
{
	char *res;
	while((res = fgets( buf, size, in ))!=NULL){
		fprintf(stdout,"%s",buf);
	  /* receive a reply message */
		if( !res )
			return( -1 );
	}
	return( strlen(buf) );

}

#define PORTNO_BUFSIZE 30
int
tcp_connect( char *server, int portno )
{
	struct addrinfo hints, *ai, *p;
	char portno_str[PORTNO_BUFSIZE];
	int s, err;
	snprintf( portno_str,sizeof(portno_str),"%d",portno );
	memset( &hints, 0, sizeof(hints) );
	hints.ai_socktype = SOCK_STREAM;
	if( (err = getaddrinfo( server, portno_str, &hints, &ai )) )
	{
		fprintf(stderr,"unknown server %s (%s)\n",server,
			gai_strerror(err) );
		goto error0;
	}
	for( p=ai ; p ; p=p->ai_next )
	{
		if( (s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0 )
		{
			perror("socket");
			goto error1;
		}
		if( connect(s, p->ai_addr, p->ai_addrlen) >= 0 )
		{
			break;
		}
		else
		{
			close( s );
		}
	}
	freeaddrinfo( ai );
	return( s );
error1:
	freeaddrinfo( ai );
error0:
	return( -1 );
}



int
fdopen_sock( int sock, FILE **inp, FILE **outp )
{
	int sock2 ;
	if( (sock2=dup(sock)) < 0 )
	{
		return( -1 );
	}
	if( (*inp = fdopen( sock2, "r" )) == NULL )
	{
		close( sock2 );
		return( -1 );
	}
	if( (*outp = fdopen( sock, "w" )) == NULL )
	{
		fclose( *inp );
		*inp = 0 ;
		return( -1 );
	}
	setvbuf(*outp, (char *)NULL, _IONBF, 0);
	return( 0 );
}
