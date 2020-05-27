#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <string>
#include <thread>
#include <iomanip>


template <size_t N, class T>
class ThreadController {
	/* This class is using for parallel calculations */

	std::mutex mutex1, mutex2;
	std::condition_variable done;
	
	std::vector<T> algVector;
	std::deque<std::string> PATHS;
	bool Finished[N];

	//Make calculations in thread that got as parametr of function
	void threadTask(size_t id){

		int count = 0;
		while (true) {
			std::unique_lock<std::mutex> lock(mutex1);
			if (algVector.empty()) {
				std::unique_lock<std::mutex> lock(mutex2);
				Finished[id] = true;
				done.notify_all();
				lock.unlock();
				return;
			}

			auto algorithm = algVector.back();
			algVector.pop_back();

			std::string fileName = PATHS.back();
			PATHS.pop_back();
			 
			lock.unlock();
			algorithm->start(fileName);

			// Method "start" starts calculations of i-th element of deque
			
			// change to iomanip
			std::cout << std::endl << "|" << "\t\t" << std::this_thread::get_id() << "\t\t" << "|" \
			<< "\t\t" << fileName << "\t\t" << "|" << std::endl;
		}
	}
	void waitFinished() 
	{ 
	std::unique_lock<std::mutex> lock(mutex2); 
		done.wait(lock, [this] { 
			return std::all_of(Finished, Finished + N, [](bool e) { return e; }); 
	}); 
	}

public:
	//Add object in queue
	ThreadController& push(T algorithm, const std::string path) {
		algVector.push_back(algorithm);
		PATHS.push_back(path);
		return *this;
	}

	

	//Implementation of the algorithm
	void start() {

		for (bool& i : Finished)
			i = false;

		for (size_t i = 0; i < N; ++i) {
			std::thread worker([this, i]() {
				threadTask(i); 
			});
			worker.detach();
		}

		this->waitFinished();
	}
};

