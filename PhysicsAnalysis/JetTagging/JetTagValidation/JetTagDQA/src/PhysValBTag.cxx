///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// PhysValBTag.cxx
// Implementation file for class PhysValBTag
// Author: E.Schopf<elisabeth.schopf@cern.ch>
// Updates: J.Hoefer <judith.hoefer@cern.ch>
///////////////////////////////////////////////////////////////////
// JetTagDQA includes
#include "PhysValBTag.h"

// STL includes
#include <vector>

// FrameWork includes
#include "GaudiKernel/IToolSvc.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingUtilities.h"
#include "AthContainers/ConstAccessor.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "ParticleJetTools/JetFlavourInfo.h"

namespace {
  // same truth jet matching as in the FTAG training dataset dumper
  bool passTruthJetMatching(const xAOD::Jet& jet, const xAOD::JetContainer& truthJets) {
    double minDR = 0.3;
    const xAOD::Jet* match = nullptr;
    for (const xAOD::Jet* truthJet : truthJets) {
      if (truthJet->pt() < 10e3) continue;
      const double dR = jet.p4().DeltaR(truthJet->p4());
      if (dR < minDR) {
        minDR = dR;
        match = truthJet;
      }
    }
    return match && match->pt() > 20e3;
  }
}

namespace JetTagDQA {

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

  // Constructors
  ////////////////

  PhysValBTag::PhysValBTag( const std::string& type,
                            const std::string& name,
                            const IInterface* parent ) :
    ManagedMonitorToolBase( type, name, parent ),
    m_isData(false),
    m_antiKt4EMTopoPlots                       (0, "BTag/AntiKt4EMTopoJets/"                ,        "antiKt4EMTopoJets"),
    m_antiKt4EMPFlowJetsPlots                  (0, "BTag/AntiKt4EMPFlowJets/"               , 	     "antiKt4EMPFlowJets"),
    m_antiKt10UFOCSSKSoftDropBeta100Zcut10Jets (0, "BTag/AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets/", "antiKt10UFOCSSKSoftDropBeta100Zcut10Jets"),
    m_nevents(0)
  {
 
    declareProperty( "isData", m_isData );

    declareProperty( "JetContainerEMTopo", m_jetNameEMTopo = "AntiKt4EMTopoJets" );
    declareProperty( "JetContainerPFlow", m_jetNamePFlow = "AntiKt4EMPFlowJets");
    declareProperty( "JetContainerR10", m_jetNameR10 = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets");

    declareProperty( "TrackContainerName", m_trackName = "InDetTrackParticles" );
    declareProperty( "VertexContainerName", m_vertexName = "PrimaryVertices" );
    declareProperty( "TruthPrimaryVertexContainerName", m_truthVertexName = "TruthPrimaryVertices" );
    declareProperty( "TruthJetContainerName", m_truthJetName = "AntiKt4TruthJets" );

    declareProperty( "OnZprime", m_onZprime );
    declareProperty( "JetPtCutTtbar", m_jetPtCutTtbar = 20000);
    declareProperty( "JetPtCutZprime", m_jetPtCutZprime = 400000);
    declareProperty( "JetPtCutR10", m_jetPtCutR10 = 200000); //pT>200 GeV for large-R jets
    declareProperty( "JetEtaCut", m_jetEtaCut = 2.5);
    declareProperty( "UseJvtProxy", m_useJvtProxy = false);
    declareProperty( "truthMatchProbabilityCut", m_truthMatchProbabilityCut = 0.75);

    declareProperty( "GN3XPV01TaggerName", m_GN3XPV01Name = "GN3XPV01");

  }

  // Destructor
  ///////////////
  PhysValBTag::~PhysValBTag()
  {}

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode PhysValBTag::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");
    ATH_CHECK(ManagedMonitorToolBase::initialize());

    // initialize the truth-track-assoiation tool
    ATH_CHECK(m_trackTruthOriginTool.retrieve( EnableTool {true} ));

    m_jetPtCut = m_onZprime ? m_jetPtCutZprime : m_jetPtCutTtbar;

    ATH_CHECK(m_GN2v01SelectionTools.retrieve());
    if (m_GN2v01SelectionTools.size() != m_GN2v01WorkingPoints.size()) {
      ATH_MSG_ERROR("GN2v01SelectionTools and GN2v01WorkingPoints need to have the same length");
      return StatusCode::FAILURE;
    }
    std::map<std::string, double> GN2v01WorkingPoints;
    for (std::size_t i = 0; i < m_GN2v01SelectionTools.size(); ++i) {
      double cut = 0;
      if (m_GN2v01SelectionTools[i]->getCutValue(0., cut) != CP::CorrectionCode::Ok) {
        ATH_MSG_ERROR("Cannot get the GN2v01 cut value for working point " << m_GN2v01WorkingPoints[i]);
        return StatusCode::FAILURE;
      }
      GN2v01WorkingPoints.emplace(m_GN2v01WorkingPoints[i], cut);
    }

    // convert the HistogramDefinitions vector to a map 
    for(unsigned int i = 0; i < m_HistogramDefinitionsVector.size(); i++){
      std::string name = m_HistogramDefinitionsVector[i][0];
      m_HistogramDefinitionsMap.insert(std::pair< std::string, std::vector< std::string > >(name, m_HistogramDefinitionsVector[i]));
    }
    if(!m_jetNameEMTopo.empty()){
    m_btagplots.insert(std::make_pair(m_jetNameEMTopo, &m_antiKt4EMTopoPlots));
    }
    m_btagplots.insert(std::make_pair(m_jetNamePFlow, &m_antiKt4EMPFlowJetsPlots));
    m_btagplots.insert(std::make_pair(m_jetNameR10, &m_antiKt10UFOCSSKSoftDropBeta100Zcut10Jets));

    std::map<std::string, std::map<std::string, double>> workingPoints;
    for (const auto& [key, cut] : m_taggerWorkingPoints) {
      const std::size_t split = key.rfind('_');
      if (split == std::string::npos) {
        ATH_MSG_ERROR("TaggerWorkingPoints key " << key << " is not of the form <tagger>_<working point>");
        return StatusCode::FAILURE;
      }
      workingPoints[key.substr(0, split)].emplace(key.substr(split + 1), cut);
    }
    workingPoints["GN2v01"] = GN2v01WorkingPoints;

    for(const auto& [name, plot]: m_btagplots){
      plot->setDetailLevel(m_detailLevel);
      plot->setHistogramDefinitions(m_HistogramDefinitionsMap);
      plot->setIsDataAndTMPCut(m_isData, m_truthMatchProbabilityCut);
      for (const auto& [tagger, decoration] : m_taggerDecorations) {
        const bool fromCDI = tagger == "GN2v01" && !m_GN2v01SelectionTools.empty();
        plot->addSmallRTagger(tagger, decoration,
                              m_taggerFractionC.value().count(tagger) ? m_taggerFractionC.value().at(tagger) : 0.,
                              m_taggerFractionTau.value().count(tagger) ? m_taggerFractionTau.value().at(tagger) : 0.,
                              workingPoints.count(tagger) ? workingPoints.at(tagger) : std::map<std::string, double>{},
                              fromCDI ? m_GN2v01SelectionTools[0].get() : nullptr);
      }
      if (!plot->setGN3XPV01Config(m_GN3XPV01Name, m_GN3XPV01HbbFractions, m_GN3XPV01HccFractions)) return StatusCode::FAILURE;
      plot->setIsLargeR(name == m_jetNameR10);
    }
   
    return StatusCode::SUCCESS;
  }

  StatusCode PhysValBTag::book(PlotBase& plots)
  {
    plots.initialize();
    std::vector<HistData> hists = plots.retrieveBookedHistograms();

    for (auto& hist : hists){
      ATH_MSG_DEBUG ("Initializing " << hist.first << " " << hist.first->GetName() << " " << hist.second << "...");
      ATH_CHECK(regHist(hist.first,hist.second,all));
    }
    return StatusCode::SUCCESS;
  }

  StatusCode PhysValBTag::bookHistograms()
  {
    ATH_MSG_INFO ("Booking hists " << name() << "...");

    if (m_detailLevel < 10) return StatusCode::SUCCESS;

    for(const auto& [name, plot] : m_btagplots ){
      ATH_CHECK(book(*plot));
    }
      
    return StatusCode::SUCCESS;
  }

  StatusCode PhysValBTag::fillHistograms(const EventContext& /*ctx*/)
  {
    ATH_MSG_DEBUG ("Filling hists " << name() << "...");
    
    if (m_detailLevel < 10) return StatusCode::SUCCESS;
    
    ++m_nevents;
    //std::cout << "Number of proccessed events = " << m_nevents << std::endl;

    // event info
    const xAOD::EventInfo* event(0);
    ATH_CHECK(evtStore()->retrieve(event, "EventInfo"));

    // get the primary vertex
    const xAOD::VertexContainer *vertices = 0;
    CHECK( evtStore()->retrieve(vertices, m_vertexName) );
    int npv(0);
    size_t indexPV = 0;
    bool has_pv = false;
    xAOD::VertexContainer::const_iterator vtx_itr = vertices->begin();
    xAOD::VertexContainer::const_iterator vtx_end = vertices->end();
    int count = -1;
    double PV_x = -999.;
    double PV_y = -999.;
    double PV_z = -999.;

    // loop over the vertices
    for (; vtx_itr != vtx_end; ++vtx_itr) {
      count++;
      if ((*vtx_itr)->nTrackParticles() >= 2) {
        npv++;
        if ((*vtx_itr)->vertexType() == 1) {
          if (has_pv) ATH_MSG_WARNING( ".... second PV in the events ...!!!!!!");
          indexPV = count;
          has_pv = true;
          PV_x = (*vtx_itr)->x();
          PV_y = (*vtx_itr)->y();
          PV_z = (*vtx_itr)->z();
        }
      }
    }
    if (!has_pv) {
      //ATH_MSG_WARNING( ".... rejecting the event due to missing PV!!!!");
      return StatusCode::SUCCESS;
    }
    const xAOD::Vertex *myVertex = vertices->at(indexPV); // the (reco?) primary vertex
    //std::cout<<"z coordinate of PV: "<< myVertex->z() <<std::endl;

    const xAOD::TruthVertex* truthPV = nullptr;
    if (!m_isData) {
      const xAOD::TruthVertexContainer* truthVertices = nullptr;
      if (m_useJvtProxy || evtStore()->contains<xAOD::TruthVertexContainer>(m_truthVertexName)) ATH_CHECK(evtStore()->retrieve(truthVertices, m_truthVertexName));
      if (truthVertices && !truthVertices->empty()) truthPV = truthVertices->at(0);
      else if (!m_warnedMissingTruthPV) {
        ATH_MSG_WARNING("No " << m_truthVertexName << ", truth Lxy is measured from the detector origin");
        m_warnedMissingTruthPV = true;
      }
    }

    // get the tracks
    const xAOD::TrackParticleContainer* tracks(0);
    ATH_CHECK(evtStore()->retrieve(tracks, m_trackName));

    // truth based JVT proxy: reconstructed PV close to the truth PV and jets matched to truth jets
    bool passTruthPV = true;
    const xAOD::JetContainer* truthJets = nullptr;
    if (m_useJvtProxy) {
      ATH_CHECK(evtStore()->retrieve(truthJets, m_truthJetName));
      passTruthPV = truthPV && std::abs(myVertex->z() - truthPV->z()) < 0.1;
    }

    // loop over the jet collections
    for(const auto& [name, plot] : m_btagplots){
      
      // get the jets
      const xAOD::JetContainer* jets(0);
      StatusCode jetException = evtStore()->retrieve(jets, name);
      // If StatusCode is not SUCCESS, no jet collection with this name, continue loop
      if (jetException != StatusCode::SUCCESS) continue;

      int nJets_withCut = 0;
      int nJets_containing_muon = 0; 
      int nJets_containing_SV = 0; 
      std::map<std::string, int> nJetsThatPassedWPCuts;
      plot->initializeNJetsThatPassedWPCutsMap(nJetsThatPassedWPCuts);

      float ptCut = (name==m_jetNameR10) ? m_jetPtCutR10 : m_jetPtCut;
      std::string label_name = "HadronConeExclTruthLabelID";
      if(name==m_jetNameR10) label_name = "R10TruthLabel_R22v1";
      // loop over the jets
      for (auto jet : *jets) {

        // apply the jet pT eta and jvt cuts
        if(jet->pt() <= ptCut) continue;
        if(std::abs(jet->eta()) >= m_jetEtaCut) continue;
        // JVT selection, not applied to large-R jets
        if(name!=m_jetNameR10){
          if (m_useJvtProxy) {
            if (!passTruthPV || !passTruthJetMatching(*jet, *truthJets)) continue;
          }
          else if (name == m_jetNamePFlow) {
            static const SG::ConstAccessor<char> NNJvtPassAcc("NNJvtPass");
            if (!NNJvtPassAcc.isAvailable(*jet)) {
              if (!m_warnedMissingNNJvt) {
                ATH_MSG_WARNING("No NNJvtPass on " << name << ", no JVT cut applied");
                m_warnedMissingNNJvt = true;
              }
            }
            else if (!NNJvtPassAcc(*jet)) continue;
          }
        }

        // count the jets that pass the cuts
        nJets_withCut++;

        // get the jet truth label
        int truth_label(1000);
        if(!m_isData){
	        SG::ConstAccessor<int> acc(label_name);
	        if(acc.isAvailable(*jet)) jet->getAttribute(label_name, truth_label);
        }

        // fill the jet related histograms
        plot->fillJetKinVars(jet, truth_label, m_onZprime, event);

        // fill the jet, btag & vertex related plots
        if (name!=m_jetNameR10){ //small-R jets

          // fill other variables
          bool contains_muon;
          double jet_Lxy = -1;
          plot->fillOther(jet, contains_muon, jet_Lxy, truth_label, truthPV, event);
          if(contains_muon) nJets_containing_muon++;

          static const SG::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer> > >
            trackLinksAcc("TracksForBTagging");
          if (!trackLinksAcc.isAvailable(*jet)) {
            if (m_collectionsWithoutTrackLinks.insert(name).second) {
              ATH_MSG_WARNING("No TracksForBTagging on " << name << ", skipping track, SV and tagger histograms");
            }
            continue;
          }

          // get the track to truth associations
          std::map<const xAOD::TrackParticle*, int> track_truth_associations = getTrackTruthAssociations(jet);

          // fill track related variables
          int num_HF_tracks_in_jet;
          plot->fillTrackVariables(jet, myVertex, track_truth_associations, contains_muon, truth_label, num_HF_tracks_in_jet, event);
          // fill SV related vars
          bool contains_SV;
          plot->fillSVVariables(jet, track_truth_associations, contains_muon, truth_label, num_HF_tracks_in_jet, contains_SV, event);
          if(contains_SV) nJets_containing_SV++;
          // fill discriminant related vars
          plot->fillDiscriminantVariables(jet, jet_Lxy, truth_label, m_onZprime, nJetsThatPassedWPCuts, event);
        }
        else if (jet && name==m_jetNameR10){ // large-R jets
          //fill track and hit information
          plot->fillTrackVariables_for_largeRjet(jet, myVertex, truth_label, event);
          // fill discriminant related vars
          plot->fillDiscriminantVariables_for_largeRjet(jet, truth_label, event);
        }
        else{
          ATH_MSG_WARNING("jet is a null pointer.");
        }
      }

      // fill multiplicities
      plot->fillMultiplicities(nJets_withCut, tracks->size(), npv, myVertex->nTrackParticles(), nJets_containing_muon, nJets_containing_SV, nJetsThatPassedWPCuts, event);
      // fill PV variables
      plot->fillPVVariables(PV_x, PV_y, PV_z, event);

    }

    return StatusCode::SUCCESS;
  }

  StatusCode PhysValBTag::procHistograms()
  {
    ATH_MSG_INFO ("Finalising hists " << name() << "...");

    for(const auto& [name, plot] : m_btagplots ){
      plot->finalize();

    }
    return StatusCode::SUCCESS;
  }

  ///////////////////////////////////////////////////////////////////
  // Const methods:
  ///////////////////////////////////////////////////////////////////

  std::map<const xAOD::TrackParticle*, int> PhysValBTag::getTrackTruthAssociations(const xAOD::Jet* jet) const {

    // define the return vector
    std::map<const xAOD::TrackParticle*, int> truthValues;

    // get the track links from the jet
    static const SG::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer> > >
      JetToTrackAssociatorAcc("TracksForBTagging");
    std::vector< ElementLink< xAOD::IParticleContainer > > assocTracks =
      JetToTrackAssociatorAcc(*jet);

    // loop over the tracks associated to the jet and get the truth values
    for(unsigned int i = 0; i < assocTracks.size(); i++) {
      if (!assocTracks.at(i).isValid()) continue;

      // get the curent track
      const xAOD::TrackParticle* track = static_cast<const xAOD::TrackParticle*>(*(assocTracks.at(i)));  

      // only try accessing the truth values if not on data
      int origin = 0;
      if(!m_isData){
        origin = m_trackTruthOriginTool->getTrackOrigin(track);
      }

      // add the truth values to the vector
      truthValues.insert( std::make_pair( track, origin ) );
    }

    // return
    return truthValues;
  }


}

//  LocalWords:  str
