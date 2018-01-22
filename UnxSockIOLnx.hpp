#ifndef UNXSOCKIOLNX_H
#define UNXSOCKIOLNX_H

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <unistd.h>

#include "SVTypesLnx.h"

//#define SOCKET int
//#define SOCKET_ERROR -1


struct SV_SockAddr
{
	__uint32 ip;			//в сетевой 
	__uint16 port;			//в сетевой

	//заполняется из sockaddr
	inline void FromSockAddr(sockaddr& addr){
		sockaddr_in& scaddr=(sockaddr_in&)addr;
		//ip=scaddr.sin_addr.S_un.S_addr;
		ip=scaddr.sin_addr.s_addr;
		port=scaddr.sin_port;
	}
	//заполн. порт из хост послед. _port
	inline void Port(u_int16_t _port){port=htons(_port);}
	
	//заполн. ip из строки
	inline void IPS(const char* _ip){ip=inet_addr(_ip);}
	
	//возвр. ip как строку
	const char* IPS(){
		static char sip[16];
		in_addr addr;
		//addr.S_un.S_addr=ip;
		addr.s_addr=ip;
		char* s=inet_ntoa(addr);
		strcpy(sip,s);
		return sip;
	}
		
	//выводит в sockaddr
	inline void ToSockAddr(sockaddr& iaddr){
		sockaddr_in saddr;
		in_addr addr;
		//addr.S_un.S_addr=inet_addr(sip);
		//addr.S_un.S_addr=ip;
		addr.s_addr=ip;
		saddr.sin_family=AF_INET;
		saddr.sin_port=port;//htons(port);
		saddr.sin_addr=addr;
		memcpy(&iaddr,&saddr,sizeof(sockaddr_in));
	}

	//возвр. ip в хост послед.
	inline __uint32 IP() {return ntohl(ip);}
	//возвр. порт в хост послед.
	inline __uint16 Port() {return ntohs(port);}
		
};



class UnxSockIo2 
{
	fd_set fdset;
	timeval ttimeout;
	
	SV_SOCKET sock;
	bool sock_created;
	bool sock_connected;
	sockaddr _sockaddr;
	
	int blocksz;
	int blocksleep;	
public:	
        
	UnxSockIo2()
	{
		sock=0;
		sock_connected=false;
		sock_created=false;
		ttimeout.tv_sec=0;
		ttimeout.tv_usec=0;
		blocksz=1024*4;
		blocksleep=0;
	}
        
    static int SocketStartup(){return 1;}
        
	
    SV_SOCKET GetSocket(){return sock;}

	//этот адрес должен быть создан перед вызовом Connect()
	void CreateTCPAddress(char* ip,int port)
	{
		sockaddr_in saddr;
		in_addr addr;
		//addr.S_un.S_addr=inet_addr(ip);
		addr.s_addr=inet_addr(ip);
		saddr.sin_family=PF_INET;
		saddr.sin_port=htons(port);
		saddr.sin_addr=addr;
		memcpy(&_sockaddr,&saddr,sizeof(sockaddr_in));
	}
	
	void SetTimeoutSeconds(int timeout){ttimeout.tv_sec=timeout;ttimeout.tv_usec=0;}

    int SetTimeoutMilliseconds(int timeout){
            ttimeout.tv_sec=0;
            ttimeout.tv_usec=timeout*1000L;   //ò.ê. TIMEVAL õðàíèò çíà÷. â ìèêðîñåêóíäàõ
            return ttimeout.tv_usec;
        }

	//позволяет задать задержку после посылки блока в sendbl
	void SetBlockSleep(int ms){blocksleep=ms;}
	void SetBlockSize(int sz){blocksz=sz;}
	
	//необяз. фун. можно сразу вызыв. Connect()
	int CreateTCPStreamSocket() 
	{
		//сокет уже создан
        if(sock_created==true) return 0;
        sock=socket(AF_INET,SOCK_STREAM,0);
		if(sock==0) {sock_created=false;return 1;}
		else {sock_created=true;return 0;}		
	}
	
	int CreateTCPUDPSocket() {
		//сокет уже создан
        if(sock_created==true) return 0;
        sock=socket(AF_INET,SOCK_DGRAM,0);
		if(sock==0) {sock_created=false;return 1;}
		else {sock_created=true;return 0;}		
	}
        
        //создает сокет сервера
	//предварительно обязательно вызвать CreateTCPAddress
	int CreateServerSocket()
	{
		int r;
		sock_connected=false;
		CreateTCPStreamSocket();
		if(sock_created==true){
			r = bind(sock,&_sockaddr,sizeof(sockaddr_in));
			if(r!=0) {return 1;}
			r=listen(sock,5);
			if(r!=0) {return 2;}
		}
		else{
			return 3;
		}
		sock_connected=true;
		return 0;
	}

	void SetNonBlocking()
	{
		fcntl(sock, F_SETFL, O_NONBLOCK);
		//printf("nonblocking: %x\n",(int)sock);
	}
        
    //инициализ. данного объкта на сокет получ. от accept
	//устанав. сокет класса на получ. accept
	//ret: 0=OK,1=timeout,2=INVALID_SOCKET
	int Accept(SV_SOCKET srvsock,int timeout_insec)
	{
		int r,rv=0;
		timeval tm;
		tm.tv_sec = timeout_insec;
		tm.tv_usec = 0;
		FD_ZERO(&fdset);
		//FD_SET(sock,&fdset);				//было ошибочно
		FD_SET(srvsock,&fdset);
			r=select(srvsock+1,&fdset,0,0,&tm);
			//if((r==0)||(FD_ISSET(sock,&fdset)==false)) {rv=1;}		//было ошибочно
			if((r==0)||(FD_ISSET(srvsock,&fdset)==false)) {rv=1;}
			else{
				sock=accept(srvsock,NULL,NULL);
				if(sock>=0){
					sock_created=true;
					sock_connected=true;
					rv=0;
					//OK();
				}
				else{
					rv=2;
					//Error();
				}
			}
		return rv;

	}
        
        //принимает timeout в мс
        int Accept2(SV_SOCKET srvsock,int timeout_ms)
	{
		int r,rv=0;
		timeval tm;
		tm.tv_sec = 0;
		tm.tv_usec = timeout_ms*1000L;
		FD_ZERO(&fdset);
		//FD_SET(sock,&fdset);				//было ошибочно
		FD_SET(srvsock,&fdset);
			r=select(srvsock+1,&fdset,0,0,&tm);
			//if((r==0)||(FD_ISSET(sock,&fdset)==false)) {rv=1;}		//было ошибочно
			if((r==0)||(FD_ISSET(srvsock,&fdset)==false)) {rv=1;}
			else{
				sock=accept(srvsock,NULL,NULL);
				if(sock>=0){
					sock_created=true;
					sock_connected=true;
					rv=0;
					//OK();
				}
				else{
					rv=2;
					//Error();
				}
			}
		return rv;

	}


        
	
	//ret: 0=сокет соединен 1=ошибка connect 2=ошибка создания сокета
	//	   -1=сокет уже соединенный
	//если сокет не создан то созд. и соед.
	//иначе только соед.
	int Connect()
	{
		int r;
		if(sock_created==false) {if(CreateTCPStreamSocket()!=0) return 2;}
		if(sock_created==true){
			if(sock_connected==false){
				r=connect(sock,&_sockaddr,sizeof(sockaddr));
				if(r!=0) return 1;
				else {sock_connected=true;return 0;}
			}
		}
		return -1;
	}
        
    //для неблокирующего сокета
    int ConnectNB()
    {
        int r;
        if(sock_created==false) {if(CreateTCPStreamSocket()!=0) return 2;}
        if(sock_created==true){
            if(sock_connected==false){
                r=connect(sock,&_sockaddr,sizeof(sockaddr));
                if(r!=0) {
                    fd_set wr;
                    FD_ZERO(&wr);
                    FD_SET(sock, &wr);
                    timeval tout;
                    tout.tv_sec = 0;
                    tout.tv_usec = 0L;
                    select(sock+1,0,&wr,0,&tout);
                    if(FD_ISSET(sock, &wr))
                    {
                        sock_connected=true;
                        return 0;
                    }
                    return 1;
                }
                else {sock_connected=true;return 0;}
            }
        }
        return -1;
    }


        //не проверена, не возвращ. сокет в блокирующий!!!
        //connect с таймаутом
	int ConnectWT(int timeout_ms)
	{
		int r;
		
		printf("ConnectWT !!! tout=%d\n",timeout_ms);
		
		if(sock_created==false) {if(CreateTCPStreamSocket()!=0) return 2;}
		if(sock_created==true){
			if(sock_connected==false){
				//set the socket in non-blocking
                                //unsigned long iMode = 1;
                                //int iResult = ioctlsocket(sock, FIONBIO, &iMode);

                                int ret=1;

                                fcntl(sock, F_SETFL, O_NONBLOCK);

                                r=connect(sock,&_sockaddr,sizeof(sockaddr));

                                timeval _Timeout;
				_Timeout.tv_sec = 0;
				_Timeout.tv_usec = timeout_ms*1000L;

                                FD_ZERO(&fdset);
                                FD_SET(sock, &fdset);
                                if (select(sock + 1, NULL, &fdset, NULL, &_Timeout) == 1)
                                {
                                    int so_error;
                                    socklen_t len = sizeof so_error;
                                    getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
                                    if (so_error == 0) {
                                           sock_connected=true;
                                           ret=0;
                                    }
                                }

                                //fcntl(sock, F_SETFL, O_BLOCK);

                                return ret;
			}
		}
		return -1;
	}
        
    int Disconnect2()
	{
		//shutdown(sock,SD_BOTH);
		//closesocket(sock);
                shutdown(sock,SHUT_RDWR);
		close(sock);
                sock_connected=false;
		sock_created=false;
		return 0;
	}
	
        //назнач. объекту уже созд. сокет, счит., что сокет создан и соединен
	void SetConnectedSocket(SV_SOCKET _sock){
		if(_sock!=-1){
			sock_connected=true;
			sock_created=true;
			sock=_sock;
		}
	}
		
	//ret:0=OK,1=заверш. соед,-1=SOCKET ERROR   rec=число запис. в буф. дан.
	//2=timeout
	int recvbl(void* buf,int bufsz,int& rec)
	{
		SV_SOCKET clsock=sock;
		int buflen=bufsz;
		int idx = 0;
		int rv=0;
		int r;
		FD_ZERO(&fdset);
		while (buflen > 0)
		{
			FD_SET(clsock,&fdset);
					timeval tt=ttimeout;
			r=select(clsock+1,&fdset,0,0,&tt);
			if((r==0)||(FD_ISSET(clsock,&fdset)==false)) {rv=2;break;}
			r = recv(clsock, (char*)buf+idx, buflen, 0);
			if (r == SV_SOCKET_ERROR){
				rv=-1;break;
			}
			if(r==0) {rv=1;break;}
			
			#ifdef _MYDEBUG
			printf("recv_block=%d\n",r);
			#endif
			
			buflen-= r;
			idx += r;
		}
		rec=idx;
		return rv;
	}
	
	int sendbl(void* buf,int bufsz,int& sending)
	{
		int sblocksz;
		if(bufsz<blocksz) sblocksz=bufsz;
		else sblocksz=blocksz;
		
		SV_SOCKET clsock=sock;
		int buflen=bufsz;
		int idx = 0;
		int rv=0;
		int r,rs;
		FD_ZERO(&fdset);
		while (buflen > 0)
		{
			FD_SET(clsock,&fdset);
					timeval tt=ttimeout;
			rs=select(clsock+1,0,&fdset,0,&tt);
			if((rs==0)||(FD_ISSET(clsock,&fdset)==false)) {rv=2;break;}
			if(buflen<sblocksz) sblocksz=buflen;
			r = send(clsock, (char*)buf+idx, sblocksz, 0);
			if (r == SV_SOCKET_ERROR){
				rv=-1;break;
			}
			if(r==0) {rv=1;break;}
			
			#ifdef _MYDEBUG
			//printf("send_block=%d\n",r);
			#endif
			
			if(blocksleep!=0) usleep(blocksleep*1000);
				
			buflen-= r;
			idx += r;
		}
		sending=idx;
		return rv;

	}
	
	
	//посылает датаграмму по адресу в объекте
	//ret: 1-если ошибка rec=кол-во обработан. байт
	int sendudp(void* data,int datasize,int& rec)
	{
		int r;
		r=sendto(sock,(char*)data,datasize,0,&_sockaddr,sizeof(_sockaddr));
		if(r==SV_SOCKET_ERROR){
			rec=0;
			return 1;
		}
		else{
			rec=r;
			return 0;
		}
	}
    int sendudp(void* data,int datasize,SV_SockAddr addr,int& rec)
	{
		int r;
		sockaddr sockaddr;
		addr.ToSockAddr(sockaddr);
		r=sendto(sock,(char*)data,datasize,0,&sockaddr,sizeof(sockaddr));
		if(r==SV_SOCKET_ERROR){
			rec=0;
			return 1;
		}
		else{
			rec=r;
			return 0;
		}
	}


    int UDPBind()
	{
		int r ;
		if(sock_created==true){
			r=bind(sock,&_sockaddr,sizeof(sockaddr_in));
			if(r!=0) {return 1;}
		}
		else{
			return 2;
		}
		return 0;
	}

    int recvudp(void *data, int datasize, SV_SockAddr &fromhost, int &rec)
	{
		int r;
		FD_ZERO(&fdset);
		FD_SET(sock,&fdset);
		timeval tt=ttimeout;
		r=select(sock+1,&fdset,0,0,&tt);    //linux изменяет tt
		if((r==0)||(FD_ISSET(sock,&fdset)==false)) {rec=0;return -1;}
		sockaddr from;
		//int fromlen=sizeof(from);
		socklen_t fromlen=sizeof(from);
		r=recvfrom(sock,(char*)data,datasize,0,&from,&fromlen);
		if(r==SV_SOCKET_ERROR){
			rec=0;
			return 1;
		}
		else{
			rec=r;
			fromhost.FromSockAddr(from);
			return 0;
		}
	}
    int IsRecvPack(int timeout_ms)
	{
		int r;
		timeval _ttimeout;
		fd_set _fdset;
		_ttimeout.tv_sec=0;
		_ttimeout.tv_usec=timeout_ms*1000L;
		FD_ZERO(&_fdset);
		FD_SET(sock,&_fdset);
		r=select(sock+1,&_fdset,0,0,&_ttimeout);
		if((r==0)||(FD_ISSET(sock,&_fdset)==false)) {return 1;}
		else return 0;

	}
};

class SVEthUDP_Base
{
protected:
	int r;

public:
	UnxSockIo2 lsock;
	SV_SockAddr from;


	int Open(const char* ip,int port){
		r=lsock.CreateTCPUDPSocket();
		if(r!=0) return 1;
		lsock.CreateTCPAddress((char*)ip,port);
		//r=lsock.SetBroadcastOn();
		if(r!=0) return 2;
		return 0;
	}

	int OpenSrv(const char* ip,int port){
		r=Open(ip,port);
		if(r!=0) return 1;
		r=lsock.UDPBind();
		if(r!=0) return 2;
		return 0;
	}

    //void Close(){
        //lsock.Disconnect();
    //}
};


#endif // UNXSOCKIOLNX_H
