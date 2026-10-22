///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// xAODEvent.h
// Header file for classes xAODTEvent and xAODREvent
// Author: Johannes Elmsheuser, Will Buttinger
///////////////////////////////////////////////////////////////////
#ifndef ATHENAROOTCOMPS_XAODEVENT_H
#define ATHENAROOTCOMPS_XAODEVENT_H

// These classes exist purely to gain public access to the getInputObject method of
// xAOD::Event. We need this because we cannot use the templated retrieve method.
// xAODTEvent reads TTree and xAODREvent reads RNTuple input files.


#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/REvent.h"

#include <string>
#include <typeinfo>

namespace xAOD {

// When we create these object, set their EventContext to what's current.
// Don't use the store itself as the context.

class xAODTEvent : public xAOD::TEvent
{
 public:
  xAODTEvent(EAuxMode mode = kClassAccess)
    : xAOD::TEvent (mode, &Gaudi::Hive::currentContext())
  {
  }
  using TEvent::getInputObject;
};

class xAODREvent : public xAOD::Experimental::REvent
{
 public:
  xAODREvent()
    : REvent(&Gaudi::Hive::currentContext())
  {
  }
  using REvent::getInputObject;
};

/// Retrieve an input object from an xAODTEvent or xAODREvent in a non-template way
inline const void* getInputObject(xAOD::Event& event, const std::string& key,
                                  const std::type_info& ti, bool silent, bool metadata)
{
  if (auto* tevent = dynamic_cast<xAODTEvent*>(&event)) {
    return tevent->getInputObject(key, ti, silent, metadata);
  }
  if (auto* revent = dynamic_cast<xAODREvent*>(&event)) {
    return revent->getInputObject(key, ti, silent, metadata);
  }
  return nullptr;
}

} //> end namespace xAOD

#endif //> !ATHENAROOTCOMPS_XAODEVENT_H
