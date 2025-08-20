// -*- c++ -*-
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKSYSTEMATICSTOOLS_JETTRACKFILTERTOOL_H
#define INDETTRACKSYSTEMATICSTOOLS_JETTRACKFILTERTOOL_H

#include "InDetTrackSystematicsTools/IJetTrackFilterTool.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "PATInterfaces/SystematicVariation.h"
#include "PATInterfaces/SystematicSet.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODJet/JetContainer.h"
#include "InDetTrackSystematicsTools/InDetTrackSystematicsTool.h"

#include <string>

class TH1;
class TH2;
class TRandom3;
class TFile;

namespace InDet {

  class IInDetTrackTruthOriginTool;

  /// @class JetTrackFilterTool
  /// This tool randomly discards tracks in the core of a jet
  /// @author Felix Clark (michael.ryan.clark@cern.ch)

  class JetTrackFilterTool
    : public virtual IJetTrackFilterTool
    , public virtual InDetTrackSystematicsTool
  //    , public asg::AsgTool 
  {

    // create constructor for Athena
    ASG_TOOL_CLASS( JetTrackFilterTool,
        InDet::IJetTrackFilterTool )

  public:
    // create constructor for standalone Root
    JetTrackFilterTool( const std::string& name );

    //  static const InterfaceID& interfaceID();
    virtual StatusCode initialize() override;
    virtual void prepare() override {};

    // right now this returns a bool; if we want to implement the ASG selection tool interface then this will need to change to a TAccept
    virtual bool accept( const xAOD::TrackParticle*, const xAOD::Jet* ) const override;
    virtual bool accept( const xAOD::TrackParticle*, const xAOD::JetContainer* ) const override;

    /// returns: whether the tool is affected by the systematic
    virtual bool isAffectedBySystematic( const CP::SystematicVariation& ) const override;
    /// returns: list of systematics this tool can be affected by
    virtual CP::SystematicSet affectingSystematics() const override;
    /// returns: list of recommended systematics to use with this tool
    virtual CP::SystematicSet recommendedSystematics() const override;
    /// configure the tool to apply a given list of systematic variations
    virtual StatusCode applySystematicVariation( const CP::SystematicSet& ) override;

  private:

    float getNomTrkEff(const xAOD::TrackParticle*) const;

    Gaudi::Property<int> m_seed{this, "Seed", 0,
      "Seed used to initialize the RNG"};
    std::unique_ptr<TRandom3> m_rnd = nullptr; //!
    Gaudi::Property<double> m_deltaR{this, "DeltaR", 0.1,
      "Delta-R cut in which to apply jet-track efficiency rejection"};
    Gaudi::Property<double> m_minJetPt{this, "minJetPt", 200000.,
      "Minimum jet pT to apply jet-track efficiency rejection (default is 200 GeV)"};
    Gaudi::Property<float> m_trkEffSystScale{this, "trkEffSystScale", 1.0,
      "Option to scale the effect of the systematic (default 1)"};

    std::unique_ptr<TH2> m_trkNomEff = nullptr; //!

    // allow the user to configure which calibration file to use if desired
    Gaudi::Property<std::string> m_calibFileNomEff{this, "calibFileNomEff",
      "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/TrackingRecommendations_prelim_rel22.root"};

    Gaudi::Property<double> m_effUncertTIDE{this, "FLostUncertainty", 0.24,
      "Option to set the uncertainty on FLost"};
    Gaudi::Property<double> m_fakeUncertTIDE{this, "FakeUncertainty", 0.35,
      "Option to set the fake uncertainty"};

    ToolHandle< IInDetTrackTruthOriginTool > m_trackOriginTool
      {this, "trackOriginTool", "InDet::InDetTrackTruthOriginTool"};

  }; // class JetTrackFilterTool

} // namespace InDet

#endif
