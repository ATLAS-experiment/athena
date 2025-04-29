// -*- c++ -*-
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKSYSTEMATICSTOOLS_INDETTRACKTRUTHFILTERTOOL_H
#define INDETTRACKSYSTEMATICSTOOLS_INDETTRACKTRUTHFILTERTOOL_H

#include "InDetTrackSystematicsTools/IInDetTrackTruthFilterTool.h"
#include "InDetTrackSystematicsTools/IInDetTrackTruthOriginTool.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "PATInterfaces/SystematicVariation.h"
#include "PATInterfaces/SystematicSet.h"
#include "InDetTrackSystematicsTools/InDetTrackSystematicsTool.h"

#include "xAODTracking/TrackParticle.h"
#include <string>

class TH2;
class TRandom3;
class TFile;

namespace InDet {

  /// @class InDetTrackTruthFilterTool
  /// This class selects tracks based on their truth origin
  /// @author Remi Zaidan (remi.zaidan@cern.ch)
  /// @author Felix Clark (michael.ryan.clark@cern.ch)

  class InDetTrackTruthFilterTool
    : public virtual IInDetTrackTruthFilterTool
    , public virtual InDetTrackSystematicsTool
  //    , public asg::AsgTool 
  {

    // create constructor for Athena
    ASG_TOOL_CLASS( InDetTrackTruthFilterTool,
            InDet::IInDetTrackTruthFilterTool )

  public:
    // create constructor for standalone Root
    InDetTrackTruthFilterTool( const std::string& name );
    virtual ~InDetTrackTruthFilterTool();

    //  static const InterfaceID& interfaceID();
    virtual StatusCode initialize() override;
    virtual void prepare() override {};

    // right now this returns a bool; if we want to implement the ASG selection tool interface then this will need to change to a TAccept

    // accept method to determine if a track should be kept or not
    virtual bool accept(const xAOD::TrackParticle* track) const override;

    /// returns: whether the tool is affected by the systematic
    virtual bool isAffectedBySystematic( const CP::SystematicVariation& ) const override;
    /// returns: list of systematics this tool can be affected by
    virtual CP::SystematicSet affectingSystematics() const override;
    /// returns: list of recommended systematics to use with this tool
    virtual CP::SystematicSet recommendedSystematics() const override;
    /// configure the tool to apply a given list of systematic variations
    virtual StatusCode applySystematicVariation( const CP::SystematicSet& ) override;

    /// directly return a per track uncertainty from the 2D histogram for tight and loose standard tracks in the style of the LRT systematic from InclusiveTrackFilterTool (TEMPORARY FIX -- experts only! Should be superseded by ATLIDTRKCP-665)
    virtual float getTrackUncertainty(const xAOD::TrackParticle* track, const std::string& systName) const override;

  private:

    StatusCode initTrkEffSystHistogram(float scale, TH2 *&histogram, std::string rootFileName, std::string histogramName) const;
    float getFractionDropped(float fDefault, const TH2 *histogram, float x, float y, bool xAxisIspT = true) const;

    ToolHandle< IInDetTrackTruthOriginTool > m_trackOriginTool{this, "trackOriginTool", "InDet::InDetTrackTruthOriginTool", "Tool to get the truth origin of a track"};

    Gaudi::Property<int> m_seed{this, "Seed", 0, "Random seed"};
    std::unique_ptr<TRandom3> m_rnd; //!
    
    Gaudi::Property<float> m_fFakeLoose{this, "fFakeLoose", -1.0, "Fake loose fraction"};
    Gaudi::Property<float> m_fFakeTight{this, "fFakeTight", -1.0, "Fake tight fraction"};
    Gaudi::Property<float> m_trkEffSystScale{this, "trkEffSystScale", 1.0, "Track efficiency systematic scale"};

    TH2* m_trkEffHistLooseGlobal = nullptr;
    TH2* m_trkEffHistLooseIBL = nullptr;
    TH2* m_trkEffHistLoosePP0 = nullptr;
    TH2* m_trkEffHistLoosePhysModel = nullptr;
    TH2* m_trkEffHistTightGlobal = nullptr;
    TH2* m_trkEffHistTightIBL = nullptr;
    TH2* m_trkEffHistTightPP0 = nullptr;
    TH2* m_trkEffHistTightPhysModel = nullptr;

    std::unordered_map<std::string, TH2*> m_histMap;

    // allow the user to configure which calibration files to use if desired
    Gaudi::Property<std::string> m_calibFileNomEff{this, "calibFileNomEff", "", "Calibration file for nominal efficiency"};

  }; // class InDetTrackTruthFilterTool

} // namespace InDet

#endif
