/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// AthMessaging.cxx 
// Implementation file for class AthMessaging
// Author: S.Binet<binet@cern.ch>, Frank Winklmeier
/////////////////////////////////////////////////////////////////// 

#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaKernel/getMessageSvc.h"

AthMessaging::AthMessaging (IMessageSvc* msgSvc,
                            const std::string& name) :
  m_nm(name), m_imsg(msgSvc)
{}


AthMessaging::AthMessaging (const std::string& name) :
  m_nm(name)
{}


AthMessaging::~AthMessaging()
{}


void AthMessaging::setLevel (MSG::Level lvl)
{
  m_lvl = lvl;
}


/**
 * Initialize our message level and MessageSvc.
 *
 * This method should only be called once.
 */
void AthMessaging::initMessaging() const
{
  m_imsg = Athena::getMessageSvc();
  // If user did not set an explicit level, set a default
  if (m_lvl == MSG::NIL) {
    m_lvl = m_imsg ?
      static_cast<MSG::Level>( m_imsg.load()->outputLevel(m_nm) ) :
      MSG::INFO;
  }
}
