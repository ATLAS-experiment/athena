/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
/// class IBTaggingLargeRSelectionJsonTool
///
/// Interface for the tool that use a large-R as input and returns if it is 
/// Xbb-tagged or not 
///
///////////////////////////////////////////////////////////////////

#ifndef CPIBTAGGINGLARGERSELECTIONJSONTOOL_H
#define CPIBTAGGINGLARGERSELECTIONJSONTOOL_H

#include "AsgTools/IAsgTool.h"
#include "xAODJet/Jet.h"

class IBTaggingLargeRSelectionJsonTool : virtual public asg::IAsgTool {

    ASG_TOOL_INTERFACE( IBTagLargeRSelectionJsonTool )

    public:
    virtual int accept( const xAOD::Jet& jet ) const = 0;
    // the following function is only for Xbb calibration team, for physics analyses, please use the one above.
    virtual int acceptOnlyForXbbCalibrationUsage(double pt, double eta, double mass, double tagger_discriminant) const = 0;

    virtual double getTaggerDiscriminant ( const xAOD::Jet& jet ) const = 0;

  };
#endif // CPIBTAGGINGLARGESELECTIONJSONTOOL_H
