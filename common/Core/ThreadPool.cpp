#include "ThreadPool.h"
#include "Log.h"
#include "Timer.h"
namespace Core {
	
	ThreadPool::ThreadPool(uint32_t threads)
		:_terminate(false)
	{
		LOG_INFO("Starting ThreadPool");
		uint32_t numThreads = std::thread::hardware_concurrency();//max number of supported threads
		if (threads > 0 && threads < numThreads)
			numThreads = threads;
		for (uint32_t i = 0; i < numThreads; i++) {
			_threads.emplace_back(std::thread(&ThreadPool::ThreadLoop, this));
		}
	}
	ThreadPool::~ThreadPool()
	{
	}
	void ThreadPool::ThreadLoop()
	{
		while (true) {
			//look for job
			std::function<void()> job;
			{
				std::unique_lock<std::mutex> lock(_mutex);//spinlock here?
				_condition.wait(lock, [this] {
					return !_jobs.empty() || _terminate;
					});
				if (_terminate)
					return;
				job = _jobs.front();
				_jobs.pop();
			}
			//LOG_INFO("Thread {0} starting job", std::this_thread::get_id());
			//char buffer[128];
			//sprintf_s(buffer, "Thread %lu",(int) std::hash<std::thread::id>{}(std::this_thread::get_id()));
			//{
			//	Timer t(buffer);
				job();
			//}
		}
	}
	void ThreadPool::QueueJob(const std::function<void()>& job)
	{
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_jobs.push(job);
		}
		_condition.notify_one();
	}
	void ThreadPool::Stop()
	{
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_terminate = true;
		}
		_condition.notify_all();
		for (std::thread& thread : _threads) {
			thread.join();
		}
		_threads.clear();
	}
	bool ThreadPool::busy()
	{
		bool poolBusy;
		{
			std::unique_lock<std::mutex> lock(_mutex);
			poolBusy = !_jobs.empty();
		}
		return poolBusy;
	}
}