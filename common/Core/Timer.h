#pragma once
#include <chrono>
#include <string>
#include "Log.h"
namespace Core {
	class Timer {
		std::chrono::time_point<std::chrono::high_resolution_clock> _startclock;
		
		std::string _name;
	public:
		Timer(const char*pname) :_name(pname) {
			_startclock = std::chrono::high_resolution_clock::now();
			
		}
		~Timer() {
			auto endclock = std::chrono::high_resolution_clock::now();

			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(_startclock).time_since_epoch().count();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endclock).time_since_epoch().count();
			auto duration = end - start;
			float ms = duration * 0.001f;
			LOG_INFO("Timer {0} {1} ms", _name, ms);
		}
		
	};

}