/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SelectionHelpers/SelectionNameSvc.h>

#include <AsgMessaging/MessageCheck.h>
#include <PATInterfaces/MakeSystematicsVector.h>
#include <cassert>
#include <cmath>
#include <regex>
#include <functional>

//
// method implementations
//

namespace CP
{

  StatusCode SelectionNameSvc ::
  initialize ()
  {
    ANA_CHECK (m_sysSvc.retrieve());
    return StatusCode::SUCCESS;
  }



  StatusCode SelectionNameSvc ::
  addAcceptInfo (const std::string& objectName, const std::string& decorName,
        const asg::AcceptInfo& acceptInfo)
  {
    ANA_MSG_DEBUG ("adding selection " << decorName << " for object " << objectName);
    auto& subMap = m_acceptInfoMap[decorName];
    if (subMap.find (objectName) != subMap.end())
    {
      ANA_MSG_ERROR ("object " << objectName << " already has a selection named " << decorName);
      return StatusCode::FAILURE;
    }
    subMap[objectName] = acceptInfo;
    if (getAcceptInfo(objectName, decorName) == nullptr)
    {
      ANA_MSG_ERROR ("failed to add selection " << decorName << " for object " << objectName);
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }



  const asg::AcceptInfo* SelectionNameSvc ::
  getAcceptInfo (const std::string& objectName,
        const std::string& decorName) const
  {
    ANA_MSG_DEBUG ("querying selection " << decorName << " for object " << objectName);
    auto subMap = m_acceptInfoMap.find (decorName);
    if (subMap == m_acceptInfoMap.end())
      return nullptr;
    std::string myObjectName = objectName;
    while (!myObjectName.empty())
    {
      auto result = subMap->second.find (myObjectName);
      if (result != subMap->second.end())
      {
        ANA_MSG_DEBUG ("found selection " << decorName << " for object " << objectName << " using name " << myObjectName);
        return &result->second;
      }
      myObjectName = m_sysSvc->getCopySource (myObjectName);
    }
    return nullptr;
  }
}
