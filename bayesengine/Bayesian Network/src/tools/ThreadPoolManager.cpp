#include "ThreadPoolManager.h"

ThreadPoolManager* ThreadPoolManager::m_instance = 0;

ThreadPoolManager::ThreadPoolManager(unsigned int nThreads) : thread_pool(nThreads) {}

void ThreadPoolManager::initialize(unsigned int nThreads) {
	m_instance = new ThreadPoolManager(nThreads);
}

ThreadPoolManager* ThreadPoolManager::getInstance() {
	if (m_instance == 0)
		m_instance = new ThreadPoolManager(std::thread::hardware_concurrency() - 1);

	return m_instance;
}

bool ThreadPoolManager::isThreadPoolInitialized() {
	return ThreadPoolManager::getInstance() != 0;
}

void ThreadPoolManager::deleteInstance() {
	delete m_instance;
}