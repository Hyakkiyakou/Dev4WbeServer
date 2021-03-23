#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <exception>  //�쳣�⣬�����׳��쳣�� 
#include <semophore.h>

class locker{
public:
	locker(){  //���캯�� 
		if(pthread_mutex_init(&m_mutex, NULL) != 0){
			throw std::exception();  //����ú�������ֵ������0���׳��쳣������ 
		}
		
	}
	
	~locker(){  //�������� 
		pthread_mutex_destroy(&m_mutex);
		
	}
	
	bool lock(){  //���� 
		return pthread_mutex_lock(&m_mutex);
	}
	
	bool unlock(){  //���� 
		return pthread_mutex_unlock(&m_mutex);
	}
	
	pthread_mutex_t * get(){  //����һ��ָ�� ��������ȡ������ 
		return &m_mutex;  //���س�Ա������ 
	}
	
	
private:
	pthread_mutex_t m_mutex;
	
};


//����������
class cond{
public:
	cond(){  //���캯�� 
		if(pthread_cond_init(&m_cond, NULL) != 0){
			throw std::exception();
		}
	}
	
	~cond(){  //�������� 
		pthread_cond_destroy(&m_cond);
	}
	
	bool wait(pthread_mutex_t * mutex){
		return pthread_cond_wait(&m_cond, mutex) == 0;
	} 
	
	bool timedwait(pthread_mutex_t * mutex, struct timespec t){  //��ʱʱ�� 
		return pthread_cond_timedwait(&m_cond, mutex, &t) == 0;
	} 
	
	bool signal(){  //������������ ��һ�������̻߳��� 
		return pthread_cond_wait(&m_cond) == 0;
	} 
	
	bool broadcast(){  //�������̻߳��� 
		return pthread_cond_broadcast(&m_cond) == 0;
	} 
	
	
private:
	pthread_cond_t m_cond;  //�������� 
	
}; 


//�ź�����
class sem{
public:
	sem(){  //���캯�����޲Σ� 
		if(sem_init(&m_sem, 0, 0) != =){
			throw std::exception();
		}
	}
	
	sem(int num){  //���캯��
		if(sem_init(&m_sem, 0, num) != =){
			throw std::exception();
		}
	}
	
	~sem(){ //��������
		sem_destroy(&m_sem);
	}
	
	bool wait(){  //�ȴ��ź��� 
		return sem_wait(&m_sem) == 0;
	}
	
	bool post(){  //�����ź��� 
		return sem_post(&m_sem) == 0;
	}
	
	
private:
	sem_t m_sem;
	
	
}; 




#endif
