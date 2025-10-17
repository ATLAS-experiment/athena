/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ****************************************************************************
// ----------------------------------------------------------------------------
// FourMuonTool
// James Catmore <James.Catmore@cern.ch>
// Evelina Bouhova-Thacker <e.bouhova@cern.ch>
// ----------------------------------------------------------------------------
// ****************************************************************************

#include "FourMuonTool.h"
#include "BPhysPVTools.h"
#include "xAODBPhys/BPhysHelper.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkV0Fitter/TrkV0VertexFitter.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "AthLinks/ElementLink.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/WriteDecorHandle.h"

#include <algorithm>

namespace DerivationFramework {

  StatusCode FourMuonTool::initialize() {

    // retrieving vertex Fitter
    ATH_CHECK( m_iVertexFitter.retrieve() );
    ATH_MSG_DEBUG("Retrieved tool " << m_iVertexFitter);

    // retrieving V0 Fitter
    ATH_CHECK( m_iV0VertexFitter.retrieve(DisableTool{!m_useV0Fitter}));

    // Get the track selector tool from ToolSvc
    ATH_CHECK ( m_trkSelector.retrieve() );
    ATH_MSG_DEBUG("Retrieved tool " << m_trkSelector);

    // Get the beam spot service
    ATH_CHECK( m_eventInfo_key.initialize() );
    ATH_CHECK(m_muonCollectionKey.initialize());
    ATH_CHECK(m_TrkParticleCollection.initialize());
    m_muonIndex = m_muonCollectionKey.key() + ".BPHY4MuonIndex";
    ATH_CHECK(m_muonIndex.initialize());
    ATH_MSG_DEBUG("Initialize successful");

    return StatusCode::SUCCESS;

  }

  FourMuonTool::FourMuonTool(const std::string& t, const std::string& n, const IInterface* p)  : AthAlgTool(t,n,p)
  {
  }

  FourMuonTool::~FourMuonTool() { }

  //-------------------------------------------------------------------------------------
  // Find the candidates
  //-------------------------------------------------------------------------------------
  StatusCode FourMuonTool::performSearch(xAOD::VertexContainer* pairVxContainer, xAOD::VertexContainer* quadVxContainer, bool &selectEvent, const EventContext& ctx) const
  {
    ATH_MSG_DEBUG( "FourMuonTool::performSearch" );
    selectEvent = false;

    // Get the muons from StoreGate
    SG::ReadHandle<xAOD::MuonContainer> importedMuonCollection(m_muonCollectionKey, ctx);
    ATH_CHECK(importedMuonCollection.isValid());
    ATH_MSG_DEBUG("Muon container size "<<importedMuonCollection->size());

    // Get ID tracks
    SG::ReadHandle<xAOD::TrackParticleContainer> importedTrackCollection(m_TrkParticleCollection, ctx);
    ATH_CHECK(importedTrackCollection.isValid());
    ATH_MSG_DEBUG("ID TrackParticle container size "<< importedTrackCollection->size());

    // Select the muons
    std::vector<const xAOD::Muon*>  theMuonsAfterSelection;
    SG::WriteDecorHandle<xAOD::MuonContainer, int> muonDecorator(m_muonIndex, ctx);
    unsigned int nCombMuons = 0;
    unsigned int nSegmentTaggedMuons = 0;

    for (const auto * muon : *importedMuonCollection) {
      if ( !muon ) continue;
      muonDecorator(*muon) = -1; // all muons must be decorated
      if (  (muon->muonType() != xAOD::Muon::Combined ) && (muon->muonType() != xAOD::Muon::SegmentTagged ) ) continue;
      if (!muon->inDetTrackParticleLink().isValid()) continue; // No muons without ID tracks
      auto& link = muon->inDetTrackParticleLink();
      const xAOD::TrackParticle* muonTrk = *link;
      if ( !muonTrk ) continue;
      const xAOD::Vertex* vx{};
      if ( !m_trkSelector->decision(*muonTrk, vx) ) continue; // all ID tracks must pass basic tracking cuts
      if ( fabs(muonTrk->pt())<m_ptCut ) continue; //  pt cut
      if ( fabs(muonTrk->eta())>m_etaCut ) continue; //  eta cut
      if ( muon->muonType() == xAOD::Muon::Combined ) ++nCombMuons;
      if ( muon->muonType() == xAOD::Muon::SegmentTagged ) ++nSegmentTaggedMuons;
      theMuonsAfterSelection.push_back(muon);
    }
    unsigned int nSelectedMuons = theMuonsAfterSelection.size();
    ATH_MSG_DEBUG("Number of muons after selection: " << nSelectedMuons);
    ATH_MSG_DEBUG("of which " << nCombMuons << " are combined");
    ATH_MSG_DEBUG("and " << nSegmentTaggedMuons << " are segment tagged");
    if ( (nSelectedMuons < 4) || (nCombMuons < 1) ) {
      ATH_MSG_DEBUG("Muon criteria not met. Skipping event.");
      return StatusCode::SUCCESS;
    }
    selectEvent = true; // if we got this far we should definitively accept the event

    // Decorators
    SG::AuxElement::Decorator< std::string > indexDecorator("CombinationCode");
    SG::AuxElement::Decorator< std::string > chargeDecorator("ChargeCode");

    // Order by pT
    std::sort(theMuonsAfterSelection.begin(), theMuonsAfterSelection.end(), [](const xAOD::Muon *a, const xAOD::Muon *b) {
      return b->pt() < a->pt();
    });

    // Decorate the selected muons (now pT ordered) with their index
    unsigned int muonIndex(0);
    for (auto selMuon : theMuonsAfterSelection) {
      muonDecorator(*selMuon) = muonIndex;
      ++muonIndex;
    }

    // Quadruplet combinatorics
    std::vector<Combination> quadruplets;
    std::vector<Combination> pairs;
    buildCombinations(theMuonsAfterSelection,pairs,quadruplets,nSelectedMuons);
    if (quadruplets.size()==0) {
      ATH_MSG_DEBUG("No acceptable quadruplets");
      return StatusCode::SUCCESS;
    }

    // Get the beam spot (for the vertexing starting point)
    SG::ReadHandle<xAOD::EventInfo> evt(m_eventInfo_key, ctx);
    ATH_CHECK(evt.isValid());
    const Amg::Vector3D beamSpot(evt->beamPosX(), evt->beamPosY(), evt->beamPosZ());

    // fit pairs
    ATH_MSG_DEBUG("Successful pairs.....");
    for (std::vector<Combination>::iterator pairItr = pairs.begin(); pairItr!=pairs.end(); ++pairItr) {
      std::vector<const xAOD::TrackParticle*> theTracks = (*pairItr).trackParticles("pair1");
      xAOD::Vertex* pairVxCandidate = fit(theTracks,importedTrackCollection.get(),beamSpot); // This line actually does the fitting and object making
      if (pairVxCandidate) {
        // decorate the candidate with its codes
        indexDecorator(*pairVxCandidate) = (*pairItr).combinationIndices();
        chargeDecorator(*pairVxCandidate) = (*pairItr).combinationCharges();
        // decorate the candidate with refitted tracks and muons via the BPhysHelper
        xAOD::BPhysHelper helper(pairVxCandidate);
        helper.setRefTrks();
        std::vector<const xAOD::Muon*> theStoredMuons;
        theStoredMuons = (*pairItr).muons;
        helper.setMuons(theStoredMuons,importedMuonCollection.get());
        // Retain the vertex
        pairVxContainer->push_back(pairVxCandidate);
        ATH_MSG_DEBUG("..... indices: " << (*pairItr).combinationIndices() <<
                      " charges: " << (*pairItr).combinationCharges() <<
                      " chi2:    " << pairVxCandidate->chiSquared());
      } else { // fit failed
        ATH_MSG_DEBUG("Fitter failed!");
      }
    }
    ATH_MSG_DEBUG("pairContainer size " << pairVxContainer->size());

    // fit quadruplets
    ATH_MSG_DEBUG("Successful quadruplets.....");
    for (std::vector<Combination>::iterator quadItr = quadruplets.begin(); quadItr!=quadruplets.end(); ++quadItr) {
      std::vector<const xAOD::TrackParticle*> theDCTracks; theDCTracks.clear();
      theDCTracks = (*quadItr).trackParticles("DC");
      xAOD::Vertex* dcVxCandidate = fit(theDCTracks,importedTrackCollection.get(), beamSpot);
      if (dcVxCandidate != 0) {
        // decorate the candidate with its codes
        indexDecorator(*dcVxCandidate) = (*quadItr).combinationIndices();
        chargeDecorator(*dcVxCandidate) = (*quadItr).combinationCharges();
        // Decorate the DC candidate with the differences between its chi2 and the other
        double dcChi2 = dcVxCandidate->chiSquared();
        // decorate the candidate with refitted tracks and muons via the BPhysHelper
        xAOD::BPhysHelper helper(dcVxCandidate);
        helper.setRefTrks();
        const std::vector<const xAOD::Muon*> &theStoredMuons = (*quadItr).muons;
        helper.setMuons(theStoredMuons,importedMuonCollection.get());
        // Retain the vertex
        quadVxContainer->push_back(dcVxCandidate);
        ATH_MSG_DEBUG("..... indices: " << (*quadItr).combinationIndices() <<
                      " charges: " << (*quadItr).combinationCharges() <<
                      " chi2(DC): " << dcChi2);
      } else { // fit failed
        ATH_MSG_DEBUG("Fitter failed!");
      }
    }
    ATH_MSG_DEBUG("quadruplet container size " << quadVxContainer->size());

    return StatusCode::SUCCESS;;
  }

  // *********************************************************************************

  // ---------------------------------------------------------------------------------
  // fit - does the fit
  // ---------------------------------------------------------------------------------

  xAOD::Vertex* FourMuonTool::fit(const std::vector<const xAOD::TrackParticle*> &inputTracks,
                                  const xAOD::TrackParticleContainer* importedTrackCollection,
                                  const Amg::Vector3D &beamSpot) const {

    const Trk::TrkV0VertexFitter* concreteVertexFitter=0;
    if (m_useV0Fitter) {
      // making a concrete fitter for the V0Fitter
      concreteVertexFitter = dynamic_cast<const Trk::TrkV0VertexFitter * >(&(*m_iV0VertexFitter));
      if(concreteVertexFitter == 0) {
        ATH_MSG_FATAL("The vertex fitter passed is not a V0 Vertex Fitter");
        return nullptr;
      }
    }

    xAOD::Vertex* myVxCandidate{};
    if (m_useV0Fitter) {
      myVxCandidate = concreteVertexFitter->fit(inputTracks, beamSpot /*vertex startingPoint*/ );
    } else {
      myVxCandidate = m_iVertexFitter->fit(inputTracks, beamSpot /*vertex startingPoint*/ );
    }

    if(myVxCandidate) BPhysPVTools::PrepareVertexLinks(myVxCandidate, importedTrackCollection);

    return myVxCandidate;

  } // End of fit method


    // *********************************************************************************

    // ---------------------------------------------------------------------------------
    // getQuadIndices: forms up index lists
    // ---------------------------------------------------------------------------------

  std::vector<std::vector<unsigned int> > FourMuonTool::getQuadIndices(unsigned int length) {

    std::vector<std::vector<unsigned int> > quadIndices = mFromN(4,length);
    return(quadIndices);
  }


  // *********************************************************************************

  // ---------------------------------------------------------------------------------
  // mFromN and combinatorics
  // ---------------------------------------------------------------------------------
  std::vector<std::vector<unsigned int> > FourMuonTool::mFromN(unsigned int m, unsigned int N) {

    std::vector<std::vector<unsigned int> > allCombinations;
    std::vector<unsigned int> mainList;
    std::vector<unsigned int> combination;
    for (unsigned int i=0; i<N; ++i) mainList.push_back(i);
    combinatorics(0,m,combination,mainList,allCombinations);
    return allCombinations;
  }

  void FourMuonTool::combinatorics(unsigned int offset,
                                   unsigned int k,
                                   std::vector<unsigned int> &combination,
                                   std::vector<unsigned int> &mainList,
                                   std::vector<std::vector<unsigned int> > &allCombinations) {
    if (k==0) {
      allCombinations.push_back(combination);
      return;
    }
    if (k>0) {
      for (unsigned int i=offset; i<=mainList.size()-k; ++i) {
        combination.push_back(mainList[i]);
        combinatorics(i+1,k-1,combination,mainList,allCombinations);
        combination.pop_back();
      }
    }
  }

  // ---------------------------------------------------------------------------------
  // getPairIndices
  // ---------------------------------------------------------------------------------

  std::vector<std::pair<unsigned int, unsigned int> > FourMuonTool::getPairIndices(unsigned int length){

    std::vector<std::pair<unsigned int, unsigned int> > uniquePairs;
    std::vector<std::vector<unsigned int> > doublets = mFromN(2,length);
    for (std::vector<std::vector<unsigned int> >::iterator it=doublets.begin(); it!=doublets.end(); ++it) {
      std::pair<unsigned int, unsigned int> tmpPair = std::make_pair((*it).at(0),(*it).at(1));
      uniquePairs.push_back(tmpPair);
    }

    return(uniquePairs);
  }



  // *********************************************************************************

  // ---------------------------------------------------------------------------------
  // buildCombinations: forms up the quadruplet of muons/tracks
  // ---------------------------------------------------------------------------------

  void FourMuonTool::buildCombinations(const std::vector<const xAOD::Muon*> &muonsIn,
                                       std::vector<Combination> &pairs,
                                       std::vector<Combination> &quadruplets,
                                       unsigned int nSelectedMuons) {

    std::vector<std::vector<unsigned int> > quadrupletIndices = getQuadIndices(nSelectedMuons);
    std::vector<std::pair<unsigned int, unsigned int> > pairIndices = getPairIndices(nSelectedMuons);

    // Quadruplets
    std::vector<std::vector<unsigned int> >::iterator quadItr;
    for (quadItr=quadrupletIndices.begin(); quadItr!=quadrupletIndices.end(); ++quadItr) {
      const std::vector<unsigned int> &quad = (*quadItr);
      std::vector<const xAOD::Muon*> theMuons = {muonsIn[quad[0]],muonsIn[quad[1]],muonsIn[quad[2]],muonsIn[quad[3]]};
      if (!passesQuadSelection(theMuons)) continue;
      Combination tmpQuad;
      tmpQuad.muons = std::move(theMuons);
      tmpQuad.quadIndices = quad;
      quadruplets.emplace_back(std::move(tmpQuad));
    }
    if (quadruplets.size() == 0) return;

    // pairs
    std::vector<std::pair<unsigned int, unsigned int> >::iterator pairItr;
    for (pairItr=pairIndices.begin(); pairItr!=pairIndices.end(); ++pairItr) {
      std::pair<unsigned int, unsigned int> pair = (*pairItr);
      Combination tmpPair;
      std::vector<const xAOD::Muon*> theMuons = {muonsIn[pair.first],muonsIn[pair.second]};
      tmpPair.muons = std::move(theMuons);
      tmpPair.pairIndices = pair;
      pairs.emplace_back(std::move(tmpPair));
    }

    return;

  }

  // *********************************************************************************

  // ---------------------------------------------------------------------------------
  // passesQuadSelection: 4-muon selection
  // ---------------------------------------------------------------------------------

  bool FourMuonTool::passesQuadSelection(const std::vector<const xAOD::Muon*> &muons) {
    bool accept(false);
    bool charges(true);
    bool quality(false);
    if ((  muons.at(0)->muonType() == xAOD::Muon::Combined ) ||
        (  muons.at(1)->muonType() == xAOD::Muon::Combined ) ||
        (  muons.at(2)->muonType() == xAOD::Muon::Combined ) ||
        (  muons.at(3)->muonType() == xAOD::Muon::Combined )
        ) quality = true;
    if (charges && quality) accept = true;
    return accept;
  }

}
