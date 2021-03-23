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
#include "locker.h"  //�߳�ͬ�� 
#include "threadpool.h"  //�̳߳� 
#include <signal.h>
#include <http_conn.h>



#define MAX_FD 65535  //����ļ����������� 
#define MAX_EVENT_NOMBER 10000 //��������¼�֧�ָ��� 




//�źŴ�������źŲ�׽ 
void addsig(int sig, void(handler)(int)){
	struct sigaction sa; //ע���źŵĲ��� 
	memset(&sa, '\0', sizeof(sa));  //��� 
	sa.sa_handler = handler;
	sigfillset(&sa.sa_mask); //��ʱ���� 
	sigaction(sig, &sa, NULL); 
} 

//����ļ���������epoll��
extern void addfd(int epollfd, int fd, bool one_shot);

//��epoll��ɾ���ļ������� 
extern void removefd(int epollfd, int fd);

//�޸��ļ�������
extern void modfd(int epollfd, int fd, int ev); 

int main(int argc, char* argv[]){
	
	if(argc <= 1){ //���ٴ���һ���˿ں� 
		printf("�������¸�ʽ���У�%s port_number\n", basename(argv[0]));
		exit(-1);
	}
	
	//��ȡ�˿ں�
	int port = atoi(argv[1]);
	
	//��SIGPIE�źŽ��д���
	addsig(SIGPIPE, SIG_IGN); //����׽��һ���źţ�ʲô������ 
	
	//�����̳߳�
	threadpool<http_conn> * pool = NULL; //������http���� 
	try{  //�쳣��׽ 
		pool = new threadpool<http_conn>;
	} catch(){
		exit(-1);
	}
	
	//����һ���������ڱ������еĿͻ�����Ϣ
	http_conn * users = new http_conn[MAX_FD];  //һ���ͻ��˶�Ӧһ���ļ������� 
	
	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	
	//��ǰ���ö˿ڸ��� 
	int reuse = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	
	//��
	struct sockaddr_in  address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);  //ת��Ϊ�����ֽ��� 
	bind(listenfd, (struct sockaddr*)&address, sizeof(address));  //bind��socket��ַaddress�󶨸�listenfd 
	
	//����
	listen(listenfd, 5);
	
	//����epoll�����¼����飬����ļ�������
    epoll_event events[MAX_EVENT_NOMBER]; //�¼����� 
    int epollfd = epoll_create(5);  //����epoll����
	
	//���������ļ���������ӵ�epoll������
	addfd(epollfd, listenfd, false);
	http_conn::m_epollfd = epollfd; 
	
	while(true){  //���̼߳���¼�����
	    int num = epoll_wait(epollfd, events, MAX_EVENT_NOMBER, -1);
    	if((num < 0) && (errno != EINTR)){
	    	printf("epoll failure\n");
		    break;
    	} 
    	
    	//ѭ�������¼�����
		for(int i = 0;i < num;i++){
			int sockfd = events[i].data.fd;
			if(sockfd = listenfd){
				//˵���пͻ������� 
				struct sockaddr_in client_address;
				socklen_t client_addrlen = sizeof(client_address);
				int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlen); 
				
				if(http_conn::m_user_count >= MAX_FD){
					//˵����������
					 close(connfd);
					 continue;
				}
				
				//���µĿͻ������ݳ�ʼ�����ŵ�������
				users[connfd].init(connfd, client_address); 
			}
			else if(events[i].events & (EPOLLREHUP | EPOLLHUP | EPOLLERR)){
				//�Է��쳣�Ͽ��������¼�,��Ӧͷ�ļ��йر����Ӻ��� 
				users[sockfd].close_conn();
				
			}else if(events[i].events & EPOLLIN){
				//�ж��Ƿ��ж��¼�����
				 if(users[sockfd].read()){
				 	//һ���԰��������ݶ��꣬���������̴߳��� 
					 pool->append(users + sockfd); 
				 } else{
				 	users[sockfd].close_conn();
				 }
			}else if(events[i].events & EPOLLOUT){
				//�ж��Ƿ���д�¼�����
				if( !users[sockfd].write()){
					//һ����д���������� 
					users[sockfd].close_conn(); 
					//�ж�ʧ�ܲŽ�������ر� 
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
