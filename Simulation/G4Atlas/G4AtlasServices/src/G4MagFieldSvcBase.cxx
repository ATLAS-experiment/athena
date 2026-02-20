/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "G4MagFieldSvcBase.h"

#include <memory>

// Geant4 includes
#include "G4MagneticField.hh"

//-----------------------------------------------------------------------------
// Implementation file for class : G4MagFieldSvcBase
//
// 2015-11-17: Andrea Dell'Acqua
//-----------------------------------------------------------------------------


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
G4MagFieldSvcBase::G4MagFieldSvcBase(const std::string& name,
                                     ISvcLocator* pSvcLocator)
  : base_class(name, pSvcLocator)
{
}

//=============================================================================
// Retrieve a G4 mag field object
//=============================================================================
G4MagneticField* G4MagFieldSvcBase::getField()
{
  ATH_MSG_DEBUG("G4MagFieldSvcBase::getField");

  // create a thread-local magnetic field instance
  static thread_local std::unique_ptr<G4MagneticField> field{makeField()};

  return field.get();
}
