/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */
 
 ///////////////////////////////////////////////////////////////////
 // NewVrtSecInclusiveAlg.cxx, (c) ATLAS Detector software
 // Author: Vadim Kostyukhin (vadim.kostyukhin@cern.ch)
 ///////////////////////////////////////////////////////////////////
#include "xAODEventInfo/EventInfo.h"
#include "NewVrtSecInclusiveTool/NewVrtSecInclusiveAlg.h"
#include "xAODTracking/VertexAuxContainer.h"

#include "TLorentzVector.h"
#include "CxxUtils/sincos.h"

namespace Rec {

   static const SG::AuxElement::Decorator<float> bvrtM("bvrtM");  
   static const SG::AuxElement::Decorator<float> bvrtPt("bvrtPt");  
   static const SG::AuxElement::Decorator<float> bvrtPhi("bvrtPhi");  
   static const SG::AuxElement::Decorator<float> bvrtEta("bvrtEta");  

   NewVrtSecInclusiveAlg::NewVrtSecInclusiveAlg(const std::string& name, ISvcLocator* pSvcLocator) :
     AthReentrantAlgorithm( name, pSvcLocator ),
     m_bvertextool("Rec::NewVrtSecInclusiveTool/SVTool",this)
   {
     declareProperty("BVertexTool",m_bvertextool);
   }

   StatusCode NewVrtSecInclusiveAlg::initialize()
   {
     ATH_CHECK( m_tpContainerKey.initialize() );
     ATH_CHECK( m_pvContainerKey.initialize() );
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
     std::vector<const xAOD::TrackParticle*> trkparticles(0);

     //-- Extract TrackParticles
     SG::ReadHandle<xAOD::TrackParticleContainer> tp_cont(m_tpContainerKey, ctx);
     if ( !tp_cont.isValid() ) {
        ATH_MSG_WARNING( "No TrackParticle container found in TES" );
     }else{
        for(const auto *tp : (*tp_cont)) trkparticles.push_back(tp);
     }

     //-- Extract Primary Vertices
     SG::ReadHandle<xAOD::VertexContainer> pv_cont(m_pvContainerKey, ctx);
     if ( !pv_cont.isValid() ) {
       ATH_MSG_WARNING( "No Primary Vertices container found in TDS" );
     }else{
       //-- Extract PV itself
       for ( const auto *v : *pv_cont ) {
         if (v->vertexType()==xAOD::VxType::PriVtx) {    pv = v;   break; }
       }
     }

     //-- create container for new vertices
     auto bVertexContainer    = std::make_unique<xAOD::VertexContainer>();
     auto bVertexAuxContainer = std::make_unique<xAOD::VertexAuxContainer>();
     bVertexContainer->setStore(bVertexAuxContainer.get());

     if( pv &&  trkparticles.size()>2 ){
       std::unique_ptr<Trk::VxSecVertexInfo> foundVrts = m_bvertextool->findAllVertices(trkparticles,*pv);      
       if(foundVrts && !foundVrts->vertices().empty()){
         const std::vector<xAOD::Vertex*> vtmp=foundVrts->vertices();
         for(const auto & iv :  vtmp) {
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
}

