// -*- mode: c++ -*-
//
//  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHASGEXUNITTEST_ATHEXUNITTESTTOOL_H
#define ATHASGEXUNITTEST_ATHEXUNITTESTTOOL_H 1

#include "AsgTools/AsgTool.h"
#include "AthAsgExUnittest/IAthAsgExUnittestTool.h"

class AthAsgExUnittestTool: public asg::AsgTool, public virtual IAthAsgExUnittestTool {
public:

  ASG_TOOL_CLASS( AthAsgExUnittestTool, IAthAsgExUnittestTool )
  // Add another constructor for non-athena use cases
  AthAsgExUnittestTool( const std::string& name );

  // Initialize is required by AsgTool base class
  virtual StatusCode initialize() override;

  // This tools method
  virtual double useTheProperty() override;

private:
  Gaudi::Property<double> m_nProperty{this, "Property", 3.0, "A double property"};
  Gaudi::Property<unsigned int> m_enumProperty{this, "ENumProperty", Val1, "A enum property"};

};

#endif //> !ATHASGEXUNITTEST_ATHEXUNITTESTTOOL_H
