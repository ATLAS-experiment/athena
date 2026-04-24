/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SelectionHelpers/SelectionAccessorReadSys.h>

#include <AsgMessaging/MessageCheck.h>
#include <SystematicsHandles/ISysHandleBase.h>
#include <SystematicsHandles/ISystematicsSvc.h>

//
// method implementations
//

namespace CP
{
  SelectionAccessorReadSys ::
  SelectionAccessorReadSys (const std::string& val_selectionName)
    : m_selectionName (val_selectionName)
  {}



  SelectionType SelectionAccessorReadSys ::
  getBits (const SG::AuxElement& element,
           const CP::SystematicSet *sys) const
  {
    if (sys == nullptr)
      throw std::logic_error ("trying to read systematics accessor without systematic");
    auto iter = m_dataCache.find (*sys);
    if (iter == m_dataCache.end())
      throw std::logic_error ("unknown systematic: " + sys->name());
    return std::get<1>(iter->second)->getBits (element, sys);
  }



  bool SelectionAccessorReadSys ::
  getBool (const SG::AuxElement& element,
           const CP::SystematicSet *sys) const
  {
    if (sys == nullptr)
      throw std::logic_error ("trying to read systematics accessor without systematic");
    auto iter = m_dataCache.find (*sys);
    if (iter == m_dataCache.end())
      throw std::logic_error ("unknown systematic: " + sys->name());
    return std::get<1>(iter->second)->getBool (element, sys);
  }


  std::string SelectionAccessorReadSys ::
  label () const
  {
    return m_selectionName;
  }



  bool SelectionAccessorReadSys ::
  isBool () const
  {
    for (auto& item : m_dataCache)
      if (!std::get<1>(item.second)->isBool())
        return false;
    return true;
  }



  CP::SystematicSet SelectionAccessorReadSys ::
  getInputAffecting (const ISystematicsSvc& svc,
                     const std::string& objectName) const
  {
    std::string baseName = m_selectionName;
    if (auto split = baseName.find (',');
        split != std::string::npos)
      baseName.resize (split);
    return svc.getDecorSystematics (objectName, baseName);
  }



  StatusCode SelectionAccessorReadSys ::
  fillSystematics (const ISystematicsSvc& svc,
                   const std::vector<CP::SystematicSet>& sysList,
                   ISysObjectHandleBase& objectHandle)
  {
    using namespace msgSelectionHelpers;

    std::string baseName = m_selectionName;
    std::string suffix;
    if (auto split = baseName.find (',');
        split != std::string::npos)
    {
      suffix = baseName.substr (split);
      baseName.resize (split);
    }

    const CP::SystematicSet affecting
      = svc.getDecorSystematics (objectHandle.getNamePattern(), baseName);
    for (auto& sys : sysList)
    {
      CP::SystematicSet inputSys;
      ANA_CHECK (SystematicSet::filterForAffectingSystematics (sys, affecting, inputSys));
      std::string decorName;
      ANA_CHECK (svc.makeSystematicsName (decorName, baseName, inputSys));
      ANA_MSG_DEBUG ("SysReadDecorHandle: " << decorName << " (" << sys.name() << ")");
      std::unique_ptr<ISelectionReadAccessor> accessor;
      ANA_CHECK (makeSelectionReadAccessor (decorName + suffix, accessor));
      m_dataCache.emplace (sys, std::make_tuple (decorName, std::move (accessor)));
    }
    ANA_CHECK (objectHandle.addDecorationDependency (svc, baseName, false));
    return StatusCode::SUCCESS;
  }
}
