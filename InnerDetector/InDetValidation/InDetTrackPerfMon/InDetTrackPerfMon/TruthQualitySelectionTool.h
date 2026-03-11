/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDETTRACKPERFMON_TRUTHQUALITYSELECTIONTOOL_H
#define INDETTRACKPERFMON_TRUTHQUALITYSELECTIONTOOL_H

// Package includes
#include "ITrackSelectionTool.h"
#include "TrkTruthTrackInterfaces/IAthSelectionTool.h"
#include "InDetTrackPerfMon/TrackAnalysisCollections.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"

// Framework includes
#include "AsgTools/AsgTool.h"
#include "AsgTools/AnaToolHandle.h"

// STL includes
#include <string>

/**
 * @class TruthQualitySelectionTool
 * @brief
 **/

namespace IDTPM {

class TruthQualitySelectionTool :  
      public virtual IDTPM::ITrackSelectionTool,  
      public asg::AsgTool {

public:

  ASG_TOOL_CLASS( TruthQualitySelectionTool, ITrackSelectionTool );
   
  TruthQualitySelectionTool( const std::string& name );

  virtual StatusCode initialize() override;

  virtual StatusCode selectTracks(
      TrackAnalysisCollections& trkAnaColls ) override;

  bool accept(const xAOD::TruthParticle* truth);
  const xAOD::TruthParticle* getParent( const xAOD::TruthParticle* truth, int flav) const;
  const xAOD::TruthParticle* getParentRec(const xAOD::TruthParticle* truth, int flav, int depth) const;
private:

  ToolHandle< IAthSelectionTool > m_truthTool { this, "truthTool", {}, "Main truth selection tool" };
  ToolHandle< InDet::IInDetTrackTruthOriginTool > m_trackTruthOriginTool {
    this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool", "truth track origin tool" };

  FloatProperty   m_minAbsEta   { this, "minAbsEta", -9999., "Lower cut on |eta| for truth particles" };
  FloatProperty   m_minAbsPhi   { this, "minAbsPhi", -9999., "Lower cut on |phi| for truth particles" };
  FloatProperty   m_maxAbsPhi   { this, "maxAbsPhi", -9999., "Higher cut on |phi| for truth particles" };
  FloatProperty   m_minAbsD0    { this, "minAbsD0", -9999., "Lower cut on |d0| for truth particles" };
  FloatProperty   m_maxAbsD0    { this, "maxAbsD0", -9999., "Higher cut on |d0| for truth particles" };
  FloatProperty   m_minAbsZ0    { this, "minAbsZ0", -9999., "Lower cut on |z0| for truth particles" };
  FloatProperty   m_maxAbsZ0    { this, "maxAbsZ0", -9999., "Higher cut on |z0| for truth particles" };
  FloatProperty   m_minAbsQoPT  { this, "minAbsQoPT", -9999., "Lower cut on |q/pt| for truth particles" };
  FloatProperty   m_maxAbsQoPT  { this, "maxAbsQoPT", -9999., "Higher cut on |q/pt| for truth particles" };
  FloatProperty   m_minEta      { this, "minEta", -9999., "Lower cut on eta for truth particles" };
  FloatProperty   m_maxEta      { this, "maxEta", -9999., "Higher cut on eta for truth particles" };
  FloatProperty   m_minPhi      { this, "minPhi", -9999., "Lower cut on phi for truth particles" };
  FloatProperty   m_maxPhi      { this, "maxPhi", -9999., "Higher cut on phi for truth particles" };
  FloatProperty   m_minD0       { this, "minD0", -9999., "Lower cut on d0 for truth particles" };
  FloatProperty   m_maxD0       { this, "maxD0", -9999., "Higher cut on d0 for truth particles" };
  FloatProperty   m_minZ0       { this, "minZ0", -9999., "Lower cut on z0 for truth particles" };
  FloatProperty   m_maxZ0       { this, "maxZ0", -9999., "Higher cut on z0 for truth particles" };
  FloatProperty   m_minQoPT     { this, "minQoPT", -9999., "Lower cut on q/pt for truth particles" };
  FloatProperty   m_maxQoPT     { this, "maxQoPT", -9999., "Higher cut on q/pt for truth particles" };
  BooleanProperty m_isHadron    { this, "isHadron", false, "Select hadrons" };
  BooleanProperty m_isPion      { this, "isPion",   false, "Select pions" };
  BooleanProperty m_isFromB     { this, "isFromB",  false, "Select particles from B hadron decay" };
  BooleanProperty m_isFromC     { this, "isFromC",  false, "Select particles from C hadron decay" };
  BooleanProperty m_isFromHeavyFlav { this, "isFromHeavyFlav",  false, "Select particles from heavy-flvour (B || C) hadron decay" };
  BooleanProperty m_isFromLightFlav { this, "isFromLightFlav",  false, "Select particles from light-flvour hadron decay" };

  BooleanProperty m_isFromTau { this, "isFromTau", false, "Selects particles from tau hadron decay"};
  FloatProperty m_minParentPt {this, "minParentPt", -9999., "Min cut on parent truth particles"};
  FloatProperty m_maxParentPt {this, "maxParentPt", -9999., "Max cut on parent truth particles"};
};

} // namespace IDTPM

#endif // INDETTRACKPERFMON_TRUTHQUALITYSELECTIONTOOL_H
