#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <exception>  //异常库，可以抛出异常类 
#include <semophore.h>

class locker{
public:
	locker(){  //构造函数 
		if(pthread_mutex_init(&m_mutex, NULL) != 0){
			throw std::exception();  //如果该函数返回值不等于0则抛出异常（对象） 
		}
		
	}
	
	~locker(){  //析构函数 
		pthread_mutex_destroy(&m_mutex);
		
	}
	
	bool lock(){  //加锁 
		return pthread_mutex_lock(&m_mutex);
	}
	
	bool unlock(){  //解锁 
		return pthread_mutex_unlock(&m_mutex);
	}
	
	pthread_mutex_t * get(){  //定义一个指针 ，用来获取互斥量 
		return &m_mutex;  //返回成员的引用 
	}
	
	
private:
	pthread_mutex_t m_mutex;
	
};


//条件变量类
class cond{
public:
	cond(){  //构造函数 
		if(pthread_cond_init(&m_cond, NULL) != 0){
			throw std::exception();
		}
	}
	
	~cond(){  //析构函数 
		pthread_cond_destroy(&m_cond);
	}
	
	bool wait(pthread_mutex_t * mutex){
		return pthread_cond_wait(&m_cond, mutex) == 0;
	} 
	
	bool timedwait(pthread_mutex_t * mutex, struct timespec t){  //超时时间 
		return pthread_cond_timedwait(&m_cond, mutex, &t) == 0;
	} 
	
	bool signal(){  //增加条件变量 让一个或多个线程唤醒 
		return pthread_cond_wait(&m_cond) == 0;
	} 
	
	bool broadcast(){  //将所有线程唤醒 
		return pthread_cond_broadcast(&m_cond) == 0;
	} 
	
	
private:
	pthread_cond_t m_cond;  //条件变量 
	
}; 


//信号量类
class sem{
public:
	sem(){  //构造函数（无参） 
		if(sem_init(&m_sem, 0, 0) != =){
			throw std::exception();
		}
	}
	
	sem(int num){  //构造函数
		if(sem_init(&m_sem, 0, num) != =){
			throw std::exception();
		}
	}
	
	~sem(){ //析构函数
		sem_destroy(&m_sem);
	}
	
	bool wait(){  //等待信号量 
		return sem_wait(&m_sem) == 0;
	}
	
	bool post(){  //增加信号量 
		return sem_post(&m_sem) == 0;
	}
	
	
private:
	sem_t m_sem;
	
	
}; 




#endif
