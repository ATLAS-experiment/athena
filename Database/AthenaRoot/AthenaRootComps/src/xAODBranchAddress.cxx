///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// xAODBranchAddress.cxx
// Implementation file for class Athena::xAODBranchAddress
// Author: Johannes ELmsheuser, Will Buttinger
///////////////////////////////////////////////////////////////////

#include "xAODBranchAddress.h"
#include "xAODEvent.h"

#include "AthenaKernel/CLIDRegistry.h"
#include "CxxUtils/checker_macros.h"

#include <typeinfo>


namespace Athena {

///////////////////////////////////////////////////////////////////
// Public methods:
///////////////////////////////////////////////////////////////////

// Constructors
////////////////

/// Default constructor:
xAODBranchAddress::xAODBranchAddress() :
  GenericAddress(),
  m_ptr    ()
{}

/// Copy constructor:
xAODBranchAddress::xAODBranchAddress( const xAODBranchAddress& rhs ) :
  GenericAddress(rhs),
  m_ptr    (rhs.m_ptr)
{}

/// Assignment operator:
xAODBranchAddress&
xAODBranchAddress::operator=( const xAODBranchAddress& rhs )
{
  if (this != &rhs) {
    GenericAddress::operator=(rhs);
    m_ptr    = rhs.m_ptr;
  }
  return *this;
}

/// Constructor with parameters:
xAODBranchAddress::xAODBranchAddress(long svc,
                                     const CLID& clid,
                                     const std::string& p1,
                                     const std::string& p2,
                                     unsigned long ip1,
                                     unsigned long ip2) :
  GenericAddress(svc, clid, p1, p2, ip1, ip2)
{
}


xAOD::Event*
  xAODBranchAddress::event()
{
  return reinterpret_cast<xAOD::Event*>(reinterpret_cast<unsigned long*>(this->ipar()[0]));
}

void
xAODBranchAddress::setEventAddress()
{
  const std::string& br_name = this->par()[1];
  const std::type_info* ti = CLIDRegistry::CLIDToTypeinfo(this->clID());
  xAOD::Event* event = this->event();
  event->setActive(); // ensure we are active event

  // we use the ipar()[1] to flag if we retrieve main event info (0) or metadata (1) (see xAODEventSelector where it creates BranchAddress)
  switch( (this->ipar()[1]) ) {
  case 0:
    { // reading event level info
      static constexpr bool SILENT = false;
      static constexpr bool METADATA = false;
      void* addr ATLAS_THREAD_SAFE = const_cast<void*>(xAOD::getInputObject(*event, br_name, *ti, SILENT, METADATA));
      m_ptr = addr;
    }
    break;
  case 1:
    { // reading metadata
      // FIXME: NEEED TO ACCESS METADATA
      event->setActive(); //ensure we are active event
      static constexpr bool SILENT = false;
      static constexpr bool METADATA = true;
      void* addr ATLAS_THREAD_SAFE = const_cast<void*>(xAOD::getInputObject(*event, br_name, *ti, SILENT, METADATA));
      m_ptr = addr;
    }
    break;
  }
}

} //> end namespace Athena
