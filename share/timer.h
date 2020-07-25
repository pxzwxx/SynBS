#ifndef _TIMER_H_
#define _TIMER_H_

#include<time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<iostream>
#include<functional>
using namespace std;

template<typename T>
class util_timer {
public:
	util_timer() : prev(NULL), next(NULL) {
		//Log::INFO("%s\n", "util_timer()");
	}
	~util_timer() {
		prev = NULL;
		next = NULL;
		//Log::INFO("%s\n", "~util_timer()");
	}
public:
	time_t expire;
	std::function<void(T* user_datar)> cb_func;
	util_timer *prev;
	util_timer *next;
	T *client;
};


//按照升序排列的循环链表
template<typename T>
class sort_timer_lst {
public:
	sort_timer_lst() {
		//Log::INFO("%s\n", "sort_timer_lst()");
		head = new util_timer<T>;
		tail = new util_timer<T>;
		head->next = tail;
		head->prev = NULL;
		tail->prev = head;
		tail->next = NULL;
	}
	~sort_timer_lst() {
		//Log::INFO("%s\n", "~sort_timer_lst()");
		auto tmp = head;
		while(tmp) {
			auto node = tmp->next;
			delete tmp;
			tmp = node;
		}
	}
public:
	//向链表中添加节点
	void add_timer(util_timer<T> *timer) {
		//Log::INFO("%s\n", "add_timer()");
		if(head->next == tail) { //链表为空
			timer->next = tail;
			timer->prev = head;
			head->next = timer;
			tail->prev = timer;
			return;
		}		
		auto tmp = head->next;		
		while(tmp != tail) {
			if(tmp->expire < timer->expire)	tmp = tmp->next;
			else break;
		}
		timer->next = tmp;
		timer->prev = tmp->prev;
		tmp->prev->next = timer;
		tmp->prev = timer;			
	}
	//调整某个节点到合适的位置
	void adjust_timer(util_timer<T> *timer) {
		//Log::INFO("%s\n", "adjust_timer()");
		if(timer == tail->prev) return; //最后一个节点
		auto tmp = head->next;
		while(tmp != tail) {
			if(tmp->expire <= timer->expire) tmp = tmp->next; 
			else break;	
		}
		//改变当前指向
		timer->prev->next = timer->next;
		timer->next->prev = timer->prev;
		
		//调整位置
		timer->prev = tmp->prev;
		timer->next = tmp;
		tmp->prev->next = timer;
		tmp->prev = timer;
	}
	//删除当前时间节点
	void del_timer(util_timer<T> *timer) {
		//Log::INFO("%s\n", "del_timer()");
		if(head->next == tail) return;
		timer->prev->next = timer->next;
		timer->next->prev = timer->prev;
		//delete timer;
	}
	//处理定时任务	
	void tick() {
		//Log::INFO("%s\n", "tick()");
		if(head->next == tail) return;
		auto tmp = head->next;
		time_t cur = time(NULL);
		while(tmp != tail) {
			if(cur >= tmp->expire) { //
				tmp->prev->next = tmp->next;
				tmp->next->prev = tmp->prev;
				tmp->cb_func(tmp->client);
				if(tmp) delete tmp;
				tmp = head->next;
			}
			else break;
		}
	}
private:
	util_timer<T> *head;
	util_timer<T>  *tail;
};

#endif
