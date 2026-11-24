/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4PROFILINGTOOLS_G4UA__TESTACTIONEHISTTOOL_H
#define G4PROFILINGTOOLS_G4UA__TESTACTIONEHISTTOOL_H

#include "G4AtlasTools/UserActionToolBase.h"
#include "TestActionEHist.h"

namespace G4UA
{

  /// @class TestActionEHistTool
  /// @brief A Tool to manage TestActionEHist
  /// Create the TestActionEHist  for each worker thread. Note that since this is using ROOT access
  /// DIRECTLY, it is aonly tested and working with 1 thread. Operation with more than 1 thread
  /// will be disabled from the python side
  ///
  /// @author Andrea Di Simone
  ///
  class TestActionEHistTool:
  public UserActionToolBase<TestActionEHist>
  {

  public:

    using UserActionToolBase<TestActionEHist>::UserActionToolBase;

  protected:

    /// Create action for this thread
    virtual std::unique_ptr<TestActionEHist>
    makeAndFillAction(G4AtlasUserActions&) override final;

  private:

    /// holds the configuration details received from python
    Gaudi::Property<std::string> m_name{this, "ROOTFileName", "default.root", "Name of root output file"};
    Gaudi::Property<int> m_dCALO{this, "CaloDepth", 2};
    Gaudi::Property<int> m_dBeam{this, "BeamPipeDepth", 2};
    Gaudi::Property<int> m_dIDET{this, "InDetDepth", 2};
    Gaudi::Property<int> m_dMUON{this, "MuonDepth", 2};
    Gaudi::Property<int> m_maxhists{this, "MaxHists", 1000};
    Gaudi::Property<std::string> m_dDetail{this, "DetailDepth", "", "Path used to set detailed depth cuts"};

  }; // class TestActionEHistTool

} // namespace G4UA

#endif
