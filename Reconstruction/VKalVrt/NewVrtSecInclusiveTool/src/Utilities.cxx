/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
///
///    @author Vadim Kostyukhin <vadim.kostyukhin@cern.ch>
///
// Header include
#include "NewVrtSecInclusiveTool/NewVrtSecInclusiveTool.h"
#include "TrkNeutralParameters/NeutralParameters.h"
#include "TrkTrackSummary/TrackSummary.h"
#include  "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "AthContainers/ConstAccessor.h"
#include "CxxUtils/sincos.h"
//-------------------------------------------------
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/Layer.h"
#include "TruthUtils/MagicNumbers.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
 // Other stuff
#include <cmath>


namespace Rec{  


  void NewVrtSecInclusiveTool::printWrkSet(const std::vector<WrkVrt> *WrkVrtSet, const std::string &name) const {
    int nGoodV=0;
    if(msgLvl(MSG::INFO)){
    for(int iv=0; iv<(int)WrkVrtSet->size(); iv++) {
      msg(MSG::INFO)<<name
      <<"= "<<(*WrkVrtSet)[iv].vertex[0]
      <<", "<<(*WrkVrtSet)[iv].vertex[1]
      <<", "<<(*WrkVrtSet)[iv].vertex[2]
      <<" NTrk="<<(*WrkVrtSet)[iv].selTrk.size()
      <<" is good="<<std::boolalpha<<(*WrkVrtSet)[iv].Good<<std::noboolalpha
      <<"  Chi2="<<(*WrkVrtSet)[iv].chi2
      <<"  Mass="<<(*WrkVrtSet)[iv].vertexMom.M()
      <<"  detached="<<(*WrkVrtSet)[iv].detachedTrack
      <<"  proj.dist="<<(*WrkVrtSet)[iv].projectedVrt
      <<" trk=";
      for(int kk=0; kk<(int)(*WrkVrtSet)[iv].selTrk.size(); kk++) {
                msg(MSG::INFO)<<", "<<(*WrkVrtSet)[iv].selTrk[kk];}
      for(int kk=0; kk<(int)(*WrkVrtSet)[iv].selTrk.size(); kk++) {
                msg(MSG::INFO)<<", "<<momAtVrt((*WrkVrtSet)[iv].trkAtVrt[kk]).Perp();}
      msg(MSG::INFO)<<endmsg;
      if((*WrkVrtSet)[iv].Good)nGoodV++;
    }
    msg(MSG::INFO)<<name<<" N="<<nGoodV<<endmsg; 
    }
  }

               /*  Technicalities */
  double NewVrtSecInclusiveTool::projSV_PV(const Amg::Vector3D & SV, const xAOD::Vertex & PV, const TLorentzVector & Direction) 
  {  
     TVector3 SV_PV( SV.x()-PV.x(), SV.y()-PV.y(), SV.z()-PV.z() );
     return Direction.Vect().Unit()*SV_PV.Unit();
  }

  double NewVrtSecInclusiveTool::vrtVrtDist(const xAOD::Vertex & primVrt, const Amg::Vector3D & secVrt, 
                                          const std::vector<double>& secVrtErr, double& signif)
  
  {
    double distx =  primVrt.x()- secVrt.x();
    double disty =  primVrt.y()- secVrt.y();
    double distz =  primVrt.z()- secVrt.z();


    AmgSymMatrix(3)  primCovMtx=primVrt.covariancePosition();  //Create
    primCovMtx(0,0) += secVrtErr[0];
    primCovMtx(0,1) += secVrtErr[1];
    primCovMtx(1,0) += secVrtErr[1];
    primCovMtx(1,1) += secVrtErr[2];
    primCovMtx(0,2) += secVrtErr[3];
    primCovMtx(2,0) += secVrtErr[3];
    primCovMtx(1,2) += secVrtErr[4];
    primCovMtx(2,1) += secVrtErr[4];
    primCovMtx(2,2) += secVrtErr[5];

    AmgSymMatrix(3)  wgtMtx = primCovMtx.inverse();

    signif = distx*wgtMtx(0,0)*distx
            +disty*wgtMtx(1,1)*disty
            +distz*wgtMtx(2,2)*distz
         +2.*distx*wgtMtx(0,1)*disty
         +2.*distx*wgtMtx(0,2)*distz
         +2.*disty*wgtMtx(1,2)*distz;
    signif=std::sqrt(std::abs(signif));
    if( signif!=signif ) signif = 0.;
    return std::sqrt(distx*distx+disty*disty+distz*distz);
  }

  double NewVrtSecInclusiveTool::vrtVrtDist2D(const xAOD::Vertex & primVrt, const Amg::Vector3D & secVrt, 
                                          const std::vector<double>& secVrtErr, double& signif)
  
  {
    double distx =  primVrt.x()- secVrt.x();
    double disty =  primVrt.y()- secVrt.y();


    AmgSymMatrix(3)  primCovMtx=primVrt.covariancePosition();  //Create
    AmgSymMatrix(2)  covMtx;
    covMtx(0,0) = primCovMtx(0,0) + secVrtErr[0];
    covMtx(0,1) = primCovMtx(0,1) + secVrtErr[1];
    covMtx(1,0) = primCovMtx(1,0) + secVrtErr[1];
    covMtx(1,1) = primCovMtx(1,1) + secVrtErr[2];

    AmgSymMatrix(2)  wgtMtx = covMtx.inverse();

    signif = distx*wgtMtx(0,0)*distx
            +disty*wgtMtx(1,1)*disty
         +2.*distx*wgtMtx(0,1)*disty;
    signif=std::sqrt(std::abs(signif));
    if( signif!=signif ) signif = 0.;
    return std::sqrt(distx*distx+disty*disty);
  }


  double NewVrtSecInclusiveTool::vrtVrtDist(const Amg::Vector3D & vrt1, const std::vector<double>  & vrtErr1,
                                            const Amg::Vector3D & vrt2, const std::vector<double>  & vrtErr2)
  
  {
    double distx =  vrt1.x()- vrt2.x();
    double disty =  vrt1.y()- vrt2.y();
    double distz =  vrt1.z()- vrt2.z();

    AmgSymMatrix(3)  primCovMtx;  //Create
    primCovMtx(0,0) =                   vrtErr1[0]+vrtErr2[0];
    primCovMtx(0,1) = primCovMtx(1,0) = vrtErr1[1]+vrtErr2[1];
    primCovMtx(1,1) =                   vrtErr1[2]+vrtErr2[2];
    primCovMtx(0,2) = primCovMtx(2,0) = vrtErr1[3]+vrtErr2[3];
    primCovMtx(1,2) = primCovMtx(2,1) = vrtErr1[4]+vrtErr2[4];
    primCovMtx(2,2) =                   vrtErr1[5]+vrtErr2[5];

    AmgSymMatrix(3)  wgtMtx = primCovMtx.inverse();

    double signif = 
               distx*wgtMtx(0,0)*distx
              +disty*wgtMtx(1,1)*disty
              +distz*wgtMtx(2,2)*distz
           +2.*distx*wgtMtx(0,1)*disty
           +2.*distx*wgtMtx(0,2)*distz
           +2.*disty*wgtMtx(1,2)*distz;
    signif=std::sqrt(std::abs(signif));
    if(signif != signif)  signif = 0.;
    return signif;
  }
//
  double NewVrtSecInclusiveTool::PntPntDist(const Amg::Vector3D & Vrt1, const Amg::Vector3D & Vrt2) 
  { 
    double dx =  Vrt1.x()- Vrt2.x();
    double dy =  Vrt1.y()- Vrt2.y();
    double dz =  Vrt1.z()- Vrt2.z();
    return std::sqrt(dx*dx+dy*dy*dz*dz);
  }

//----------------------------
//   Vertex error along radius
//----------------------------
  double NewVrtSecInclusiveTool::vrtRadiusError(const Amg::Vector3D & SecVrt, const std::vector<double>  & VrtErr) 
  {
    double DirX=SecVrt.x(), DirY=SecVrt.y(); 
    double Covar =    DirX*VrtErr[0]*DirX
                  +2.*DirX*VrtErr[1]*DirY
                     +DirY*VrtErr[2]*DirY;
    Covar /= DirX*DirX + DirY*DirY;
    Covar=std::sqrt(std::abs(Covar));
    if(Covar != Covar)  Covar = 0.;
    return Covar;
  }


    /* Invariant mass calculation for V0 decays*/
    /* Gives correct mass assignment in case of nonequal masses*/


   double NewVrtSecInclusiveTool::massV0(const std::vector< std::vector<double> >& TrkAtVrt,
                               double massP, double massPi )
   
   {
        double ap1i=std::abs(TrkAtVrt[0][2]); double ap2i=std::abs(TrkAtVrt[1][2]);
        CxxUtils::sincos   phi1(TrkAtVrt[0][0]);
        CxxUtils::sincos theta1(TrkAtVrt[0][1]);
        CxxUtils::sincos   phi2(TrkAtVrt[1][0]);
        CxxUtils::sincos theta2(TrkAtVrt[1][1]);
        double px = phi1.cs * theta1.sn * ap1i 
                  + phi2.cs * theta2.sn * ap2i;
        double py = phi1.sn * theta1.sn * ap1i 
                  + phi2.sn * theta2.sn * ap2i;
        double pz =           theta1.cs * ap1i 
                  +           theta2.cs * ap2i;
        double ee= (ap1i > ap2i) ? 
            (std::sqrt(ap1i*ap1i+massP*massP)+std::sqrt(ap2i*ap2i+massPi*massPi)):
            (std::sqrt(ap2i*ap2i+massP*massP)+std::sqrt(ap1i*ap1i+massPi*massPi));
        double test=(ee-pz)*(ee+pz)-px*px-py*py;
        return test>0 ? std::sqrt(test) : 0.; 
    }



  TLorentzVector NewVrtSecInclusiveTool::momAtVrt(const std::vector< double >& inpTrk) 
  const
  {
     double api=1./std::abs(inpTrk[2]);
     CxxUtils::sincos   phi(inpTrk[0]);
     CxxUtils::sincos theta(inpTrk[1]);
     double px = phi.cs  * theta.sn * api;
     double py = phi.sn  * theta.sn * api;
     double pz =           theta.cs * api;
     double ee = std::sqrt( px*px + py*py + pz*pz + m_massPi*m_massPi);
     return {px,py,pz,ee}; 
   }


/*************************************************************************************************************/
  int   NewVrtSecInclusiveTool::getIBLHit(const xAOD::TrackParticle* Part) 
  {
        uint8_t IBLhit,IBLexp;
        if(!Part->summaryValue( IBLexp,  xAOD::expectInnermostPixelLayerHit) )           IBLexp = 0;
        if( IBLexp==0 ) return -1;
        if(!Part->summaryValue( IBLhit,  xAOD::numberOfInnermostPixelLayerHits) )        IBLhit = 0;
        if(IBLhit) return 1;
        else       return 0;
  }
  int   NewVrtSecInclusiveTool::getBLHit(const xAOD::TrackParticle* Part) 
  {
        uint8_t BLhit,BLexp;
        if(!Part->summaryValue( BLexp,  xAOD::expectNextToInnermostPixelLayerHit) )           BLexp = 0;
        if( BLexp==0 ) return -1;
        if(!Part->summaryValue( BLhit,  xAOD::numberOfNextToInnermostPixelLayerHits) )        BLhit = 0;
        if(BLhit) return 1;
        else      return 0;
  }

  void   NewVrtSecInclusiveTool::getPixelDiscs(const xAOD::TrackParticle* Part, int &d0Hit, int &d1Hit, int &d2Hit) 
  {
        uint32_t HitPattern=Part->hitPattern();
        d0Hit=0; if( HitPattern&((1<<Trk::pixelEndCap0)) ) d0Hit=1;
        d1Hit=0; if( HitPattern&((1<<Trk::pixelEndCap1)) ) d1Hit=1;
        d2Hit=0; if( HitPattern&((1<<Trk::pixelEndCap2)) ) d2Hit=1;
  }
/*************************************************************************************************************/

  int NewVrtSecInclusiveTool::getIdHF(const xAOD::TrackParticle* TP ) const {
    static const SG::ConstAccessor< ElementLink< xAOD::TruthParticleContainer> >
      truthParticleLinkAcc ( "truthParticleLink");
    if( truthParticleLinkAcc.isAvailable(*TP) ) {
       const ElementLink<xAOD::TruthParticleContainer>& tplink = truthParticleLinkAcc(*TP);
       if( !tplink.isValid() ) return 0;
       static const SG::ConstAccessor< float >truthMatchProbabilityAcc ( "truthMatchProbability" );
       if( truthMatchProbabilityAcc( *TP ) < 0.75 ) return 0;
       if( HepMC::is_simulation_particle((*tplink))) return 0;
       //-------------
       const xAOD::TruthParticle * parent1=getPreviousParent(*tplink);
       if(!parent1 || !parent1->isHadron()) return 0;
       if(parent1->isBottomHadron()) return isExcitedHadron(parent1) ? 0 : 1;  //Ground state B-hadron
       if(parent1->isCharmHadron() && !isExcitedHadron(parent1))return 1;      //Ground state C-hadron
       //--------------
       const xAOD::TruthParticle * parent2=getPreviousParent(parent1);
       if(!parent2 || !parent2->isHadron()) return 0;
       if(parent2->isBottomHadron()) return isExcitedHadron(parent2) ? 0 : 1;  //Ground state B-hadron
       if(parent2->isCharmHadron() && !isExcitedHadron(parent2))return 1;      //Ground state C-hadron
       //--------------
       const xAOD::TruthParticle * parent3=getPreviousParent(parent2);
       if(!parent3 || !parent3->isHadron()) return 0;
       if(parent3->isBottomHadron()) return isExcitedHadron(parent3) ? 0 : 1;  //Ground state B-hadron
       if(parent3->isCharmHadron() && !isExcitedHadron(parent3))return 1;      //Ground state C-hadron
    }
    return 0;
  }
  const xAOD::TruthParticle * NewVrtSecInclusiveTool::getPreviousParent(const xAOD::TruthParticle * child) {
    if( child->hasProdVtx() ){
       if( child->prodVtx()->nIncomingParticles()==1 ){
            return *(child->prodVtx()->incomingParticleLinks())[0];
       }
    }
    return nullptr;
  }
  bool NewVrtSecInclusiveTool::isExcitedHadron(const xAOD::TruthParticle * tp) {
    if( !tp->hasProdVtx() )return false;
    if( !tp->hasDecayVtx() )return false;
    Amg::Vector3D pvrt(tp->prodVtx()->x(),tp->prodVtx()->y(),tp->prodVtx()->z());
    Amg::Vector3D dvrt(tp->decayVtx()->x(),tp->decayVtx()->y(),tp->decayVtx()->z());
    return Amg::distance(pvrt,dvrt) < 0.001*Gaudi::Units::mm ? true : false;
  }

  bool NewVrtSecInclusiveTool::isDisplaced(const xAOD::TrackParticle * TP) {
     if(!TP)return false;
     static const SG::ConstAccessor<ElementLink<xAOD::TruthParticleContainer>> truthParticleLinkAcc( "truthParticleLink");
     if( truthParticleLinkAcc.isAvailable( *TP ) ) {
       const ElementLink<xAOD::TruthParticleContainer>& tplink = truthParticleLinkAcc( *TP );
       if( !tplink.isValid() ) return false;
       if((*tplink)->hasProdVtx() && (*tplink)->prodVtx()->perp() > 1.*Gaudi::Units::mm) return true;
     }
     return false;
  }

  int NewVrtSecInclusiveTool::getG4Inter(const xAOD::TrackParticle* TP ) {
      static const SG::ConstAccessor< ElementLink< xAOD::TruthParticleContainer> >
        truthParticleLinkAcc ( "truthParticleLink");
      if( truthParticleLinkAcc.isAvailable( *TP ) ) {
        const ElementLink<xAOD::TruthParticleContainer>& tplink = 
          truthParticleLinkAcc( *TP );
        if( tplink.isValid() && HepMC::is_simulation_particle((*tplink))) return 1;
      }
      return 0;
  }
  int NewVrtSecInclusiveTool::getMCPileup(const xAOD::TrackParticle* TP ) {
      static const SG::ConstAccessor< ElementLink< xAOD::TruthParticleContainer> >
        truthParticleLinkAcc ( "truthParticleLink");
      if( truthParticleLinkAcc.isAvailable( *TP ) ) {
        const ElementLink<xAOD::TruthParticleContainer>& tplink = 
          truthParticleLinkAcc( *TP );
        if( !tplink.isValid() ) return 1;
      } else { return 1; }
      return 0;
  }

  int NewVrtSecInclusiveTool::getProdVrtBarcode(const xAOD::TrackParticle * TP, float resolLimit)
  {
     int barVrt=0;
     if(!TP)return barVrt;
     static const SG::ConstAccessor<ElementLink<xAOD::TruthParticleContainer>> truthParticleLinkAcc( "truthParticleLink");
     if( truthParticleLinkAcc.isAvailable( *TP ) ) {
       barVrt=-1;
       const ElementLink<xAOD::TruthParticleContainer>& tplink = truthParticleLinkAcc( *TP );
       if( !tplink.isValid() ) return barVrt;
       const xAOD::TruthParticle *tparticle = (*tplink);         // truth particle for TrackPartile
       if(tparticle->hasProdVtx()){                              // truth particle has production vertex
          barVrt = tparticle->prodVtx()->barcode();
          if(HepMC::is_simulation_vertex(tparticle->prodVtx()))      return barVrt;          // Geant4 vertex
	  if(tparticle->prodVtx()->nIncomingParticles()!=1) return barVrt;           // Safety!
	  const xAOD::TruthParticle *parent = *(tparticle->prodVtx()->incomingParticleLinks())[0];
	  Amg::Vector3D vpos0(tparticle->prodVtx()->x(),tparticle->prodVtx()->y(),tparticle->prodVtx()->z()); //Truth vertex position
          //
	  //--  Now check parent
	  if(!parent->isHadron()) return barVrt;    // Parent is parton not particle! Stop.
          if(!parent->hasProdVtx())         return barVrt;    // Parent particle doesn't have production vertex
	  Amg::Vector3D vpos1(parent->prodVtx()->x(),parent->prodVtx()->y(),parent->prodVtx()->z()); //Truth vertex position
          if( Amg::distance(vpos0,vpos1) > resolLimit) return barVrt;    // Parent vertex is far
          barVrt = parent->prodVtx()->barcode();                         // Else use parent vertex as reference
          if(HepMC::is_simulation_vertex(parent->prodVtx()))   return barVrt;     // Geant4 vertex
	  if(parent->prodVtx()->nIncomingParticles()!=1)       return barVrt;     // Not a decay vertex
	  const xAOD::TruthParticle *grandparent = *(parent->prodVtx()->incomingParticleLinks())[0];
          //
	  //--  Now check grandparent
	  if(!grandparent->isHadron()) return barVrt;    // Grandparent is parton not particle! Stop.
          if(!grandparent->hasProdVtx())         return barVrt;    // Parent particle doesn't have production vertex
	  Amg::Vector3D vpos2(grandparent->prodVtx()->x(),grandparent->prodVtx()->y(),grandparent->prodVtx()->z()); //Truth vertex position
          if( Amg::distance(vpos0,vpos2) > resolLimit) return barVrt;    // Grandparent vertex is far
          barVrt = grandparent->prodVtx()->barcode();                    // Use grandparent vertex as reference
          if(HepMC::is_simulation_vertex(grandparent->prodVtx()))      return barVrt;          // Geant4 vertex
	  if(grandparent->prodVtx()->nIncomingParticles()!=1) return barVrt;           // Not a decay vertex
	  const xAOD::TruthParticle *biggrandparent = *(grandparent->prodVtx()->incomingParticleLinks())[0];
          //
	  //--  Now check biggrandparent
	  if(!biggrandparent->isHadron()) return barVrt;    // BigGrandparent is parton not particle! Stop.
          if(!biggrandparent->hasProdVtx())         return barVrt;    // Parent particle doesn't have production vertex
	  Amg::Vector3D vpos3(biggrandparent->prodVtx()->x(),biggrandparent->prodVtx()->y(),biggrandparent->prodVtx()->z());
          if( Amg::distance(vpos0,vpos3) > resolLimit) return barVrt;    // Grandparent vertex is far
          barVrt = biggrandparent->prodVtx()->barcode();               // Use grandparent vertex as reference
       }
     }
     return barVrt;
  }

  bool NewVrtSecInclusiveTool::checkTrue2TrVrt(const xAOD::TrackParticle * TP1, const xAOD::TrackParticle * TP2, float nearCut)
  {
     if( !TP1 || !TP2 )return false;
     static const SG::ConstAccessor<ElementLink<xAOD::TruthParticleContainer>> truthParticleLinkAcc( "truthParticleLink");
     if( !truthParticleLinkAcc.isAvailable(*TP1) ) return false;
     if( !truthParticleLinkAcc.isAvailable(*TP2) ) return false;
     const ElementLink<xAOD::TruthParticleContainer>& tplink1 = truthParticleLinkAcc( *TP1 );
     if( !tplink1.isValid() ) return false;
     const ElementLink<xAOD::TruthParticleContainer>& tplink2 = truthParticleLinkAcc( *TP2 );
     if( !tplink2.isValid() ) return false;
     const xAOD::TruthParticle *tpart1 = (*tplink1);        // truth particle for TrackPartile1
     const xAOD::TruthParticle *tpart2 = (*tplink2);        // truth particle for TrackPartile2
     //-----
     if(!tpart1->hasProdVtx()) return false;
     if(!tpart2->hasProdVtx()) return false;
     if(tpart1->prodVtx()->nOutgoingParticles()<2) return false;
     if(tpart2->prodVtx()->nOutgoingParticles()<2) return false;
     //-----
     Amg::Vector3D vpos1(tpart1->prodVtx()->x(),tpart1->prodVtx()->y(),tpart1->prodVtx()->z());
     Amg::Vector3D vpos2(tpart2->prodVtx()->x(),tpart2->prodVtx()->y(),tpart2->prodVtx()->z());
     return Amg::distance(vpos1,vpos2)<nearCut ? true : false;
  }


  double NewVrtSecInclusiveTool::distToMatLayerSignificance(Vrt2Tr & Vrt) const
  { 
     const EventContext& ctx = Gaudi::Hive::currentContext();
     if(Vrt.fitVertex.perp()<20.) return 1.e9;
     double normP=1./Vrt.momentum.P();
     Amg::Vector3D momentumP(Vrt.momentum.Px()*normP,Vrt.momentum.Py()*normP,Vrt.momentum.Pz()*normP);
     Amg::Vector3D momentumN=-momentumP;
     
     const Trk::Layer * someLayer  = nullptr;
     const Trk::Layer * nextLayerP = nullptr;
     const Trk::Layer * nextLayerN = nullptr;
     const auto *volume = m_extrapolator->trackingGeometry()->lowestTrackingVolume(Vrt.fitVertex);
     someLayer = volume->associatedLayer(Vrt.fitVertex);
     const auto *material =  someLayer->layerMaterialProperties();
     if(material){
       nextLayerP=someLayer;
     } else {
       nextLayerP = someLayer->nextLayer(Vrt.fitVertex,momentumP);
       if(nextLayerP){ if(!nextLayerP->layerMaterialProperties())nextLayerP=nullptr; }
       nextLayerN = someLayer->nextLayer(Vrt.fitVertex,momentumN);
       if(nextLayerN){ if(!nextLayerN->layerMaterialProperties())nextLayerN=nullptr; }
     }
     momentumP *= 1.e5;   //100GeV to have straight trajectory
     double charge = 1.;
     const Trk::Perigee pseudoVrtPart(Vrt.fitVertex, momentumP, charge, Vrt.fitVertex);

     const Trk::TrackParameters * extrapParP=nullptr; //along momentum
     const Trk::TrackParameters * extrapParN=nullptr; //backward
     if(nextLayerP){ extrapParP = m_extrapolator->extrapolate(ctx, pseudoVrtPart,
                     nextLayerP->surfaceRepresentation(), Trk::anyDirection, false, Trk::nonInteractingMuon).release();}
     if(nextLayerN){ extrapParN = m_extrapolator->extrapolate(ctx, pseudoVrtPart,
                     nextLayerN->surfaceRepresentation(), Trk::anyDirection, false, Trk::nonInteractingMuon).release();}

     float distanceP=1.e9, distanceN=1.e9;
     if(extrapParP)distanceP=PntPntDist(extrapParP->position(), Vrt.fitVertex);
     if(extrapParN)distanceN=PntPntDist(extrapParN->position(), Vrt.fitVertex);
     if(distanceP==1.e9 && distanceN==1.e9) return 1.e9;

     //std::pair<const Trk::TrackParameters*,const Trk::Layer*> next= 
     //         m_extrapolator->extrapolateToNextActiveLayer(pseudoVrtPart,Trk::anyDirection,true,Trk::pion) ;

     double signif=1.e9;
     std::vector<double> pntCovar={1.e-2,0.,1.e-2,0.,0.,4.e-2};
     if(distanceP<distanceN)signif=vrtVrtDist(Vrt.fitVertex, Vrt.errorMatrix, extrapParP->position(), pntCovar);
     else                   signif=vrtVrtDist(Vrt.fitVertex, Vrt.errorMatrix, extrapParN->position(), pntCovar);
     delete extrapParP;
     delete extrapParN;
     return signif;
  }

  std::vector<double> NewVrtSecInclusiveTool::estimVrtPos( int nTrk, std::deque<long int> &selTrk, std::map<long int, std::vector<double>> & vrt) 
  {
    std::vector<double> estimation(3,0.);
    int ntsel=selTrk.size();
    for( int i=0; i<ntsel-1; i++){
       for( int j=i+1; j<ntsel; j++){
          int k = selTrk[i]<selTrk[j] ? selTrk[i]*nTrk+selTrk[j] : selTrk[j]*nTrk+selTrk[i];
          estimation[0]+=vrt.at(k)[0];
          estimation[1]+=vrt[k][1];
          estimation[2]+=vrt[k][2];
    }  }
    estimation[0] /= ntsel*(ntsel-1)/2;
    estimation[1] /= ntsel*(ntsel-1)/2;
    estimation[2] /= ntsel*(ntsel-1)/2;
    return estimation;
  }

}  //end namespace
