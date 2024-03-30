/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CPIBTAGGINGSELECTIONTOOL_H
#define CPIBTAGGINGSELECTIONTOOL_H

#include "AsgTools/IAsgTool.h"
#include "xAODJet/Jet.h"
#include "PATInterfaces/CorrectionCode.h"
#include "PATCore/AcceptData.h"
#include <string>

class IBTaggingSelectionTool : virtual public asg::IAsgTool {

    /// Declare the interface that the class provides
    ASG_TOOL_INTERFACE( IBTagSelectionTool )

    public:

    /// Get the decision using a generic IParticle pointer
    virtual asg::AcceptData accept( const xAOD::IParticle* p ) const = 0;
    virtual asg::AcceptData accept( const xAOD::Jet& j ) const = 0;
    /// Get the decision using thet jet's pt and weight values (number of weight values depends on which tagger is used)
    virtual asg::AcceptData accept(double /* jet pt */, double /* jet eta */, double /* tag_weight */ ) const = 0;
    virtual asg::AcceptData accept(double /* jet pt */, double /* jet eta*/ , double /* taggerWeight_b */, double /* taggerWeight_c */) const = 0;
    virtual asg::AcceptData accept(double /* jet pt */, double /* jet eta */, double /* dl1pb */, double /* dl1pc  */ , double /* dl1pu  */) const = 0;
    virtual asg::AcceptData accept(double /* jet pt */, double /* jet eta */, double /* dl1pb */, double /* dl1pc  */ , double /* dl1pu  */, double /* dl1ptau */) const = 0;

    /// Decide in which quantile of the tagger weight distribution the jet belongs
    /// The return value represents the bin index of the quantile distribution
    virtual int getQuantile( const xAOD::IParticle* ) const = 0;
    virtual int getQuantile( const xAOD::Jet& ) const = 0;
    virtual int getQuantile( double  /* jet pt */, double  /* jet eta */, double /* tag weight */) const = 0;
    virtual int getQuantile( double /*pT*/, double /*eta*/, double /*tag_weight_b*/, double /*tag_weight_c*/ ) const = 0;

    virtual CP::CorrectionCode getCutValue(double /* jet pt */, double & cutval) const = 0;
    virtual CP::CorrectionCode getTaggerWeight( const xAOD::Jet& jet, double & tagweight) const = 0;
    virtual CP::CorrectionCode getTaggerWeight( double pb, double pc, double pu, double & tagweight) const = 0;
    virtual CP::CorrectionCode getTaggerWeight( double pb, double pc, double pu, double & tagweight, double ptau) const = 0;

    //flexibility for Continuous2D
    virtual CP::CorrectionCode getTaggerWeight( const xAOD::Jet& jet, double & weight ,bool getCTagW) const = 0;
    virtual CP::CorrectionCode getTaggerWeight( double /* dl1pb */, double /* dl1pc  */ , double /* dl1pu  */ , double & weight, bool getCTagW, double /* dl1ptau  */ = 0.) const = 0;
  };
#endif // CPIBTAGGINGSELECTIONTOOL_H
