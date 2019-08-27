#ifndef ECHO_TASK_ABSTRACTWORKER_HPP
#define ECHO_TASK_ABSTRACTWORKER_HPP

#include <thread>
#include <mutex>
#include <condition_variable>

class AbstractWorker
{

public:
	virtual ~AbstractWorker() { stop(); };

	void start()
	{
		std::lock_guard<std::mutex> locker(_mutex);

		if (_running)
			return;

		_running = true;
		_thread = std::thread(&AbstractWorker::thread_work, this);
	}

	void stop()
	{
		{
			std::lock_guard<std::mutex> locker(_mutex);

			if (!_running)
				return;

			_running = false;
			_event_happend.notify_one();
		}

		_thread.join();
	}

protected:
	virtual void thread_work() = 0;

protected:
	std::thread _thread;
	std::mutex _mutex;
	std::condition_variable _event_happend;
	bool _running{0};

};


#endif //ECHO_TASK_ABSTRACTWORKER_HPP
