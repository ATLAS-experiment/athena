/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CPIBTAGGINGSELECTIONJSONTOOL_H
#define CPIBTAGGINGSELECTIONJSONTOOL_H

#include "AsgTools/IAsgTool.h"
#include "xAODJet/Jet.h"

class IBTaggingSelectionJsonTool : virtual public asg::IAsgTool {

    ASG_TOOL_INTERFACE( IBTagSelectionJsonTool )

    public:
    virtual int accept( const xAOD::Jet& jet ) const = 0;

    virtual double getTaggerDiscriminant ( const xAOD::Jet& jet ) const = 0;

  };
#endif // CPIBTAGGINGSELECTIONJSONTOOL_H
