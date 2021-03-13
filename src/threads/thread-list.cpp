#include <map>
#include <thread>

namespace ThreadList
{
	void addThread(std::thread* pthread);
	void removeThread(std::thread* pthread);
	
}