/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ApplyE2YFakeRate_H
#define ApplyE2YFakeRate_H

#include <FakeBkgTools/BaseLinearFakeBkgTool.h>
#include <array>

namespace CP
{

class ATLAS_NOT_THREAD_SAFE ApplyE2YFakeRate : public CP::BaseLinearFakeBkgTool
{

  ASG_TOOL_CLASS2(ApplyE2YFakeRate, ILinearFakeBkgTool, IFakeBkgTool)

public: 
  /// Standard constructor
  ApplyE2YFakeRate(const std::string& name);
  
  /// Standard destructor
  ~ApplyE2YFakeRate();

  // Main methods
public:
  // Initialize this class
  
  virtual StatusCode initialize() override;

protected:
  // Methods which must be implemented
  virtual StatusCode addEventCustom() override;
  virtual StatusCode getEventWeightCustom(FakeBkgTools::Weight& weight, const FakeBkgTools::FinalState& fs) override;

  /// This indicates which type of efficiencies/fake factor need to be filled
  virtual FakeBkgTools::Client clientForDB() override final;
  
private:
  int m_e2y_option=1;
};

}

//----------------------------------------------------------------------------------------
#endif
