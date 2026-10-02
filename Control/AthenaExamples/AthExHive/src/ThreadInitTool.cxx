/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ThreadInitTool.h"
#include "unistd.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

ThreadInitTool::ThreadInitTool( const std::string& type, const std::string& name,
				const IInterface* parent ) 
  : base_class(type, name, parent)
{
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void
ThreadInitTool::initThread()
{
  ATH_MSG_INFO ("initThread in thread {:#x} at {}",
                pthread_self(), static_cast<void*>(this));

  // Thread Local initializations would go here.

  m_nInitThreads++;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void
ThreadInitTool::terminateThread()
{
  ATH_MSG_INFO ("terminateThread in thread {:#x}", pthread_self());

  m_nInitThreads--;

}


