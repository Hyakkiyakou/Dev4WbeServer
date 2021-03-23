#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <list>
#include <exception>
#include <cstdio> 
#include "locker.h"


//�̳߳��࣬�����ģ���� Ϊ�˴���ĸ��á�ģ�����T�������� 
template <typename T>
class threadpool{
public:
	threadpool(int thread_number = 8, int max_requests = 10000);  //���캯�� ��Ĭ���߳�����Ϊ8 
	~threadpool();    //�������� 
	
	bool append(T* request);  //�������ķ��� 
	

private:
	static void* worker(void * arg);
	void run();
	
private:
	//�߳����� 
	int m_thread_number;
	
	//�̳߳����飬��СΪ m_thread_number
	pthread_t * m_threads;
	
	//����������������ģ��ȴ�������������� 
	int m_max_requests;
	
	//������У�װ����������
	 std::list< T*> m_workqueue;
	 
	 //������
	 locker m_queuelocker; 
	 
	 //�ź����������ж��Ƿ���������Ҫ����
	 sem m_queuestat;
	 
	 //�Ƿ�����߳�
	 bool m_stop; 
	 
	
};

template <typename T>  //����ʵ�� 
//���캯�� 
threadpool<T>::threadpool(int thread_number, int max_requests): //ð�ſ��Գ�ʼ�� 
    m_thread_number(thread_number), m_max_requests(max_requests),
    //45min
    m_stop(false), m_threads(NULL)  {  //�̳߳����� 
    if((thread_number <= 0) || (max_requests <= 0)){
    	throw std::exception();
	}
	
	m_threads = new pthread_t[m_thread_number];  //new��̬�����̳߳����� 
	if(!m_threads){
		throw std::exception();
	}
	
	//����thread_number���߳� ������������Ϊ�߳����� 
	for(int i = 0;i < thread_number;i++){
		printf("crating %dth thread");
		
		//��һ��������pthread_t��ָ�룬����������ʾ �����������Ǿ�̬���� 
		if(pthread_create(m_threads + i, NULL, worker, NULL) != 0){
			//������������ˣ���ɾ���������׳��쳣 
			delete [] m_threads;
			throw std::exception();
		}
		
		if(pthread_detach(m_thread[i]) ){
			delete [] m_threads;
			throw std::exception();
		}
	}

}


template <typename T>  
//�������� 
threadpool<T>::~threadpool(){
	delete [] m_threads;
	m_stop = true;

}

template <typename T> 

bool threadpool<T>::append(T * request){ //������� 
	m_queuelocker.lock();
	if(m_workqueue.size() > m_max_requests){
		m_queuelocker.unlock();
		return false;
	}
	
	m_workqueue.push_back(request);
	m_queuelocker.unlock();
	m_queuestat.post();  //����������������ź������� 
	return true;

}

template <typename T>  
//��worker�ܷ���private��Ա 
void* threadpool<T>::worker(void * arg){
	threadpool * pool = (threadpool *) arg;
	pool-<run();
	return pool;

}

template <typename T>  
void threadpool<T>::run(){
	while(!m_stop){ //һֱ����ֱ��m_stop 
	    //�������̣��ӹ���������ȡ����
		m_queuestat.wait(); //���û���ݻ���������������������� 
		m_queuelocker.lock();  //����
		if(m_workqueue.empty()) {
			m_queuelocker.unlock();
			continue;
		}
		
		T* request = m_workqueue.front();  //�����ݣ���ȡ��һ��
		m_workqueue.pop_front(); 
		m_queuelocker.unlock();
		
		if(!request){
			continue;
		}
		
		request->process(); //��һ���� 
	}

}

#endif
