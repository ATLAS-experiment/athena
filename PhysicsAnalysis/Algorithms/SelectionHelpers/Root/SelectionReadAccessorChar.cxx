/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SelectionHelpers/SelectionReadAccessorChar.h>

#include <PATInterfaces/SystematicSet.h>
#include <SystematicsHandles/ISysHandleBase.h>

//
// method implementations
//

namespace CP
{
  SelectionReadAccessorChar ::
  SelectionReadAccessorChar (const std::string& name)
    : m_constAccessor (name), m_label(name)
  {}



  SelectionType SelectionReadAccessorChar ::
  getBits (const SG::AuxElement& element,
           const CP::SystematicSet * /*sys*/) const
  {
    if (m_constAccessor (element))
      return selectionAccept();
    else
      return 0;
  }



  bool SelectionReadAccessorChar ::
  getBool (const SG::AuxElement& element,
           const CP::SystematicSet * /*sys*/) const
  {
    return m_constAccessor (element);
  }



  std::string SelectionReadAccessorChar ::
  label () const
  {
    return m_label;
  }



  bool SelectionReadAccessorChar ::
  isBool () const
  {
    return true;
  }



  CP::SystematicSet SelectionReadAccessorChar ::
  getInputAffecting (const ISystematicsSvc& /*svc*/,
                     const std::string& /*objectName*/) const
  {
    return CP::SystematicSet ();
  }



  StatusCode SelectionReadAccessorChar ::
  fillSystematics (const ISystematicsSvc& svc,
                   const std::vector<CP::SystematicSet>& /*sysList*/,
                   ISysObjectHandleBase& objectHandle)
  {
    using namespace msgSelectionHelpers;

    ANA_CHECK (objectHandle.addDecorationDependency (svc, m_label, false));
    return StatusCode::SUCCESS;
  }
}
