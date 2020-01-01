#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>

#define MAXLEN   400

char  sbuf[MAXLEN];
char  obuf[MAXLEN];
char  hwaddr[100];
char  ipaddr[100];

void get_system_info(void)
{
    int   status;
    int   fd;
    char  *s1;
    char  *s2;
    char  *i1;

    status = system("uname -a > /tmp/aaa_tmp1.txt");
    status = system("ifconfig eth1 >> /tmp/aaa_tmp1.txt");

    fd     = open("/tmp/aaa_tmp1.txt", O_RDONLY);
    status = read(fd,sbuf,500);
    close(fd);

//    printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",sbuf[0],sbuf[1],sbuf[2],sbuf[3],sbuf[4],sbuf[5],
//               sbuf[6],sbuf[7],sbuf[8],sbuf[9],sbuf[10],sbuf[11]);

    s1  = strchr( (const char *)sbuf,  (int)' ');
    s2  = strchr( (const char *)(s1+1),(int)' ');
    *s2 = 0;
    i1 = s1+1;

    s1 = strstr( (const char *)(s2+1), (const char *)"HWaddr ");
    strncpy(hwaddr,s1+7,17);
    hwaddr[17] = 0;

    s1  = strstr( (const char *)(s1+1), (const char *)"inet addr:");
    s2  = strchr( (const char *)(s1+8), (int)' ');
    *s2 = 0;
    strcpy(ipaddr,s1+10);

    //sprintf(obuf,"%s\n    %s\n    %s\n", i1,hwaddr,ipaddr);
    sprintf(obuf,"%s\n\r%s\n\r\n",i1,hwaddr);
}

void
dg_echo(int sockfd, struct sockaddr *pcli_addr, int maxclilen)
{
    int     n, clilen;

    while( 1 )
    {
        clilen = maxclilen;
        printf("calling recvfrom\n");
        n = recvfrom(sockfd, sbuf, MAXLEN, 0, pcli_addr, &clilen);
        if (n < 0)
        {
            perror("server: recvfrom error");
            exit(1);
        }
        sbuf[n] = 0;
        //printf("%s\n",sbuf);

        sendto(sockfd, obuf, strlen(obuf), 0, pcli_addr, (socklen_t)clilen);
    }
}


int main()
{
    int                   sockfd;
    struct sockaddr_in    serv_addr, cli_addr;
    int                   opt2;
    int                   optval = 0;
    socklen_t             optlen = 4;
    int                   z      = -1;
    int                   sendbuff;


    get_system_info();

    if(( sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP )) < 0 )
    {
        perror("server: can't open datagram socket");
        exit(1);
    }

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    serv_addr.sin_port        = htons(30303);

    opt2 = 1;
    z = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (const void *)&opt2, (socklen_t)sizeof(opt2));
   

    z = getsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (void *)&optval, (socklen_t *)&optlen);
    printf("optval  = %d\n", optval);
    printf("optlen  = %d\n", optlen);


    if( bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("server: can't bind local address");
        exit(1);
    }


    dg_echo( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) );

    return 1;
}
