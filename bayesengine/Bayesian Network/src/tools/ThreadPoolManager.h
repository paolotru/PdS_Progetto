#pragma once

#include "base/thread_pool.hpp"

//static class for managing the thread pool
class ThreadPoolManager : public thread_pool {
public:
	//initializes the thread to the given number of threads. if no number is provided a number equal to max cpu parallelism is used
	static void initialize(unsigned int nThreads = std::thread::hardware_concurrency());

	//returns the instance of the thread pool
	static ThreadPoolManager* getInstance();

	//deletes the instance of the thread pool
	static void deleteInstance();

	static bool isThreadPoolInitialized();

	//returns the number of tasks that has been reserved for this loop
	template <typename T>
	static int reserve_tasks_for_loop(T first_index, T last_index, T nIterationsPerTask) {
		int num_tasks = 0;

		{
			std::lock_guard<std::mutex> lg(ThreadPoolManager::getInstance()->m_submitJobMutex);

			int nThread = ThreadPoolManager::getInstance()->get_thread_count();
			int nTask = ThreadPoolManager::getInstance()->get_tasks_total();

			//if the number of threads available are 2 or more than tries to reserve threads
			if (nThread - nTask > 1) {
				num_tasks = last_index - first_index + 1;
				num_tasks /= nIterationsPerTask;
				if (num_tasks > nThread - nTask) {
					num_tasks = nThread - nTask;
				}

				//if the number of tasks that needs to be submitted is more than 1 then submits them
				if (num_tasks > 1)
					ThreadPoolManager::getInstance()->reserve_tasks(num_tasks);

			}

		}

		//if the number of tasks is less than 2 or there aren't threads available, returns 0 or 1, otherwise the number of reserved tasks is returned
		return num_tasks;
	}

	//voids the task reservation done beforehand and submits the loop with nTasks
	template <typename T, typename F>
	static void parallelizeLoopWithReservedThreads(T first_index, T last_index, const F& loop, int nTasks) {
		{
			std::lock_guard<std::mutex> lg(ThreadPoolManager::getInstance()->m_submitJobMutex);

			ThreadPoolManager::getInstance()->remove_reserved_tasks(nTasks);
		}

		ThreadPoolManager::getInstance()->parallelize_loop(first_index, last_index, loop, nTasks);
	}

	//wrapper function with syncronization used to submit work to the thread onyl if any thread is available.
	//it is used in order to avoid deadlocks due to recursive functions.
	//returns a pair containing :
	//a bool indicating whetere the job has been submitted to thread pool or not
	//the future relative to the job submitted
	template <typename F, typename... A, typename = std::enable_if_t<std::is_void_v<std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>>>
	static std::pair<bool, std::future<bool>> trySubmitJob(const F& task, const A &...args) {
		{
			std::lock_guard<std::mutex> lg(ThreadPoolManager::getInstance()->m_submitJobMutex);

			int nThread = ThreadPoolManager::getInstance()->get_thread_count();
			int nTask = ThreadPoolManager::getInstance()->get_tasks_total();

			if (nThread - nTask > 0) {
				std::future<bool> future = ThreadPoolManager::getInstance()->submit(task, args...);

				std::pair<bool, std::future<bool>> p{ true, std::move(future) };
				return p;
			}
		}

		std::pair<bool, std::future<bool>> p{ false, std::move(std::future<bool>()) };
		return p;
	}

	//wrapper function with syncronization used to submit work to the thread onyl if any thread is available.
	//it is used in order to avoid deadlocks due to recursive functions
	//returns a pair containing :
	//a bool indicating whetere the job has been submitted to thread pool or not
	//the future relative to the job submitted
	template <typename F, typename... A, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>, typename = std::enable_if_t<!std::is_void_v<R>>>
	static std::pair<bool, std::future<R>> trySubmitJob(const F& task, const A &...args) {
		{
			std::lock_guard<std::mutex> lg(ThreadPoolManager::getInstance()->m_submitJobMutex);

			int nThread = ThreadPoolManager::getInstance()->get_thread_count();
			int nTask = ThreadPoolManager::getInstance()->get_tasks_total();

			if (nThread - nTask > 0) {
				std::future<R> future = ThreadPoolManager::getInstance()->submit(task, args...);

				std::pair<bool, std::future<R>> p{ true, std::move(future) };
				return p;
			}
		}

		std::pair<bool, std::future<R>> p{ false, std::move(std::future<R>()) };
		return p;
	}

private:
	ThreadPoolManager(unsigned int nThreads);

	~ThreadPoolManager() {}

	static ThreadPoolManager* m_instance;

	std::mutex m_submitJobMutex;
};