/*************************************************************************
*  Copyright (C) 2006 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include "ThreadWorker.hpp"

void ThreadWorker::setTerminate(bool b)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_should_terminate=b;
};

bool ThreadWorker::shouldTerminate()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_should_terminate;
};

void ThreadWorker::setProgress(float i)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_progress=i;
};

void ThreadWorker::setStatus(std::string s)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_status=s;
};

float ThreadWorker::progress()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_progress;
};

std::string ThreadWorker::getStatus()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_status;
};

void ThreadWorker::setReturnValue(std::any a)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_val = a;
};

std::any ThreadWorker::getReturnValue()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_val;
};

bool ThreadWorker::done()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_done;
};

void ThreadWorker::callSingleAction()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_done = false;
	}
	this->singleAction();
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_done = true;
	}
};

