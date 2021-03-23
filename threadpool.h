#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <list>
#include <exception>
#include <cstdio> 
#include "locker.h"


//线程池类，定义成模板类 为了代码的复用。模板参数T是任务类 
template <typename T>
class threadpool{
public:
	threadpool(int thread_number = 8, int max_requests = 10000);  //构造函数 ，默认线程数量为8 
	~threadpool();    //析构函数 
	
	bool append(T* request);  //添加任务的方法 
	

private:
	static void* worker(void * arg);
	void run();
	
private:
	//线程数量 
	int m_thread_number;
	
	//线程池数组，大小为 m_thread_number
	pthread_t * m_threads;
	
	//请求队列中最多允许的，等待处理的请求数量 
	int m_max_requests;
	
	//请求队列，装的是任务类
	 std::list< T*> m_workqueue;
	 
	 //互斥锁
	 locker m_queuelocker; 
	 
	 //信号量，用来判断是否有任务需要处理
	 sem m_queuestat;
	 
	 //是否结束线程
	 bool m_stop; 
	 
	
};

template <typename T>  //具体实现 
//构造函数 
threadpool<T>::threadpool(int thread_number, int max_requests): //冒号可以初始化 
    m_thread_number(thread_number), m_max_requests(max_requests),
    //45min
    m_stop(false), m_threads(NULL)  {  //线程池数组 
    if((thread_number <= 0) || (max_requests <= 0)){
    	throw std::exception();
	}
	
	m_threads = new pthread_t[m_thread_number];  //new动态创建线程池数组 
	if(!m_threads){
		throw std::exception();
	}
	
	//创建thread_number个线程 并将它们设置为线程脱离 
	for(int i = 0;i < thread_number;i++){
		printf("crating %dth thread");
		
		//第一个参数是pthread_t型指针，用数组名表示 第三个参数是静态函数 
		if(pthread_create(m_threads + i, NULL, worker, NULL) != 0){
			//如果创建出错了，先删除数组再抛出异常 
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
//析构函数 
threadpool<T>::~threadpool(){
	delete [] m_threads;
	m_stop = true;

}

template <typename T> 

bool threadpool<T>::append(T * request){ //添加任务 
	m_queuelocker.lock();
	if(m_workqueue.size() > m_max_requests){
		m_queuelocker.unlock();
		return false;
	}
	
	m_workqueue.push_back(request);
	m_queuelocker.unlock();
	m_queuestat.post();  //工作队列添加请求，信号量增加 
	return true;

}

template <typename T>  
//让worker能访问private成员 
void* threadpool<T>::worker(void * arg){
	threadpool * pool = (threadpool *) arg;
	pool-<run();
	return pool;

}

template <typename T>  
void threadpool<T>::run(){
	while(!m_stop){ //一直工作直到m_stop 
	    //工作过程：从工作队列中取数据
		m_queuestat.wait(); //如果没数据会阻塞在这里，有数据则往下 
		m_queuelocker.lock();  //上锁
		if(m_workqueue.empty()) {
			m_queuelocker.unlock();
			continue;
		}
		
		T* request = m_workqueue.front();  //有数据，获取第一个
		m_workqueue.pop_front(); 
		m_queuelocker.unlock();
		
		if(!request){
			continue;
		}
		
		request->process(); //另一个类 
	}

}

#endif
