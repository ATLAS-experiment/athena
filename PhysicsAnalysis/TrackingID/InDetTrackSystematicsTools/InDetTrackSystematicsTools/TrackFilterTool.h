// -*- c++ -*-
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKSYSTEMATICSTOOLS_TRACKFILTERTOOL_H
#define INDETTRACKSYSTEMATICSTOOLS_TRACKFILTERTOOL_H

#include "InDetTrackSystematicsTools/ITrackFilterTool.h"
#include "InDetTrackSystematicsTools/IInDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/InDetTrackSystematicsTool.h"

#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "PATInterfaces/SystematicVariation.h"
#include "PATInterfaces/SystematicSet.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODJet/JetContainer.h"

#include "xAODTracking/TrackParticle.h"
#include <mutex>
#include <string>

class TH1;
class TH2;
class TFile;

namespace InDet {

  /// @class TrackFilterTool
  /// This class selects tracks based on their truth origin
  /// and tracks in the core of a jet
  /// @author Henry van der Smagt (henry.van.der.smagt@cern.ch)
  /// This tool is a mix of the InDetTrackTruthFilterTool and the JetTrackFilterTool
  /// @author Remi Zaidan (remi.zaidan@cern.ch)
  /// @author Felix Clark (michael.ryan.clark@cern.ch)

  class TrackFilterTool
    : public virtual ITrackFilterTool
    , public virtual InDetTrackSystematicsTool
  //    , public asg::AsgTool
  {

    // create constructor for Athena
    ASG_TOOL_CLASS( TrackFilterTool,
            InDet::ITrackFilterTool )

  public:
    // create constructor for standalone Root
    TrackFilterTool( const std::string& name );

    virtual ~TrackFilterTool();

    //  static const InterfaceID& interfaceID();
    virtual StatusCode initialize() override;
    virtual void prepare() override {};

    // right now this returns a bool; if we want to implement the ASG selection tool interface then this will need to change to a TAccept

    // accept method to determine if a track should be kept or not
    virtual bool accept(const xAOD::TrackParticle* track) const override;
    /// Reentrant accept (see IInDetTrackTruthFilterTool)
    virtual bool accept(const xAOD::TrackParticle* track,
                        const CP::SystematicSet& syst) const override;
    virtual bool accept( const xAOD::TrackParticle*, const xAOD::Jet* ) const override;
    virtual bool accept( const xAOD::TrackParticle*, const xAOD::JetContainer* ) const override;
    /// Reentrant accept (see IJetTrackFilterTool)
    virtual bool accept(const xAOD::TrackParticle*,
                        const xAOD::JetContainer*,
                        const CP::SystematicSet& syst) const override;

    bool acceptInclusive(
    const xAOD::TrackParticle* track) const;

    bool acceptJet(
        const xAOD::TrackParticle* track,
        const xAOD::Jet* jet) const;

    bool isInJet(
        const xAOD::TrackParticle* track,
        const xAOD::Jet* jet) const;

    /// returns: whether the tool is affected by the systematic
    virtual bool isAffectedBySystematic( const CP::SystematicVariation& ) const override;
    /// returns: list of systematics this tool can be affected by
    virtual CP::SystematicSet affectingSystematics() const override;
    /// returns: list of recommended systematics to use with this tool
    virtual CP::SystematicSet recommendedSystematics() const override;
    /// configure the tool to apply a given list of systematic variations
    virtual StatusCode applySystematicVariation( const CP::SystematicSet& ) override;

  private:

    StatusCode initTrkEffSystHistogram(float scale, std::unique_ptr<TH2>& histogram, const std::string & rootFileName, const std::string & histogramName) const;
    float getFractionDropped(float fDefault, const std::unique_ptr<TH2>& histogram, float x, float y, bool xAxisIspT = true) const;
    float getNomTrkJetEff(const xAOD::TrackParticle*) const;

    ToolHandle< IInDetTrackTruthOriginTool > m_trackOriginTool{this, "trackOriginTool", "InDet::InDetTrackTruthOriginTool", "Tool to get the truth origin of a track"};

    Gaudi::Property<int> m_truthSeed{this, "TruthSeed", 1,
      "Seed offset for truth-origin track rejection"};

    Gaudi::Property<int> m_jetSeed{this, "JetSeed", 2,
      "Seed offset for jet-track rejection"};
    Gaudi::Property<float> m_fFakeLoose{this, "fFakeLoose", -1.0, "Fake loose fraction for combined tracks"};
    Gaudi::Property<float> m_fFakeTight{this, "fFakeTight", -1.0, "Fake tight fraction"};
    Gaudi::Property<float> m_trkEffSystScale{this, "trkEffSystScale", 1.0, "Track efficiency systematic scale"};

    // Jet-track
    Gaudi::Property<double> m_effUncertTIDE{this, "FLostUncertainty", 0.24,
      "Option to set the uncertainty on jet FLost"};
    Gaudi::Property<double> m_fFakeUncertTIDE{this, "fFakeUncertTIDE", 0.25,
      "Option to set the fake uncertainty"};
    Gaudi::Property<double> m_deltaR{this, "DeltaR", 0.1,
      "Delta-R cut in which to apply jet-track efficiency rejection"};
    Gaudi::Property<double> m_minJetPt{this, "minJetPt", 200000.,
      "Minimum jet pT to apply jet-track efficiency rejection (default is 200 GeV)"};

    Gaudi::Property<std::string> m_calibFileJetNomEff{
      this,
      "calibFileJetNomEff",
      "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/TrackingRecommendations_prelim_rel22.root",
      "Calibration file for jet-track nominal efficiency"
    };

    Gaudi::Property<std::string> m_calibFileNomEff{
      this,
      "calibFileNomEff",
      "",
      "Calibration file for truth-origin track efficiency"
    };

    std::unique_ptr<TH2> m_trkJetNomEff = nullptr; //!

    std::unique_ptr<TH2> m_trkEffHistLooseGlobal;
    std::unique_ptr<TH2> m_trkEffHistLooseIBL;
    std::unique_ptr<TH2> m_trkEffHistLoosePP0;
    std::unique_ptr<TH2> m_trkEffHistLoosePhysModel;
    std::unique_ptr<TH2> m_trkEffHistTightGlobal;
    std::unique_ptr<TH2> m_trkEffHistTightIBL;
    std::unique_ptr<TH2> m_trkEffHistTightPP0;
    std::unique_ptr<TH2> m_trkEffHistTightPhysModel;

    std::unordered_map<std::string, TH2*> m_histMap;

    mutable std::mutex m_sysLock;

  }; // class TrackFilterTool

} // namespace InDet

#endif
