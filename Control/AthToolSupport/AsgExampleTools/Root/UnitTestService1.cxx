/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgExampleTools/UnitTestService1.h>
#include <AsgMessaging/MessageCheck.h>

//
// method implementations
//

namespace asg
{
  UnitTestService1 ::
  UnitTestService1 (const std::string& name, ISvcLocator* pSvcLocator)
    : base_class (name, pSvcLocator)
  {
    ANA_MSG_DEBUG ("create UnitTestService1 " << this);
  }



  UnitTestService1 ::
  ~UnitTestService1 ()
  {
    ANA_MSG_DEBUG ("destroy UnitTestService1 " << this);
  }



  StatusCode UnitTestService1 ::
  initialize ()
  {
    ANA_MSG_INFO ("initialize UnitTestService1 " << this);
    ANA_MSG_INFO ("  propertyString: " << m_propertyString);
    ANA_MSG_INFO ("  propertyInt: " << m_propertyInt);

    if (m_initializeFail)
    {
      ATH_MSG_ERROR ("service configured to fail initialize");
      return StatusCode::FAILURE;
    }
    if (m_isInitialized)
    {
      ATH_MSG_ERROR ("initialize called twice");
      return StatusCode::FAILURE;
    }
    m_isInitialized = true;
    return StatusCode::SUCCESS;
  }



  std::string UnitTestService1 ::
  getPropertyString () const
  {
    return m_propertyString;
  }



  int UnitTestService1 ::
  getPropertyInt () const
  {
    return m_propertyInt;
  }



  void UnitTestService1 ::
  setPropertyInt (int val_property)
  {
    m_propertyInt = val_property;
  }



  bool UnitTestService1 ::
  isInitialized () const
  {
    return m_isInitialized;
  }

}
