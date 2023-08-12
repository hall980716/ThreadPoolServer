#include <iostream>
#include "CServer.h"
#include "Singleton.h"
#include "LogicSystem.h"
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioThreadPool.h"
using namespace std;
bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main()
{
	try {
		auto pool = AsioThreadPool::GetInstance();
		boost::asio::io_context io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([pool,&io_context](auto, auto) {
			io_context.stop();
			pool->Stop();
			std::unique_lock<std::mutex> lock(mutex_quit);
			bstop = true;
			cond_quit.notify_one();
			});
		CServer s(pool->GetIOService(), 10086);
		{
			std::unique_lock<std::mutex> lock(mutex_quit);
			while (!bstop) {
				cond_quit.wait(lock);
			}
		}
		
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}

}