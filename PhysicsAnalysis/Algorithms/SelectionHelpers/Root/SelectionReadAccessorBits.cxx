/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SelectionHelpers/SelectionReadAccessorBits.h>

#include <PATInterfaces/SystematicSet.h>
#include <SystematicsHandles/ISysHandleBase.h>

//
// method implementations
//

namespace CP
{
  SelectionReadAccessorBits ::
  SelectionReadAccessorBits (const std::string& name)
    : m_constAccessor (name), m_label(name)
  {}



  SelectionType SelectionReadAccessorBits ::
  getBits (const SG::AuxElement& element,
           const CP::SystematicSet * /*sys*/) const
  {
    return m_constAccessor (element);
  }



  bool SelectionReadAccessorBits ::
  getBool (const SG::AuxElement& element,
           const CP::SystematicSet * /*sys*/) const
  {
    return m_constAccessor (element) == selectionAccept();
  }



  bool SelectionReadAccessorBits ::
  isBool () const
  {
    return false;
  }



  std::string SelectionReadAccessorBits ::
  label () const
  {
    return m_label;
  }



  CP::SystematicSet SelectionReadAccessorBits ::
  getInputAffecting (const ISystematicsSvc& /*svc*/,
                     const std::string& /*objectName*/) const
  {
    return CP::SystematicSet ();
  }



  StatusCode SelectionReadAccessorBits ::
  fillSystematics (const ISystematicsSvc& svc,
                   const std::vector<CP::SystematicSet>& /*sysList*/,
                   ISysObjectHandleBase& objectHandle)
  {
    using namespace msgSelectionHelpers;

    ANA_CHECK (objectHandle.addDecorationDependency (svc, m_label, false));
    return StatusCode::SUCCESS;
  }
}
