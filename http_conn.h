#ifndef HTTPCONNETION_H 
#define HTTPCONNETION_H

#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/mman.h> //�ڴ�ӳ�� 
#include <stdarg.h>
#include <errno.h>
#include "locker.h" 
#include <sys/uio.h>

class http_conn{  //������ 
public:
	
	static int m_epollfd;  //���е�socket�ϵ��¼�����ע�ᵽͬһ��epoll������ 
	static int m_user_count;  //�û����� 
	static const int READ_BUFFER_SIZE = 2048;  //���������Ĵ�С 
	static const int WRITE_BUFFER_SIZE = 2048;  //д�������Ĵ�С 
	
	// HTTP���󷽷�������ֻ֧��GET
    enum METHOD {GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};
    
    /*
        �����ͻ�������ʱ����״̬����״̬
        CHECK_STATE_REQUESTLINE:��ǰ���ڷ���������
        CHECK_STATE_HEADER:��ǰ���ڷ���ͷ���ֶ�
        CHECK_STATE_CONTENT:��ǰ���ڽ���������
    */
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };
    
    /*
        ����������HTTP����Ŀ��ܽ�������Ľ����Ľ��
        NO_REQUEST          :   ������������Ҫ������ȡ�ͻ�����
        GET_REQUEST         :   ��ʾ�����һ����ɵĿͻ�����
        BAD_REQUEST         :   ��ʾ�ͻ������﷨����
        NO_RESOURCE         :   ��ʾ������û����Դ
        FORBIDDEN_REQUEST   :   ��ʾ�ͻ�����Դû���㹻�ķ���Ȩ��
        FILE_REQUEST        :   �ļ�����,��ȡ�ļ��ɹ�
        INTERNAL_ERROR      :   ��ʾ�������ڲ�����
        CLOSED_CONNECTION   :   ��ʾ�ͻ����Ѿ��ر�������
    */
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    
    // ��״̬�������ֿ���״̬�����еĶ�ȡ״̬���ֱ��ʾ
    // 1.��ȡ��һ���������� 2.�г��� 3.���������Ҳ�����
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };
	
	http_conn(){};
	~http_conn(){};
	
	void process(); //����ͻ������� 
	void init(int sockfd, const sockaddr_int & addr);  //��ʼ���½��ܵ����� 
	void close_conn();  //�ر����� 
	
	bool read();  //�������� 
	bool write();  //������д 
	
private:
	
	HTTP_CODE process_read();    // ����HTTP����
	// ������һ�麯����process_read�����Է���HTTP����
    HTTP_CODE parse_request_line( char* text );  //������������ 
    HTTP_CODE parse_headers( char* text );  //��������ͷ 
    HTTP_CODE parse_content( char* text );  //������������ 
    
    LINE_STATUS parse_line();  //��״̬������1�� 
	
	int m_sockfd;  //��HTTP���ӵ�socket 
	sockaddr_in m_address;  //ͨ�ŵ�socket��ַ 
	
	char m_read_buf[READ_BUFFER_SIZE];  //��������
	int m_read_index;  //��ʶ������������ʼ��λ�� 
	
	
};

#endif
