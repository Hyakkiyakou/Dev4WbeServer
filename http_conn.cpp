#include <http_conn.h>

int http_conn::m_epollfd = -1;  
int http_conn::m_user_count = 0; 
	
//设置文件描述符非阻塞
void setnonblocking(int fd){
	int old_flag = fcntl(fd, F_GETFL);
	int new_lag = old_flag | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_lag);
}

//添加需要监听的文件描述符到epoll中
void addfd(int epollfd, int fd, bool one_shot){
//epollfd表示epoll实例的fd，fd表示要添加到epoll的fd 
    //oneshot保证一个socket连接在任一时刻都中被一个线程处理 
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLRDHUP;
	//可以在这设置工作模式：水平触发/边缘触发 EPOLLLT\EPOLLET
	//event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	
	if(one_shot){
		event.events | EPOLLONESHOT
	}
	
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);   //epoll控制函数，EPOLL_CTL_ADD表示添加 
	//设置文件描述符非阻塞
	setnonblocking(fd); 
}

//从epoll中删除文件描述符 
void removefd(int epollfd, int fd){
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);  //epoll控制函数，EPOLL_CTL_DEL表示删除 
	close(fd);
}

//修改文件描述符，重置socket上EPOLLONESHOT事件 
void modfd(int epollfd, int fd, int ev){
	epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);  //epoll控制函数，EPOLL_CTL_MOD表示修改 
}

//初始化连接 
void http_conn::init(int sockfd, const sockaddr_int & addr){
	m_sockfd = sockfd;
	m_address = addr;
	
	//端口复用 
	int reuse = 1;
	setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	
	//添加到epoll中
	addfd(m_epollfd, m_sockfd, true);
	m_user_count++;
}

//关闭连接 
void http_conn::close_conn();{  
    if(m_sockfd != -1){
    	removefd(m_epollfd, m_sockfd);
    	m_sockfd = -1;
    	m_user_count--;
	}
}

//非阻塞读 
bool http_conn::read(){
	if(m_read_index >= READ_BUFFER_SIZE){
		return false;
	}
	
	//读取到的字节
	int bytes_read = 0;
	while(true){
	 	bytes_read = recv(m_sockfd, m_read_buf + m_read_index, READ_BUFFER_SIZE - m_read_index, 0)
	 	//第二个参数意义就是数组起始地址加下一次要读的起始地址 
	 	if(bytes_read == -1){
	 		if(errno == EAGAIN || errno == EWOULDBLOCK){
	 			//没有数据
				 break; 
			 }
			 return false;
		 }else if(bytes_read == 0){
		 	//对方关闭连接
			 return false; 
		 }
	}
	m_read_index += bytes_read;
	printf("读取到了数据 %s\n",m_read_buf)
	
	return true;
}

//主状态机，解析请求 
http_conn::HTTP_CODE http_conn::process_read(){
	//定义一些初始状态 
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


//非阻塞写 
bool http_conn::write(){
	printf("一次性写完数据\n");
	return true;
} 

//由线程池中的工作线程调用，处理HTTP请求的入口函数 
void http_conn::process(){
	//解析HTTP请求
	
	HTTP_CODE read_ret = process_read();
	//printf("解析HTTP请求,生成响应\n");

	//生成响应 
}
 
