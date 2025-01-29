/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
 
 ///////////////////////////////////////////////////////////////////
 // NewVrtSecInclusiveAlg.cxx, (c) ATLAS Detector software
 // Author: Vadim Kostyukhin (vadim.kostyukhin@cern.ch)
 ///////////////////////////////////////////////////////////////////
#include "xAODEventInfo/EventInfo.h"
#include "NewVrtSecInclusiveTool/NewVrtSecInclusiveAlg.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "xAODEgamma/ElectronxAODHelpers.h"

#include "TLorentzVector.h"
#include "CxxUtils/sincos.h"

namespace Rec {

   static const SG::AuxElement::Decorator<float> bvrtM("bvrtM");  
   static const SG::AuxElement::Decorator<float> bvrtPt("bvrtPt");  
   static const SG::AuxElement::Decorator<float> bvrtPhi("bvrtPhi");  
   static const SG::AuxElement::Decorator<float> bvrtEta("bvrtEta");  
   static const SG::AuxElement::Decorator<float> mindRjetP("mindRjetP");
   static const SG::AuxElement::Decorator<float> mindRjetV("mindRjetV");
   static const SG::AuxElement::Decorator<float> mindRBTagSV("mindRBTagSV");

   NewVrtSecInclusiveAlg::NewVrtSecInclusiveAlg(const std::string& name, ISvcLocator* pSvcLocator) :
     AthReentrantAlgorithm( name, pSvcLocator )
   {
   }

   StatusCode NewVrtSecInclusiveAlg::initialize()
   {
     ATH_CHECK( m_tpContainerKey.initialize() );
     ATH_CHECK( m_gsfContainerKey.initialize() );
     ATH_CHECK( m_muonContainerKey.initialize() );
     ATH_CHECK( m_electronContainerKey.initialize() );
     ATH_CHECK( m_pvContainerKey.initialize() );
     ATH_CHECK( m_jetContainerKey.initialize() );
     ATH_CHECK( m_btsvContainerKey.initialize() );
     ATH_CHECK( m_foundVerticesKey.initialize() );
     ATH_CHECK( m_bvertextool.retrieve() );
     return StatusCode::SUCCESS;
   }

   StatusCode NewVrtSecInclusiveAlg::finalize()
   {
     return StatusCode::SUCCESS;
   }

   StatusCode NewVrtSecInclusiveAlg::execute(const EventContext &ctx) const
   {

     const xAOD::Vertex* pv = nullptr;
     std::unordered_set<const xAOD::TrackParticle*> tp_set{};
     if (m_addIDTracks) addInDetTracks(ctx, tp_set);
     if (m_addMuonTracks) addMuonTracks(ctx, tp_set);
     if (m_addGSFTracks) addGSFTracks(ctx, tp_set);
     if (m_addElectronTracks) addElectronTracks(ctx, tp_set);
     ATH_MSG_DEBUG("Found " << tp_set.size() << " useful tracks in this event" );

     std::vector<const xAOD::TrackParticle*> trkparticles(tp_set.begin(), tp_set.end());

     //-- Extract Primary Vertex
     SG::ReadHandle<xAOD::VertexContainer> pv_cont(m_pvContainerKey, ctx);
     if ( !pv_cont.isValid() ) {
       ATH_MSG_WARNING( "No Primary Vertices container found in TDS" );
     }else{
       //-- Extract PV itself
       for ( const auto *v : *pv_cont ) {
         if (v->vertexType()==xAOD::VxType::PriVtx) {    pv = v;   break; }
       }
     }

     //-- Extract SV from b-jets Vertices
     SG::ReadHandle<xAOD::VertexContainer> btsv_cont(m_btsvContainerKey, ctx);
     if ( !btsv_cont.isValid() ) {
       ATH_MSG_WARNING( "No BTagging Vertices container found in TDS" );
     }

     //-- Extract Jets
     SG::ReadHandle<xAOD::JetContainer> jet_cont(m_jetContainerKey, ctx);
     if ( !jet_cont.isValid() ) {
        ATH_MSG_WARNING( "No AntiKt4EMPFlowJet container found in TES" );
     }

     //-- create container for new vertices
     auto bVertexContainer    = std::make_unique<xAOD::VertexContainer>();
     auto bVertexAuxContainer = std::make_unique<xAOD::VertexAuxContainer>();
     bVertexContainer->setStore(bVertexAuxContainer.get());

     if( pv &&  trkparticles.size()>1 ){
       std::unique_ptr<Trk::VxSecVertexInfo> foundVrts = m_bvertextool->findAllVertices(trkparticles,*pv);      
       if(foundVrts && !foundVrts->vertices().empty()){
         const std::vector<xAOD::Vertex*> vtmp=foundVrts->vertices();
         double mindRSVPV=1.e3;  // Check coincidence with existing SV1 vertex
         for(const auto & iv :  vtmp) {
           if( btsv_cont.isValid() ){
             for ( const auto *btsv : *btsv_cont ) mindRSVPV=std::min(Amg::deltaR(btsv->position()-pv->position(),iv->position()-pv->position()),mindRSVPV);
           }
           if (m_removeNonLepVerts && vertexHasNoLep(ctx, iv)) continue;
           bVertexContainer->push_back(iv);
           std::vector< Trk::VxTrackAtVertex > & vtrk = iv->vxTrackAtVertex();
           TLorentzVector VSUM(0.,0.,0.,0.);
           TLorentzVector tmp;
           for(auto & it : vtrk){
              const Trk::Perigee* mPer = dynamic_cast<const Trk::Perigee*>(it.perigeeAtVertex());
              CxxUtils::sincos   phi(mPer->parameters()[Trk::phi]);
              CxxUtils::sincos theta(mPer->parameters()[Trk::theta]);
              double absP  =  1./std::abs(mPer->parameters()[Trk::qOverP]);
              tmp.SetXYZM( phi.cs*theta.sn*absP, phi.sn*theta.sn*absP, theta.cs*absP, Trk::ParticleMasses::mass[Trk::pion]);
              VSUM+=tmp;
           }
           bvrtM(*iv)  =VSUM.M();
           bvrtPt(*iv) =VSUM.Pt();
           bvrtEta(*iv)=VSUM.Eta();
           bvrtPhi(*iv)=VSUM.Phi();
           TVector3 SVmPV(iv->x()-pv->x(),iv->y()-pv->y(),iv->z()-pv->z());
           double mindRMOM=1.e3, mindRSV=1.e3;
           if( jet_cont.isValid() ){
             for(const auto *jet : (*jet_cont)) {
               mindRMOM=std::min(VSUM.DeltaR(jet->p4()),mindRMOM);
               mindRSV =std::min(SVmPV.DeltaR(jet->p4().Vect()),mindRSV);
             }
           }
           mindRBTagSV(*iv) =mindRSVPV;
           mindRjetP(*iv)   =mindRMOM;
           mindRjetV(*iv)   =mindRSV;
         }
       }
     }
     ATH_MSG_DEBUG("Found Vertices in this event: " << bVertexContainer->size());
     //
     //--Update track ElementLinks
     for(auto iv : (*bVertexContainer)){
       std::vector< ElementLink< xAOD::TrackParticleContainer > > newLinkVec;
       for(auto &it : iv->trackParticleLinks()){
         ElementLink< xAOD::TrackParticleContainer > tmpLnk=it;
         const xAOD::TrackParticleContainer* tp_cont = dynamic_cast<const xAOD:: TrackParticleContainer*>((*it)->container());
         tmpLnk.setStorableObject(*tp_cont);
         newLinkVec.push_back(tmpLnk);
       }
       iv->setTrackParticleLinks(newLinkVec);
     }

     SG::WriteHandle<xAOD::VertexContainer>  vrtInThisEvent(m_foundVerticesKey,ctx);
     ATH_CHECK( vrtInThisEvent.record (std::move(bVertexContainer),
                                       std::move(bVertexAuxContainer)) );
     return StatusCode::SUCCESS;
   }

   void NewVrtSecInclusiveAlg::addInDetTracks(const EventContext &ctx, std::unordered_set<const xAOD::TrackParticle*> &trkparticles) const {
     //-- Extract IDTrackParticles
     SG::ReadHandle<xAOD::TrackParticleContainer> tp_cont(m_tpContainerKey, ctx);
     if ( !tp_cont.isValid() ) {
       ATH_MSG_WARNING( "No TrackParticle container found in TES" );
       return;
     }
     trkparticles.reserve(trkparticles.size() + tp_cont->size());

     for (const auto *tp : (*tp_cont))  { trkparticles.insert(tp); }
   }

   void NewVrtSecInclusiveAlg::addMuonTracks(const EventContext &ctx, std::unordered_set<const xAOD::TrackParticle*> &trkparticles) const {
     //-- Extract Muons
     SG::ReadHandle<xAOD::MuonContainer> muon_cont(m_muonContainerKey, ctx);
     if ( !muon_cont.isValid() ) {
       ATH_MSG_WARNING( "No muon container found in TES" );
       return;
     }
     trkparticles.reserve(trkparticles.size() + muon_cont->size());

     for (const auto *muon : (*muon_cont)) {
       const auto *tp = muon->trackParticle( xAOD::Muon::InnerDetectorTrackParticle );
       if (!tp) { continue; }
       trkparticles.insert(tp);
     }
   }

   void NewVrtSecInclusiveAlg::addGSFTracks(const EventContext &ctx, std::unordered_set<const xAOD::TrackParticle*> &trkparticles) const {
     //-- Extract GSFTrackParticles
     SG::ReadHandle<xAOD::TrackParticleContainer> gsf_cont(m_gsfContainerKey, ctx);
     if ( !gsf_cont.isValid() ) {
       ATH_MSG_WARNING( "No GSF container found in TES" );
       return;
     }
     trkparticles.reserve(trkparticles.size() + gsf_cont->size());

     for (const auto *gsf : (*gsf_cont)) {
       trkparticles.insert(gsf);
       // remove the corresponding ID track from the list if it exists already.
       trkparticles.erase(xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF(gsf));
     }
   }

   void NewVrtSecInclusiveAlg::addElectronTracks(const EventContext &ctx, std::unordered_set<const xAOD::TrackParticle*> &trkparticles) const {
     //-- Extract Electrons
     SG::ReadHandle<xAOD::ElectronContainer> electron_cont(m_electronContainerKey, ctx);
     if ( !electron_cont.isValid() ) {
       ATH_MSG_WARNING( "No electron container found in TES" );
       return;
     }
     trkparticles.reserve(trkparticles.size() + electron_cont->size());

     for(const auto *electron : (*electron_cont)) {
       if( 0 == electron->nTrackParticles() ) continue;
       // the 0th GSF TP is the best matched one.
       const auto *gsf = electron->trackParticle(0);
       if (!gsf) continue;
       trkparticles.insert(gsf);
       // remove the corresponding ID track from the list if it exists already.
       trkparticles.erase(xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF(gsf));
     }
   }

   bool NewVrtSecInclusiveAlg::vertexHasNoLep(const EventContext &ctx, const xAOD::Vertex* vertex) const {
     std::unordered_set<const xAOD::TrackParticle*> trkparts{};
     trkparts.reserve(vertex->nTrackParticles());
     for (const auto &trk : vertex->trackParticleLinks()) {
       trkparts.insert(*trk);
     }

     //-- Extract Muons
     SG::ReadHandle<xAOD::MuonContainer> muon_cont(m_muonContainerKey, ctx);
     if ( muon_cont.isValid() ) {
       for (const auto *muon : (*muon_cont)) {
         const auto *tp = muon->trackParticle( xAOD::Muon::InnerDetectorTrackParticle );
         if (!tp) { continue; }
         if(trkparts.count(tp)) return false;
       }
     }

     //-- Extract Electrons
     SG::ReadHandle<xAOD::ElectronContainer> electron_cont(m_electronContainerKey, ctx);
     if ( electron_cont.isValid() ) {
       for(const auto *electron : (*electron_cont)) {
         if( 0 == electron->nTrackParticles() ) continue;
         // the 0th GSF TP is the best matched one.
         const auto *gsf = electron->trackParticle(0);
         if (!gsf) continue;
         if(trkparts.count(gsf)) return false;
         if(trkparts.count(xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF(gsf))) return false;
       }
     }
     // If both containers are missing, or no leptons are found in the vertex, return true.
     return true;
   }
}

