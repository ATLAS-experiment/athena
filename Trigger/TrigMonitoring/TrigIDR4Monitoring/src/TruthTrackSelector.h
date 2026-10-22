/** emacs: this is -*- c++ -*-
 **
 **  @file  TruthTrackSelector.h
 **  @class TruthTrackSelector
 **
 **  @brief a selector of truth particles, including the flags for ancestry
 **         navigation
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/

#ifndef IDTPM2_TRUTHTRACKSELECTOR_H
#define IDTPM2_TRUTHTRACKSELECTOR_H

// Package includes
#include "GenericTrackSelector.h"
#include "AncestorFinder.h"

// Framework includes
// #include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h" 
#include "xAODTruth/TruthParticleContainer.h"

#include "TrkTruthTrackInterfaces/IAthSelectionTool.h"
#include "AsgTools/AnaToolHandle.h"

#include <string>
#include <memory>



namespace IDTPM2 {

class TruthTrackSelector : public virtual GenericTrackSelector {

public:

  ASG_TOOL_CLASS( TruthTrackSelector, ITrackSelector );
   
  TruthTrackSelector( const std::string& name );

  virtual StatusCode initialize() override;

  bool selectTrack( const TrackView& t ) const;
  
  const xAOD::TruthParticle* getParent( const xAOD::TruthParticle* truth, int flav, int depth=0 ) const;
  
private:

  ToolHandle< IAthSelectionTool > m_truthTool  { this, "truthTool", {}, "Main truth selection tool" };

  std::unique_ptr<IFinder<xAOD::TruthParticle> > m_ancestorFinder = 0;
  
  /// This here is never used, so comment it out for the
  /// time being in case it is important
  //  ToolHandle< InDet::IInDetTrackTruthOriginTool > m_trackTruthOriginTool {
  //    this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool", "truth track origin tool" };

  Gaudi::Property<ConfiguredValue<int>>  m_pdgId             { this, "PdgId",           {}, "pdgid is the required particle" };

  Gaudi::Property<ConfiguredValue<int>>  m_fromHadronPdgId   { this, "FromHadronPdgId", {}, "pdgid is the species of hadron, ie MB::b for a B hadron etc"}; 
  Gaudi::Property<ConfiguredValue<int>>  m_fromPdgId         { this, "FromPdgId",       {}, "pdgid is the required ancestor" };

  Gaudi::Property<bool> m_useTruthTool { this, "UseTruthTool", false, "use the offline truth tool" };
  
  Gaudi::Property<bool> m_isHadron     { this, "isHadron", false, "Select hadrons" };
  Gaudi::Property<bool> m_isPion       { this, "isPion",   false, "Select pions" };

  /// set with parent pdgid = 4 or 5
  //  BooleanProperty m_isFromB     { this, "isFromB",  false, "Select particles from B hadron decay" };
  //  BooleanProperty m_isFromC     { this, "isFromC",  false, "Select particles from C hadron decay" };
  Gaudi::Property<bool> m_isFromHeavyFlavour { this, "isFromHeavyFlavour",  false, "Select particles from heavy-flvour (B || C) hadron decay" };
  Gaudi::Property<bool> m_isFromLightFlavour { this, "isFromLightFlavour",  false, "Select particles from light-flvour hadron decay" };

  /// set with parent pdgid = 15
  //  BooleanProperty m_isFromTau   { this, "isFromTau", false, "Selects particles from tau hadron decay"};

  /// max PT ???
  Gaudi::Property<ConfiguredValue<float>>   m_minParentPt {this, "minParentPt", {}, "Min cut on parent truth particles"};
  Gaudi::Property<ConfiguredValue<float>>   m_maxParentPt {this, "maxParentPt", {}, "Max cut on parent truth particles"};
};

} // namespace IDTPM2

#endif // IDTPM2_TRUTHTRACKSELECTOR_H
