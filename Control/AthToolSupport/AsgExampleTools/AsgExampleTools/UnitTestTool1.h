/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ASG_TOOLS__UNIT_TEST_TOOL1_H
#define ASG_TOOLS__UNIT_TEST_TOOL1_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgExampleTools/IUnitTestTool1.h>
#include <CxxUtils/checker_macros.h>

#include <mutex>

namespace asg
{
  /// \brief a tool used to unit test AnaToolHandle
  ///
  /// This allows to unit test the various capabilities of
  /// AnaToolHandle in a controlled fashion.

  struct UnitTestTool1 : virtual public IUnitTestTool1,
			 public AsgTool
  {
  public:
    ASG_TOOL_CLASS (UnitTestTool1, IUnitTestTool1)

    /// \brief standard constructor
    UnitTestTool1 (const std::string& val_name);

    /// \brief standard destructor
    ~UnitTestTool1 ();

    virtual StatusCode initialize () override;
    virtual std::string getPropertyString () const override;
    virtual int getPropertyInt () const override;
    virtual void setPropertyInt (int val_property) override;
    virtual bool isInitialized () const override;

    /// \brief the number of times the tool of the given name has been
    /// instantiated
    static int instance_counts (const std::string& name);

  private:
    /// \brief whether initialize has been called
    bool m_isInitialized = false;

    /// \brief instance counts per name
    inline static std::map<std::string, int> m_instances ATLAS_THREAD_SAFE;

    /// \brief mutex for above map
    inline static std::mutex m_mutex;

    /// \brief the string property
    Gaudi::Property<std::string> m_propertyString{this, "propertyString", {}, "the string property"};

    /// \brief the integer property
    Gaudi::Property<int> m_propertyInt{this, "propertyInt", 0, "the integer property"};

    /// \brief whether initialize should fail
    Gaudi::Property<bool> m_initializeFail{this, "initializeFail", false, "whether initialize should fail"};
  };
}

#endif
