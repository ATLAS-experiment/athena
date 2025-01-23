/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ASG_TOOLS__UNIT_TEST_SERVICE1_H
#define ASG_TOOLS__UNIT_TEST_SERVICE1_H

#include <AsgServices/AsgService.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgExampleTools/IUnitTestService1.h>

namespace asg
{
  /// \brief a service used to unit test AnaToolHandle
  ///
  /// This allows to unit test the various capabilities of
  /// AnaToolHandle in a controlled fashion.

  struct UnitTestService1 : extends<AsgService, IUnitTestService1>
  {
  public:
    /// \brief standard constructor
    UnitTestService1 (const std::string& name, ISvcLocator* pSvcLocator);

    /// \brief standard destructor
    ~UnitTestService1 ();

    virtual StatusCode initialize () override;
    virtual std::string getPropertyString () const override;
    virtual int getPropertyInt () const override;
    virtual void setPropertyInt (int val_property) override;
    virtual bool isInitialized () const override;

  private:
    /// \brief whether initialize has been called
    bool m_isInitialized{false};

    /// \brief the string property
    Gaudi::Property<std::string> m_propertyString{this, "propertyString", {}, "the string property"};

    /// \brief the integer property
    Gaudi::Property<int> m_propertyInt{this, "propertyInt", -7, "the integer property"};

    /// \brief whether initialize should fail
    Gaudi::Property<bool> m_initializeFail{this, "initializeFail", false, "whether initialize should fail"};
  };
}

#endif
