#include <http_conn.h>

int http_conn::m_epollfd = -1;  
int http_conn::m_user_count = 0; 
	
//�����ļ�������������
void setnonblocking(int fd){
	int old_flag = fcntl(fd, F_GETFL);
	int new_lag = old_flag | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_lag);
}

//�����Ҫ�������ļ���������epoll��
void addfd(int epollfd, int fd, bool one_shot){
//epollfd��ʾepollʵ����fd��fd��ʾҪ��ӵ�epoll��fd 
    //oneshot��֤һ��socket��������һʱ�̶��б�һ���̴߳��� 
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLRDHUP;
	//�����������ù���ģʽ��ˮƽ����/��Ե���� EPOLLLT\EPOLLET
	//event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	
	if(one_shot){
		event.events | EPOLLONESHOT
	}
	
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);   //epoll���ƺ�����EPOLL_CTL_ADD��ʾ��� 
	//�����ļ�������������
	setnonblocking(fd); 
}

//��epoll��ɾ���ļ������� 
void removefd(int epollfd, int fd){
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);  //epoll���ƺ�����EPOLL_CTL_DEL��ʾɾ�� 
	close(fd);
}

//�޸��ļ�������������socket��EPOLLONESHOT�¼� 
void modfd(int epollfd, int fd, int ev){
	epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);  //epoll���ƺ�����EPOLL_CTL_MOD��ʾ�޸� 
}

//��ʼ������ 
void http_conn::init(int sockfd, const sockaddr_int & addr){
	m_sockfd = sockfd;
	m_address = addr;
	
	//�˿ڸ��� 
	int reuse = 1;
	setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	
	//��ӵ�epoll��
	addfd(m_epollfd, m_sockfd, true);
	m_user_count++;
}

//�ر����� 
void http_conn::close_conn();{  
    if(m_sockfd != -1){
    	removefd(m_epollfd, m_sockfd);
    	m_sockfd = -1;
    	m_user_count--;
	}
}

//�������� 
bool http_conn::read(){
	if(m_read_index >= READ_BUFFER_SIZE){
		return false;
	}
	
	//��ȡ�����ֽ�
	int bytes_read = 0;
	while(true){
	 	bytes_read = recv(m_sockfd, m_read_buf + m_read_index, READ_BUFFER_SIZE - m_read_index, 0)
	 	//�ڶ��������������������ʼ��ַ����һ��Ҫ������ʼ��ַ 
	 	if(bytes_read == -1){
	 		if(errno == EAGAIN || errno == EWOULDBLOCK){
	 			//û������
				 break; 
			 }
			 return false;
		 }else if(bytes_read == 0){
		 	//�Է��ر�����
			 return false; 
		 }
	}
	m_read_index += bytes_read;
	printf("��ȡ�������� %s\n",m_read_buf)
	
	return true;
}

//��״̬������������ 
http_conn::HTTP_CODE http_conn::process_read(){
	//����һЩ��ʼ״̬ 
	LINE_STATUS line_status = LINE_OK;
	
}

http_conn::HTTP_CODE http_conn::parse_request_line( char* text ){
	
}
http_conn::HTTP_CODE http_conn::parse_headers( char* text ){

}

http_conn::HTTP_CODE http_conn::parse_content( char* text ){
}
	
    
http_conn::LINE_STATUS http_conn::parse_line(){
	
}


//������д 
bool http_conn::write(){
	printf("һ����д������\n");
	return true;
} 

//���̳߳��еĹ����̵߳��ã�����HTTP�������ں��� 
void http_conn::process(){
	//����HTTP����
	
	HTTP_CODE read_ret = process_read();
	//printf("����HTTP����,������Ӧ\n");

	//������Ӧ 
}
 
