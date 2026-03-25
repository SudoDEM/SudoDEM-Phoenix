/*************************************************************************
*  Copyright (C) 2006 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include <sudodem/lib/base/Logging.hpp>
#include <sudodem/core/ThreadRunner.hpp>
#include <sudodem/core/ThreadWorker.hpp>

#include <thread>
#include <functional>
#include <functional>

#ifdef SUDODEM_OPENMP
#include <omp.h>
#endif

CREATE_LOGGER(ThreadRunner);

void ThreadRunner::run()
{
	// this is the body of execution of separate thread
	// Propagate OpenMP thread count to this worker thread
	#ifdef SUDODEM_OPENMP
	const char* envThreads = getenv("OMP_NUM_THREADS");
	if (envThreads) omp_set_num_threads(atoi(envThreads));
	#endif
	std::lock_guard<std::mutex> lock(m_runmutex);
	try{
		workerThrew=false;
		while(looping()) {
			call();
			if(m_thread_worker->shouldTerminate()){ stop(); return; }
		}
	} catch (std::exception& e){
		LOG_FATAL("Exception occured: "<<std::endl<<e.what());
		workerException=std::exception(e); workerThrew=true;
		stop(); return;
	}
}

void ThreadRunner::call()
{
	// this is the body of execution of separate thread
	// Propagate OpenMP thread count to this worker thread
	#ifdef SUDODEM_OPENMP
	const char* envThreads = getenv("OMP_NUM_THREADS");
	if (envThreads) omp_set_num_threads(atoi(envThreads));
	#endif
	//
	// FIXME - if several threads are blocked here and waiting, and the
	// destructor is called we get a crash. This happens if some other
	// thread is calling spwanSingleAction in a loop (instead of calling
	// start() and stop() as it normally should). This is currently the
	// case of SimulationController with synchronization turned on.
	//
	// the solution is to use a counter (perhaps recursive_mutex?) which
	// will count the number of threads in the queue, and only after they
	// all finish execution the destructor will be able to finish its work
	//
	std::lock_guard<std::mutex> lock(m_callmutex);
	m_thread_worker->setTerminate(false);
	m_thread_worker->callSingleAction();
}

void ThreadRunner::pleaseTerminate()
{
	stop();
	m_thread_worker->setTerminate(true);
}

void ThreadRunner::spawnSingleAction()
{
	std::lock_guard<std::mutex> boollock(m_boolmutex);
	std::lock_guard<std::mutex> calllock(m_callmutex);
	if(m_looping) return;
	std::function<void()> call([this](){ this->call(); });
	std::thread th(call);
	th.detach();
}

void ThreadRunner::start()
{
	std::lock_guard<std::mutex> lock(m_boolmutex);
	if(m_looping) return;
	m_looping=true;
	std::function<void()> run([this](){ this->run(); });
	std::thread th(run);
	th.detach();
}

void ThreadRunner::stop()
{
	//std::cerr<<__FILE__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
	if(!m_looping) return;
	//std::cerr<<__FILE__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
	std::lock_guard<std::mutex> lock(m_boolmutex);
	//std::cerr<<__FILE__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
	m_looping=false;
}

bool ThreadRunner::looping()
{
	std::lock_guard<std::mutex> lock(m_boolmutex);
	return m_looping;
}

ThreadRunner::~ThreadRunner()
{
	pleaseTerminate();
	std::lock_guard<std::mutex> runlock(m_runmutex);
	std::lock_guard<std::mutex> calllock(m_callmutex);
}