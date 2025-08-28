/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
///
///  @author  Vadim Kostyukhin <vadim.kostyukhin@cern.ch>
///


// Header include
#include "NewVrtSecInclusiveTool/TwoTrackVrtBDTSelector.h"
#include "PathResolver/PathResolver.h"
#include "CxxUtils/checker_macros.h"
 
#include "GaudiKernel/ConcurrencyFlags.h"
#include "TFile.h"
#include "TTree.h"
#include "Math/Vector3D.h"

#include "MVAUtils/BDT.h"


namespace Rec {

//
//Constructor-------------------------------------------------------------- 
TwoTrackVrtBDTSelector::TwoTrackVrtBDTSelector(const std::string& type,
                                           const std::string& name,
                                           const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_SV2T_BDT(nullptr),
    m_instanceName(name)
   {
//
// Declare additional interface
//
    declareInterface< ITwoTrackVertexSelector >(this);
//
    m_massPi  =  Trk::ParticleMasses::mass[Trk::pion];
    m_massP   =  Trk::ParticleMasses::mass[Trk::proton];
    m_massE   =  Trk::ParticleMasses::mass[Trk::electron];
  }

//Destructor---------------------------------------------------------------
  TwoTrackVrtBDTSelector::~TwoTrackVrtBDTSelector(){
     ATH_MSG_DEBUG("TwoTrackVertBDTSelector destructor called");
  }

//Initialize---------------------------------------------------------------
  StatusCode TwoTrackVrtBDTSelector::initialize(){
    ATH_MSG_DEBUG( "Initialising TwoTrackVrtBDTSelector" );

    ATH_CHECK( m_fitSvc.retrieve() );
//--------------------------------------------------------
    //std::string fileName="NewVrtSecInclusiveTool/Fake2TrVertexReject.MVA.v01.root";   ///For local calibration file
    //std::string rootFilePath = PathResolver::find_file(fileName, "DATAPATH");         ///
    std::string rootFilePath = PathResolver::find_calib_file("NewVrtSecInclusiveTool/"+m_calibFileName);
    std::unique_ptr<TFile> rootFile(TFile::Open(rootFilePath.c_str(), "READ")); 
    if (!rootFile) {
        ATH_MSG_FATAL("Could not retrieve root file: " << m_calibFileName);
        return StatusCode::FAILURE;
    }
    std::unique_ptr<TTree> training((TTree*)rootFile->Get("BDT"));
    m_SV2T_BDT = std::make_unique<MVAUtils::BDT>(training.get());
//--------------------------------------------------------
    return StatusCode::SUCCESS;
  }



  StatusCode TwoTrackVrtBDTSelector::finalize()
  {
    ATH_MSG_DEBUG("TwoTrackVrtBDTSelector finalize()");
    return StatusCode::SUCCESS; 
  }


  bool TwoTrackVrtBDTSelector::isgood( const std::pair<const xAOD::TrackParticle*,const xAOD::TrackParticle*> iTrks,
                                       const xAOD::Vertex & candV,
                                             std::pair<ROOT::Math::XYZTVector,ROOT::Math::XYZTVector> moms,
                                       const xAOD::Vertex & tPV) const
  {
      float quality;
      return isgood(iTrks, candV, moms,tPV,quality);
  }

  bool TwoTrackVrtBDTSelector::isgood( const std::pair<const xAOD::TrackParticle*,const xAOD::TrackParticle*> iTrks,
                                       const xAOD::Vertex & candV,
                                             std::pair<ROOT::Math::XYZTVector,ROOT::Math::XYZTVector> moms,
                                       const xAOD::Vertex & tPV,
                                       float & quality) const
  {
      quality=-99.;  
      double Prob2v=TMath::Prob(candV.chiSquared(),1);
      if( Prob2v       < m_sel2VrtProbCut )   return false;    // Vertex probability check
      double vrtR=candV.position().perp();
      auto sumMom=moms.first+moms.second;
      if( sumMom.M()   > m_vrt2TrMassLimit ) return false;    // Invariant mass check 
      if( vrtR         > m_maxSVRadiusCut)   return false;    // Too far from interaction point
      if( sumMom.Pt()  < m_vrt2TrPtMin)      return false;    // Summary momentum > minimal allowed 2-track vertex pt
      if( sumMom.Pt()  > m_vrt2TrPtMax)      return false;    // Maximal allowed 2-track vertex Pt

      ROOT::Math::XYZVector vSVPV(candV.x()-tPV.x(),candV.y()-tPV.y(),candV.z()-tPV.z());
      double cosSVPV=vSVPV.Unit().Dot(sumMom.Vect().Unit());
      if(cosSVPV  < m_cosSVPVCut)             return false;    // Angle between tV-PV direction and summary momentum
	     

      double vrtRErr=vrtRadiusError(candV.position(),candV.covariance() );
//
// Check pixel hits vs vertex positions.
      int ihitIBL  = getIBLHit(iTrks.first);
      int jhitIBL  = getIBLHit(iTrks.second);
      if( m_do2TrkIBLChecks && ( (ihitIBL==0&&jhitIBL>0) || (ihitIBL>0&&jhitIBL==0) ) ) return false;
      int ihitBL   = getBLHit (iTrks.first);
      int jhitBL   = getBLHit (iTrks.second);
//--Very general cleaning cuts based on ID geometry and applicable to all processes
      if( m_do2TrkIBLChecks && vrtR<m_firstPixelLayerR-2.*vrtRErr ){
        if( ihitIBL<1 && ihitBL<1) return false;
        if( jhitIBL<1 && jhitBL<1) return false;
      }
      float ihitR  = iTrks.first->radiusOfFirstHit();
      float jhitR  = iTrks.second->radiusOfFirstHit();
      if(std::abs(ihitR-jhitR)>50.)return false;             //- FMPs are in very different layers
      if( vrtR-std::min(ihitR,jhitR) > 50.) return false;    //- FMP is closer to (0,0) than SV itself
      if(ihitR-vrtR > 180.+2.*vrtRErr)return false;  //- Distance FMP-vertex should be less then SCT-Pixel gap
      if(jhitR-vrtR > 180.+2.*vrtRErr)return false;  //- Distance FMP-vertex should be less then SCT-Pixel gap
//-------------------------------------------------------
      if(m_useVertexCleaning){ //More agressive cleaning 
        if(std::abs(ihitR-jhitR)>12.) return false;
        if( ihitR-vrtR > 36.) return false; // Too big dR between vertex and hit in pixel
        if( jhitR-vrtR > 36.) return false; // Should be another layer in between 
        if( ihitR-vrtR <-2.*vrtRErr) return false; // Vertex is behind hit in pixel 
        if( jhitR-vrtR <-2.*vrtRErr) return false; // Vertex is behind hit in pixel 
      }
//-------------------BDT based rejection
      std::vector<double> impact,impactError;
      m_fitSvc->VKalGetImpact( iTrks.first, tPV.position(), 1, impact, impactError);
      float trk1Signif = sqrt( impact[0]*impact[0]/impactError[0] + impact[1]*impact[1]/impactError[2]);
      m_fitSvc->VKalGetImpact( iTrks.second, tPV.position(), 1, impact, impactError);
      float trk2Signif = sqrt( impact[0]*impact[0]/impactError[0] + impact[1]*impact[1]/impactError[2]);
      float minPtT = std::min(iTrks.first->pt(),iTrks.second->pt());
      std::vector<float> VARS(10);
      VARS[0]=Prob2v;
      VARS[1]=log(sumMom.Pt());
      VARS[2]=log(minPtT);
      VARS[3]=log(vrtR<20. ? vSVPV.Rho() : vrtR);
      VARS[4]=log(std::min(trk1Signif,trk2Signif));
      VARS[5]=log(std::max(trk1Signif,trk2Signif));
      VARS[6]=sumMom.M();
      VARS[7]=sqrt(std::abs(1.-cosSVPV*cosSVPV));
      VARS[8]=vSVPV.Eta();
      VARS[9]=std::max(ihitR,jhitR);
      quality=m_SV2T_BDT->GetGradBoostMVA(VARS);
      if(quality<m_v2tBDTCut) return false;         // BDT rejection

      return true;   //// Finally accept vertex
  }

//----------------------------
//   Vertex error along radius
//----------------------------
  double TwoTrackVrtBDTSelector::vrtRadiusError(const Amg::Vector3D & SecVrt, const std::vector<float>  & VrtErr)
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
  //
  // IBL/BL hit-on-track getters
  int   TwoTrackVrtBDTSelector::getIBLHit(const xAOD::TrackParticle* Part)
  {
        uint8_t IBLhit,IBLexp;
        if(!Part->summaryValue( IBLexp,  xAOD::expectInnermostPixelLayerHit) )    IBLexp = 0;
        if( IBLexp==0 ) return -1;
        if(!Part->summaryValue( IBLhit,  xAOD::numberOfInnermostPixelLayerHits) ) IBLhit = 0;
        if(IBLhit) return 1;
        else       return 0;
  }
  int   TwoTrackVrtBDTSelector::getBLHit(const xAOD::TrackParticle* Part)
  {
        uint8_t BLhit,BLexp;
        if(!Part->summaryValue( BLexp,  xAOD::expectNextToInnermostPixelLayerHit) )     BLexp = 0;
        if( BLexp==0 ) return -1;
        if(!Part->summaryValue( BLhit,  xAOD::numberOfNextToInnermostPixelLayerHits) )  BLhit = 0;
        if(BLhit) return 1;
        else      return 0;
  }

}  // end Rec namespace
