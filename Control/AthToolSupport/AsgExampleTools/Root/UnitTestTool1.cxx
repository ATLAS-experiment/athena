/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgExampleTools/UnitTestTool1.h>

#include <map>
#include <mutex>

//
// method implementations
//

namespace asg
{
  UnitTestTool1 ::
  UnitTestTool1 (const std::string& val_name)
    : AsgTool (val_name)
  {
    std::scoped_lock lock(m_mutex);
    ++ m_instances[name()];

    ANA_MSG_DEBUG ("create UnitTestTool1 " << this);
  }



  UnitTestTool1 ::
  ~UnitTestTool1 ()
  {
    ANA_MSG_DEBUG ("destroy UnitTestTool1 " << this);

    std::scoped_lock lock(m_mutex);
    -- m_instances[name()];
  }



  StatusCode UnitTestTool1 ::
  initialize ()
  {
    ANA_MSG_INFO ("initialize UnitTestTool1 " << this);
    ANA_MSG_INFO ("  propertyString: " << m_propertyString);
    ANA_MSG_INFO ("  propertyInt: " << m_propertyInt);

    if (m_initializeFail)
    {
      ATH_MSG_ERROR ("tool configured to fail initialize");
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



  std::string UnitTestTool1 ::
  getPropertyString () const
  {
    return m_propertyString;
  }



  int UnitTestTool1 ::
  getPropertyInt () const
  {
    return m_propertyInt;
  }



  void UnitTestTool1 ::
  setPropertyInt (int val_property)
  {
    m_propertyInt = val_property;
  }



  bool UnitTestTool1 ::
  isInitialized () const
  {
    return m_isInitialized;
  }



  int UnitTestTool1 ::
  instance_counts (const std::string& name)
  {
    std::scoped_lock lock(m_mutex);
    auto iter = m_instances.find(name);
    assert (iter != m_instances.end());
    return iter->second;
  }
}
