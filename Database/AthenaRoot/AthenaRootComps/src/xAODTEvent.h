///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// xAODTEvent.h 
// Header file for class xAODTEvent
// Author: Johannes Elmsheuser, Will Buttinger 
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENAROOTCOMPS_XAODTEVENT_H
#define ATHENAROOTCOMPS_XAODTEVENT_H 1

//This class exists purely to gain public access to the getInputObject method of
//xAOD::TEvent. We need this because we cannot use the templated retrieve method


#include "xAODRootAccess/TEvent.h"

namespace xAOD {

class xAODTEvent : public xAOD::TEvent
{ 
 public: 
  xAODTEvent(EAuxMode mode = kClassAccess)
    : xAOD::TEvent (mode)
  {
    m_ctx = Gaudi::Hive::currentContext();
  }
  using TEvent::getInputObject;
}; 

} //> end namespace xAOD

#endif //> !ATHENAROOTCOMPS_XAODTEVENT_H
