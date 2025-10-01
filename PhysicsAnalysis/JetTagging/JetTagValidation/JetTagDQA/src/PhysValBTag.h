///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// PhysValBTag.h
// Header file for class PhysValBTag
// Author: E.Schopf<elisabeth.schopf@cern.ch>
// Updates: J.Hoefer <judith.hoefer@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef JETTAGDQA_PHYSVALBTag_H
#define JETTAGDQA_PHYSVALBTag_H 1

// STL includes
#include <string>

// FrameWork includes
#include "GaudiKernel/ServiceHandle.h"

// Local includes
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "TrkValHistUtils/PlotBase.h"
#include "BTaggingValidationPlots.h"

#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"

// Root includes
#include "TH1.h"

// Forward declaration

namespace JetTagDQA {

  class PhysValBTag
    : public ManagedMonitorToolBase
  {
    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:
    // Copy constructor:

    /// Constructor with parameters:
    PhysValBTag( const std::string& type,
                 const std::string& name,
                 const IInterface* parent );

    /// Destructor:
    virtual ~PhysValBTag();

    // Athena algtool's Hooks
    virtual StatusCode initialize();
    virtual StatusCode bookHistograms();
    virtual StatusCode fillHistograms();
    virtual StatusCode procHistograms();


    ///////////////////////////////////////////////////////////////////
    // Const methods:
    ///////////////////////////////////////////////////////////////////
    std::map<const xAOD::TrackParticle*, int> getTrackTruthAssociations(const xAOD::BTagging* btag) const;

    ///////////////////////////////////////////////////////////////////
    // Non-const methods:
    ///////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    // Private data:
    ///////////////////////////////////////////////////////////////////
  private:

    /// Default constructor:
    PhysValBTag();

    ToolHandle<InDet::IInDetTrackTruthOriginTool> m_trackTruthOriginTool{this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool"};

    // isData flag
    bool m_isData;
    bool m_doTrackTruth = true;

    // Containers
    std::string m_jetNameEMTopo;
    std::string m_jetNamePFlow;
    std::string m_jetNameR10;
    std::string m_jetNameTrackJet;

    std::string m_trackName;
    std::string m_vertexName;

    std::map<std::string, JetTagDQA::BTaggingValidationPlots*> m_btagplots;
    
    // histogram definitions
    // the first one is a vector because I can only pass vectors from the joboptions to the algs (and no maps)
    Gaudi::Property< std::vector< std::vector< std::string > > > m_HistogramDefinitionsVector {this, "HistogramDefinitions", {}, "Map with histogram definitions"};
    // have a useful map nevertheless
    std::map< std::string, std::vector< std::string > > m_HistogramDefinitionsMap;

    float m_jetPtCut = -1;
    bool m_onZprime = false;
    float m_jetPtCutTtbar;
    float m_jetPtCutZprime;
    float m_jetPtCutR10;
    float m_jetEtaCut;
    float m_JVTCutAntiKt4EMTopoJets;
    float m_JVTCutLargerEtaAntiKt4EMTopoJets;
    float m_JVTCutAntiKt4EMPFlowJets;
    float m_truthMatchProbabilityCut;

    std::string m_dipsName;
    std::string m_DL1dv01Name;
    std::string m_GN2v01Name;
    std::string m_GN2Xv01Name;

    JetTagDQA::BTaggingValidationPlots m_antiKt4EMTopoPlots;
    JetTagDQA::BTaggingValidationPlots m_antiKt4EMPFlowJetsPlots;
    JetTagDQA::BTaggingValidationPlots m_antiKt10UFOCSSKSoftDropBeta100Zcut10Jets;
    JetTagDQA::BTaggingValidationPlots m_antiKtVR30Rmax4Rmin02PV0TrackJetsPlots;

    int m_nevents;

    StatusCode book(PlotBase& plots);
  };

}

#endif //> !JETTAGDQA_PHYSVALBTag_H
