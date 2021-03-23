#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
 // + = 
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "locker.h"  //线程同步 
#include "threadpool.h"  //线程池 
#include <signal.h>
#include <http_conn.h>



#define MAX_FD 65535  //最大文件描述符个数 
#define MAX_EVENT_NOMBER 10000 //监听最大事件支持个数 




//信号处理，添加信号捕捉 
void addsig(int sig, void(handler)(int)){
	struct sigaction sa; //注册信号的参数 
	memset(&sa, '\0', sizeof(sa));  //清空 
	sa.sa_handler = handler;
	sigfillset(&sa.sa_mask); //临时阻塞 
	sigaction(sig, &sa, NULL); 
} 

//添加文件描述符到epoll中
extern void addfd(int epollfd, int fd, bool one_shot);

//从epoll中删除文件描述符 
extern void removefd(int epollfd, int fd);

//修改文件描述符
extern void modfd(int epollfd, int fd, int ev); 

int main(int argc, char* argv[]){
	
	if(argc <= 1){ //至少传递一个端口号 
		printf("按照如下格式运行：%s port_number\n", basename(argv[0]));
		exit(-1);
	}
	
	//获取端口号
	int port = atoi(argv[1]);
	
	//对SIGPIE信号进行处理
	addsig(SIGPIPE, SIG_IGN); //当捕捉到一个信号，什么都不做 
	
	//创建线程池
	threadpool<http_conn> * pool = NULL; //任务是http连接 
	try{  //异常捕捉 
		pool = new threadpool<http_conn>;
	} catch(){
		exit(-1);
	}
	
	//创建一个数组用于保存所有的客户端信息
	http_conn * users = new http_conn[MAX_FD];  //一个客户端对应一个文件描述符 
	
	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	
	//绑定前设置端口复用 
	int reuse = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	
	//绑定
	struct sockaddr_in  address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);  //转换为网络字节序 
	bind(listenfd, (struct sockaddr*)&address, sizeof(address));  //bind将socket地址address绑定给listenfd 
	
	//监听
	listen(listenfd, 5);
	
	//创建epoll对象，事件数组，添加文件描述符
    epoll_event events[MAX_EVENT_NOMBER]; //事件数组 
    int epollfd = epoll_create(5);  //创建epoll对象
	
	//将监听的文件描述符添加到epoll对象中
	addfd(epollfd, listenfd, false);
	http_conn::m_epollfd = epollfd; 
	
	while(true){  //主线程检测事件发生
	    int num = epoll_wait(epollfd, events, MAX_EVENT_NOMBER, -1);
    	if((num < 0) && (errno != EINTR)){
	    	printf("epoll failure\n");
		    break;
    	} 
    	
    	//循环遍历事件数组
		for(int i = 0;i < num;i++){
			int sockfd = events[i].data.fd;
			if(sockfd = listenfd){
				//说明有客户端连接 
				struct sockaddr_in client_address;
				socklen_t client_addrlen = sizeof(client_address);
				int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlen); 
				
				if(http_conn::m_user_count >= MAX_FD){
					//说明连接数满
					 close(connfd);
					 continue;
				}
				
				//将新的客户的数据初始化，放到数组中
				users[connfd].init(connfd, client_address); 
			}
			else if(events[i].events & (EPOLLREHUP | EPOLLHUP | EPOLLERR)){
				//对方异常断开或错误等事件,对应头文件中关闭连接函数 
				users[sockfd].close_conn();
				
			}else if(events[i].events & EPOLLIN){
				//判断是否有读事件发生
				 if(users[sockfd].read()){
				 	//一次性把所有数据读完，交给工作线程处理 
					 pool->append(users + sockfd); 
				 } else{
				 	users[sockfd].close_conn();
				 }
			}else if(events[i].events & EPOLLOUT){
				//判断是否有写事件发生
				if( !users[sockfd].write()){
					//一次性写完所有数据 
					users[sockfd].close_conn(); 
					//判断失败才进入这里关闭 
				}
			}
			
		} 
	
	
		
	}
	
	close(epollfd);
	close(listenfd);
	delete [] users;
	delete pool;
	
	return 0;
}
