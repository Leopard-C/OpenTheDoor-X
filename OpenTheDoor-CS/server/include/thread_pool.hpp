#ifndef ThreadPool_h
#define ThreadPool_h

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>

class ThreadPool {
	public:
		ThreadPool(size_t);    //构造函数，size_t n 表示连接数

		template<class F, class... Args>
			auto enqueue(F&& f, Args&&... args)   //任务管道函数
			-> std::future<typename std::result_of<F(Args...)>::type>;  //利用尾置限定符  std future用来获取异步任务的结果

		~ThreadPool();
	private:
		// need to keep track of threads so we can join them
		std::vector< std::thread > workers;   //追踪线程
		// the task queue
		std::queue< std::function<void()> > tasks;    //任务队列，用于存放没有处理的任务。提供缓冲机制

		// synchronization  同步？
		std::mutex queue_mutex;   //互斥锁
		std::condition_variable condition;   //条件变量？
		bool stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads): stop(false)
{
	for(size_t i = 0;i<threads;++i)
		workers.emplace_back(     //以下为构造一个任务，即构造一个线程
				[this]{
					for(;;){
						std::function<void()> task;   //线程中的函数对象
						{
							//大括号作用：临时变量的生存期，即控制lock的时间
							std::unique_lock<std::mutex> lock(this->queue_mutex);
							this->condition.wait(lock,
									[this]{ 
										return this->stop || !this->tasks.empty();  //当stop==false&&tasks.empty(),该线程被阻塞 !this->stop&&this->tasks.empty()
									});
							if(this->stop && this->tasks.empty())
								return;
							task = std::move(this->tasks.front());
							this->tasks.pop();
						}

						task(); //调用函数，运行函数
					}
				}
		);
}

// add new work item to the pool
	template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)  //&& 引用限定符，参数的右值引用，  此处表示参数传入一个函数
	-> std::future<typename std::result_of<F(Args...)>::type>
{
	using return_type = typename std::result_of<F(Args...)>::type;
	//packaged_task是对任务的一个抽象，我们可以给其传递一个函数来完成其构造。之后将任务投递给任何线程去完成，通过
	//packaged_task.get_future()方法获取的future来获取任务完成后的产出值
	auto task = std::make_shared<std::packaged_task<return_type()> >(  //指向F函数的智能指针
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)  //传递函数进行构造
			);
	//future为期望，get_future获取任务完成后的产出值
	std::future<return_type> res = task->get_future();   //获取future对象，如果task的状态不为ready，会阻塞当前调用者
	{
		std::unique_lock<std::mutex> lock(queue_mutex);  //保持互斥性，避免多个线程同时运行一个任务

		// don't allow enqueueing after stopping the pool
		if(stop)
			throw std::runtime_error("enqueue on stopped ThreadPool");

		tasks.emplace([task](){ (*task)(); });  //将task投递给线程去完成，vector尾部压入
	}
	condition.notify_one();  //选择一个wait状态的线程进行唤醒，并使他获得对象上的锁来完成任务(即其他线程无法访问对象)
	return res;
}//notify_one不能保证获得锁的线程真正需要锁，并且因此可能产生死锁

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();  //通知所有wait状态的线程竞争对象的控制权，唤醒所有线程执行
	for(std::thread &worker: workers)
		worker.join(); //因为线程都开始竞争了，所以一定会执行完，join可等待线程执行完
}



#endif /* ThreadPool_h */
