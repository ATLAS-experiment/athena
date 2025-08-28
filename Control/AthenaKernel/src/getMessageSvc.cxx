/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ServiceHandle.h"
#include "CxxUtils/checker_macros.h"

#include "AthenaKernel/getMessageSvc.h"

using namespace Athena;

/// Set this to force off the warning messages from getMessageSvc
/// (in unit tests, for example).
std::atomic<bool> Athena::getMessageSvcQuiet;

IMessageSvc* Athena::getMessageSvc( bool quiet ) { return getMessageSvc( Options::Lazy, quiet ); }
IMessageSvc* Athena::getMessageSvc( const Options::CreateOptions opt, bool quiet ) {

  // We cache the MessageSvc, but only once it has been found. This ensures that an
  // early call to this method (before MessageSvc is available) does not prevent
  // from finding it in subsequent calls. The limited use of ServiceHandle for this
  // purpose should be thread-safe:
  static ServiceHandle<IMessageSvc> msgSvc ATLAS_THREAD_SAFE ("MessageSvc", "getMessageSvc");

  IMessageSvc* svc = nullptr;
  try {
    svc = msgSvc.get();
    if (svc) {
      svc->addRef();  // even if cached, maintain correct ref-count
      return svc;
    }

    const bool warn = !(quiet || Athena::getMessageSvcQuiet);
    if ( ((opt==Athena::Options::Lazy && !Gaudi::svcLocator()->existsService("MessageSvc")) ||
          msgSvc.retrieve().isFailure()) && warn ) {
      std::cerr << "Athena::getMessageSvc: WARNING MessageSvc not found, will use std::cout" << std::endl;
    }
    svc = msgSvc.get();
  }
  catch (const GaudiException& e)
  {
    std::cerr << "Athena::getMessageSvc: WARNING Got Gaudi exception " << e
              << "; will use std::out" << std::endl;
    return nullptr;
  }


  return svc;
}


void Athena::reportMessage (IMessageSvc* ims, const std::string &source, int type, const std::string &message) {
  if (ims) ims->reportMessage(source, type, message);
}

int Athena::outputLevel(const IMessageSvc* ims, const std::string &source) {
  if (ims) return ims->outputLevel(source);
  else return MSG::INFO;
}

void Athena::setOutputLevel(IMessageSvc* ims, const std::string &source, int level) {
  if(ims) ims->setOutputLevel(source, level);
}

IMessageSvcHolder::IMessageSvcHolder(IMessageSvc *ims) : m_ims(ims) {
  assert(m_ims);
  m_ims->addRef(); //take ownership till we go out of scope
}

IMessageSvcHolder::IMessageSvcHolder(const IMessageSvcHolder& rhs) :
  m_ims(rhs.m_ims) 
{
  if (m_ims) m_ims->addRef(); //take ownership till we go out of scope
}

IMessageSvcHolder& 
IMessageSvcHolder::operator=(const IMessageSvcHolder& rhs) {
  if (this != & rhs && m_ims != rhs.m_ims) {
    if (m_ims) m_ims->release(); //give up our IMessageSvc*
    m_ims = rhs.m_ims;
    if (m_ims) m_ims->addRef(); //take ownership till we go out of scope
  }
  return *this;
}

IMessageSvcHolder::IMessageSvcHolder( const Options::CreateOptions opt ) :
  m_ims(0)
{
  if (opt == Athena::Options::Eager) m_ims = getMessageSvc(opt);
}

IMessageSvcHolder::~IMessageSvcHolder() {
  if (m_ims) m_ims->release();
}

IMessageSvc*
IMessageSvcHolder::get() {
  if (!m_ims) m_ims = getMessageSvc();
  return m_ims;
}
