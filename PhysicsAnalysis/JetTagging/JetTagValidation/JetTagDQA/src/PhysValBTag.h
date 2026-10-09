///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// PhysValBTag.h
// Header file for class PhysValBTag
// Author: E.Schopf<elisabeth.schopf@cern.ch>
// Updates: J.Hoefer <judith.hoefer@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef JETTAGDQA_PHYSVALBTag_H
#define JETTAGDQA_PHYSVALBTag_H 1

// STL includes
#include <set>
#include <string>

// FrameWork includes
#include "GaudiKernel/ServiceHandle.h"

// Local includes
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "TrkValHistUtils/PlotBase.h"
#include "BTaggingValidationPlots.h"

#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"

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
    virtual StatusCode fillHistograms(const EventContext& ctx);
    virtual StatusCode procHistograms();


    ///////////////////////////////////////////////////////////////////
    // Const methods:
    ///////////////////////////////////////////////////////////////////
    std::map<const xAOD::TrackParticle*, int> getTrackTruthAssociations(const xAOD::Jet* jet) const;

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
    ToolHandleArray<IBTaggingSelectionTool> m_cdiSelectionTools{this, "CDISelectionTools", {}, "Selection tools providing the cut values of CDITaggerName from the CDI, one per working point"};

    // isData flag
    bool m_isData;

    // Containers
    std::string m_jetNameEMTopo;
    std::string m_jetNamePFlow;
    std::string m_jetNameR10;
    std::string m_jetNameTrackJet;

    std::string m_trackName;
    std::string m_vertexName;
    std::string m_truthVertexName;
    std::string m_truthJetName;
    bool m_warnedMissingTruthPV = false;

    std::map<std::string, JetTagDQA::BTaggingValidationPlots*> m_btagplots;
    std::set<std::string> m_collectionsWithoutTrackLinks;
    
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
    bool m_useJvtProxy;
    bool m_warnedMissingNNJvt = false;
    float m_truthMatchProbabilityCut;

    std::string m_GN3XPV01Name;
    Gaudi::Property<std::map<std::string, std::string>> m_taggerDecorations{this, "TaggerDecorations", {}, "Decoration prefix of each small-R tagger, keyed by the name its histograms carry"};
    Gaudi::Property<std::map<std::string, double>> m_taggerFractionC{this, "TaggerFractionC", {}, "c-fraction of each small-R discriminant, unused where a selection tool provides it"};
    Gaudi::Property<std::map<std::string, double>> m_taggerFractionTau{this, "TaggerFractionTau", {}, "tau-fraction of each small-R discriminant, unused where a selection tool provides it"};
    Gaudi::Property<std::map<std::string, double>> m_taggerWorkingPoints{this, "TaggerWorkingPoints", {}, "Cut value of each working point, keyed by tagger and working point label as <tagger>_<wp>"};
    Gaudi::Property<std::vector<std::string>> m_cdiWorkingPoints{this, "CDIWorkingPoints", {}, "Working point labels of CDISelectionTools"};
    Gaudi::Property<std::string> m_cdiTaggerName{this, "CDITaggerName", "", "Tagger whose working points come from the CDI through CDISelectionTools"};
    Gaudi::Property<std::map<std::string, double>> m_GN3XPV01HbbFractions{this, "GN3XPV01HbbFractions", {}, "Background fractions of the GN3XPV01 Hbb discriminant, empty to disable"};
    Gaudi::Property<std::map<std::string, double>> m_GN3XPV01HccFractions{this, "GN3XPV01HccFractions", {}, "Background fractions of the GN3XPV01 Hcc discriminant, empty to disable"};

    JetTagDQA::BTaggingValidationPlots m_antiKt4EMTopoPlots;
    JetTagDQA::BTaggingValidationPlots m_antiKt4EMPFlowJetsPlots;
    JetTagDQA::BTaggingValidationPlots m_antiKt10UFOCSSKSoftDropBeta100Zcut10Jets;

    int m_nevents;

    StatusCode book(PlotBase& plots);
  };

}

#endif //> !JETTAGDQA_PHYSVALBTag_H
