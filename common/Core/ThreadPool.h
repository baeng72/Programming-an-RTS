#pragma once
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <functional>
//https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
namespace Core {
	class ThreadPool {
		void ThreadLoop();
		bool _terminate;
		std::mutex _mutex;
		std::condition_variable _condition;
		std::vector<std::thread> _threads;
		std::queue < std::function<void()>> _jobs;
	public:
		ThreadPool();
		~ThreadPool();
		void QueueJob(const std::function<void()>& job);
		void Stop();
		bool busy();
	};
}