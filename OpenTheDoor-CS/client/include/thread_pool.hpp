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
		ThreadPool(size_t);    //���캯����size_t n ��ʾ������

		template<class F, class... Args>
			auto enqueue(F&& f, Args&&... args)   //����ܵ�����
			-> std::future<typename std::result_of<F(Args...)>::type>;  //����β���޶���  std future������ȡ�첽����Ľ��

		~ThreadPool();
	private:
		// need to keep track of threads so we can join them
		std::vector< std::thread > workers;   //׷���߳�
		// the task queue
		std::queue< std::function<void()> > tasks;    //������У����ڴ��û�д���������ṩ�������

		// synchronization  ͬ����
		std::mutex queue_mutex;   //������
		std::condition_variable condition;   //����������
		bool stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads): stop(false)
{
	for(size_t i = 0;i<threads;++i)
		workers.emplace_back(     //����Ϊ����һ�����񣬼�����һ���߳�
				[this]{
					for(;;){
						std::function<void()> task;   //�߳��еĺ�������
						{
							//���������ã���ʱ�����������ڣ�������lock��ʱ��
							std::unique_lock<std::mutex> lock(this->queue_mutex);
							this->condition.wait(lock,
									[this]{ 
										return this->stop || !this->tasks.empty();  //��stop==false&&tasks.empty(),���̱߳����� !this->stop&&this->tasks.empty()
									});
							if(this->stop && this->tasks.empty())
								return;
							task = std::move(this->tasks.front());
							this->tasks.pop();
						}

						task(); //���ú��������к���
					}
				}
		);
}

// add new work item to the pool
	template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)  //&& �����޶�������������ֵ���ã�  �˴���ʾ��������һ������
	-> std::future<typename std::result_of<F(Args...)>::type>
{
	using return_type = typename std::result_of<F(Args...)>::type;
	//packaged_task�Ƕ������һ���������ǿ��Ը��䴫��һ������������乹�졣֮������Ͷ�ݸ��κ��߳�ȥ��ɣ�ͨ��
	//packaged_task.get_future()������ȡ��future����ȡ������ɺ�Ĳ���ֵ
	auto task = std::make_shared<std::packaged_task<return_type()> >(  //ָ��F����������ָ��
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)  //���ݺ������й���
			);
	//futureΪ������get_future��ȡ������ɺ�Ĳ���ֵ
	std::future<return_type> res = task->get_future();   //��ȡfuture�������task��״̬��Ϊready����������ǰ������
	{
		std::unique_lock<std::mutex> lock(queue_mutex);  //���ֻ����ԣ��������߳�ͬʱ����һ������

		// don't allow enqueueing after stopping the pool
		if(stop)
			throw std::runtime_error("enqueue on stopped ThreadPool");

		tasks.emplace([task](){ (*task)(); });  //��taskͶ�ݸ��߳�ȥ��ɣ�vectorβ��ѹ��
	}
	condition.notify_one();  //ѡ��һ��wait״̬���߳̽��л��ѣ���ʹ����ö����ϵ������������(�������߳��޷����ʶ���)
	return res;
}//notify_one���ܱ�֤��������߳�������Ҫ����������˿��ܲ�������

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();  //֪ͨ����wait״̬���߳̾�������Ŀ���Ȩ�����������߳�ִ��
	for(std::thread &worker: workers)
		worker.join(); //��Ϊ�̶߳���ʼ�����ˣ�����һ����ִ���꣬join�ɵȴ��߳�ִ����
}



#endif /* ThreadPool_h */
