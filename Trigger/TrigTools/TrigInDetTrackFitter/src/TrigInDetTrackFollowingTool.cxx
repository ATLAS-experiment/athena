/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>
#include <iostream>
#include <memory>
#include "GaudiKernel/SystemOfUnits.h"

#include "TrkTrack/Track.h"
#include "TrkTrack/TrackInfo.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkParameters/TrackParameters.h"

#include "TrkPrepRawData/PrepRawData.h"
#include "TrkSpacePoint/SpacePoint.h"

#include "TrigInDetToolInterfaces/ITrigInDetTrackFollowingTool.h"
#include "TrigInDetTrackFollowingTool.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaBaseComps/AthCheckMacros.h"

#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"

#include "TrkSurfaces/Surface.h"

#include "TrkEventPrimitives/ParticleHypothesis.h"

#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"

#include "TrkExUtils/RungeKuttaUtils.h"


TrigFTF_ExtendedTrackState::TrigFTF_ExtendedTrackState(double const* P, const Trk::PlaneSurface* pS) : m_chi2(0), m_ndof(-5), m_pS(pS), m_pO(pS), m_nClusters(0), m_nHoles(0), m_isSwapped(false) {

  for(int i=0;i<5;i++) m_Xk[i] = m_Xk[i+5] = P[i];//duplicate the lower half of the state vector

  memset(&m_Gk[0][0],0,sizeof(m_Gk));

  //covariance block C

  m_Gk[0][0] = 1.0;
  m_Gk[1][1] = 1.0;
  m_Gk[2][2] = 0.0001;
  m_Gk[3][3] = 0.0001;
  m_Gk[4][4] = P[5];

  //triplicate the lower block
  for(int i=0;i<5;i++) {
    m_Gk[i+5][i+5] = m_Gk[i][i+5] = m_Gk[i+5][i] = m_Gk[i][i];
  }

  m_track.clear();
}

void TrigFTF_ExtendedTrackState::AddHole() {
  if(m_isSwapped) {
    m_track.emplace_front(nullptr, nullptr, nullptr, 0, -1);
  }
  else {
    m_track.emplace_back(nullptr, nullptr, nullptr, 0, -1);
  }
  m_nHoles++;
}

void TrigFTF_ExtendedTrackState::correctAngles() {
  if (m_Xk[2] >  M_PI) m_Xk[2] -= 2 * M_PI;
  if (m_Xk[2] < -M_PI) m_Xk[2] += 2 * M_PI;
  if (m_Xk[3] < 0.0)   m_Xk[3] += M_PI;
  if (m_Xk[3] > M_PI)  m_Xk[3] -= M_PI;
  if (m_Xk[7] >  M_PI) m_Xk[7] -= 2 * M_PI;
  if (m_Xk[7] < -M_PI) m_Xk[7] += 2 * M_PI;
  if (m_Xk[8] < 0.0)   m_Xk[8] += M_PI;
  if (m_Xk[8] > M_PI)  m_Xk[8] -= M_PI;

}

void TrigFTF_ExtendedTrackState::AddHit(const Trk::PrepRawData* pPRD, double dchi2, int ndof) {
  
  correctAngles();

  double cov[15];
  int idx=0;
  for(int i=0;i<5;i++) 
    for(int j=0;j<=i;j++) 
      cov[idx++] = m_Gk[i][j];

  if(m_isSwapped) {
    m_track.emplace_front(pPRD, m_Xk, &cov[0], dchi2, ndof);
  }
  else {
    m_track.emplace_back(pPRD, m_Xk, &cov[0], dchi2, ndof);
  }
  
  m_chi2 += dchi2;
  m_ndof += ndof;
  m_nClusters++;
  m_nHoles = 0;//reset the counter
}

void TrigFTF_ExtendedTrackState::SwapTheEnds() {

  m_isSwapped = !m_isSwapped;

  std::swap(m_pS, m_pO);
  
  double tmpX[10];
  memcpy(&tmpX[0], &m_Xk[0], sizeof(tmpX));

  for(int i=0;i<5;i++) {
    m_Xk[i]   = tmpX[i+5];
    m_Xk[i+5] = tmpX[i];
  }

  double tmpG[10][10];
  memcpy(&tmpG[0][0], &m_Gk[0][0], sizeof(tmpG));

  //covariance blocks are moved around to reflect the state vectors swap done above

  for(int i=0;i<5;i++) {
    for(int j=0;j<5;j++) {
      m_Gk[i+5][j+5] = tmpG[i][j];    //C -> E
      m_Gk[i][j]     = tmpG[i+5][j+5];//E -> C
      m_Gk[i][j+5]   = tmpG[i+5][j];  //D -> B
      m_Gk[i+5][j]   = tmpG[i][j+5];  //B -> D
    }
  }
}

void TrigFTF_ExtendedTrackState::report() const {
  std::cout<<"L: ";
  for(int i=0;i<4;i++) std::cout<<m_Xk[i]<<" ";
  std::cout<<1/m_Xk[4]<<" "<<std::sin(m_Xk[3])/m_Xk[4]<<std::endl;
  

  std::cout<<"Covariance at last point:"<<std::endl;

  for(int i=0;i<5;i++) {
    for(int j=0;j<5;j++) std::cout<<m_Gk[i][j]<<" ";
    std::cout<<std::endl;
  }

  std::cout<<"F: ";
  for(int i=0;i<4;i++) std::cout<<m_Xk[i+5]<<" ";
  std::cout<<1/m_Xk[4+5]<<" "<<std::sin(m_Xk[3+5])/m_Xk[4+5]<<std::endl;
  std::cout<<std::endl;

  std::cout<<"Covariance at the first point:"<<std::endl;

  for(int i=0;i<5;i++) {
    for(int j=0;j<5;j++) std::cout<<m_Gk[i+5][j+5]<<" ";
    std::cout<<std::endl;
  }

  std::cout<<"chi2="<<m_chi2<<" ndof="<<m_ndof<<" nClusters="<<m_nClusters<<" nHoles="<<m_nHoles<<std::endl;

}


TrigInDetTrackFollowingTool::TrigInDetTrackFollowingTool(const std::string& t,
							 const std::string& n,
							 const IInterface*  p ): AthAlgTool(t,n,p)
{
  declareInterface< ITrigInDetTrackFollowingTool >( this );
}

StatusCode TrigInDetTrackFollowingTool::initialize() {

  ATH_CHECK( m_fieldCondObjInputKey.initialize());
  ATH_CHECK( m_pixcontainerkey.initialize() );
  ATH_CHECK( m_sctcontainerkey.initialize() );

  return StatusCode::SUCCESS;
}

StatusCode TrigInDetTrackFollowingTool::finalize() {
  return StatusCode::SUCCESS;
}


double TrigInDetTrackFollowingTool::processHit(const InDet::PixelCluster* pPRD, double* resid, double* invcov, const TrigFTF_ExtendedTrackState& ets) const {

  double cluster_cov[2];

  if(m_useHitErrors) {
    cluster_cov[0] = pPRD->localCovariance()(0,0);
    cluster_cov[1] = pPRD->localCovariance()(1,1);
  }
  else {
    cluster_cov[0] = pPRD->width().phiR();
    cluster_cov[1] = pPRD->width().z();
    for(int i=0;i<2;i++) cluster_cov[i] *= cluster_cov[i]/12.0;
  }
 
  double covsum[2][2] = {{ets.m_Gk[0][0] + cluster_cov[0], ets.m_Gk[0][1]}, {ets.m_Gk[0][1], ets.m_Gk[1][1] + cluster_cov[1]}};
    
  double detr = 1/(covsum[0][0]*covsum[1][1] - covsum[0][1]*covsum[1][0]);
  
  invcov[0] = detr*covsum[1][1];
  invcov[1] = -detr*covsum[1][0];
  invcov[2] = detr*covsum[0][0];
  
  resid[0] = pPRD->localPosition().x() - ets.m_Xk[0];
  resid[1] = pPRD->localPosition().y() - ets.m_Xk[1];

  return (resid[0]*(resid[0]*invcov[0] + resid[1]*invcov[1]) + resid[1]*(resid[0]*invcov[1] + resid[1]*invcov[2]));
 
}

double TrigInDetTrackFollowingTool::processHit(const InDet::SCT_Cluster* pPRD, int shape, double& resid, double& invcov, double* H, const TrigFTF_ExtendedTrackState& ets) const {
  
  if(shape==InDetDD::Box) {
	  
    resid = pPRD->localPosition().x() - ets.m_Xk[0];

    double covX = pPRD->localCovariance()(0, 0);

    if(!m_useHitErrors) {
      covX = pPRD->width().phiR(); 
      covX *= (covX/12);
    }

    invcov = 1/(ets.m_Gk[0][0] + covX);
    H[0] = 1;
    H[1] = 0;
  }

  else {
	
    double meas_x = pPRD->localPosition().x();
    double meas_y = pPRD->localPosition().y();
	
    double e00 = pPRD->localCovariance()(0, 0);
    double e01 = pPRD->localCovariance()(0, 1);
    double e11 = pPRD->localCovariance()(1, 1);
    
    double beta = 0.5*std::atan(2*e01/(e00-e11));	
    double sinB, cosB;
    sincos(beta, &sinB, &cosB);
    
    resid = (meas_x - ets.m_Xk[0])*cosB + (meas_y - ets.m_Xk[1])*sinB;
    
    H[0] = cosB;
    H[1] = sinB;
	
    double track_cov = cosB*cosB*(ets.m_Gk[0][0]+e00) + 2*cosB*sinB*(ets.m_Gk[0][1]+e01) + sinB*sinB*(ets.m_Gk[1][1]+e11);
    
    invcov = 1/track_cov;
  }

  return (resid * resid * invcov);

}


const Trk::PrepRawData* TrigInDetTrackFollowingTool::updateTrackState(const InDet::PixelCluster* pInputHit, const InDet::PixelClusterCollection* pColl, TrigFTF_ExtendedTrackState& ets) const {

  const InDet::PixelCluster* bestHit = pInputHit;

  double resid[2];
  double invcov[3];

  if(pColl != nullptr) {
    
    double bestChi2Dist = m_maxChi2Dist_Pixels;

    for(const auto pPRD : *pColl) {

      double dchi2 = processHit(pPRD, resid, invcov, ets);

      if(dchi2 < bestChi2Dist) {
	bestHit = pPRD;
	bestChi2Dist = dchi2;
      }
    }
  }

  if(bestHit == nullptr) return bestHit;

  const InDet::PixelCluster* pPRD = bestHit;

  double dchi2 = processHit(pPRD, resid, invcov, ets);

  double CHT[10][2];

  for(int i=0;i<10;i++) {
    CHT[i][0] =  ets.m_Gk[i][0];
    CHT[i][1] =  ets.m_Gk[i][1];
  }

  double Gain[10][2];
  double V[2][2] = {{invcov[0], invcov[1]}, {invcov[1], invcov[2]}};
    
  for(int i=0;i<10;i++) 
    for(int j=0;j<2;j++) Gain[i][j] =  CHT[i][0]*V[0][j] + CHT[i][1]*V[1][j];
  
  for(int i=0;i<10;i++) {
    ets.m_Xk[i] += Gain[i][0]*resid[0] + Gain[i][1]*resid[1];
    
    for(int j=0;j<=i;j++) {
      ets.m_Gk[i][j] = ets.m_Gk[i][j] - (Gain[i][0]*CHT[j][0] + Gain[i][1]*CHT[j][1]);
      ets.m_Gk[j][i] = ets.m_Gk[i][j];
    }
  }

  ets.AddHit(pPRD, dchi2, 2);

  return pPRD;
}


const Trk::PrepRawData* TrigInDetTrackFollowingTool::updateTrackState(const InDet::SCT_Cluster* pInputHit, const InDet::SCT_ClusterCollection* pColl, int shape, TrigFTF_ExtendedTrackState& ets) const {

  const InDet::SCT_Cluster* bestHit = pInputHit;

  double resid, invcov;
  double H[2];//linearized observation matrix
  
  if(pColl && !pColl->empty()) {

    double bestChi2Dist = m_maxChi2Dist_Strips;

    for(const auto pPRD : *pColl) {

      double dchi2 = processHit(pPRD, shape, resid, invcov, H, ets);

      if(dchi2 < bestChi2Dist) {
	bestHit = pPRD;
	bestChi2Dist = dchi2;
      }
    }
  }

  if(bestHit == nullptr) return bestHit;

  const InDet::SCT_Cluster* pPRD = bestHit;
  
  double dchi2 = 0.0;

  if(shape == InDetDD::Box) {

    dchi2 = processHit(pPRD, shape, resid, invcov, H, ets);

    double CHT[10], Gain[10];
    
    for(int i=0;i<10;i++) {
      CHT[i] =  ets.m_Gk[i][0];
      Gain[i] = CHT[i] * invcov;
    }
    
    for(int i=0;i<10;i++) {
      ets.m_Xk[i] += Gain[i]*resid;
      for(int j=0;j<=i;j++) {
	ets.m_Gk[i][j] = ets.m_Gk[i][j] - Gain[i]*CHT[j];
	ets.m_Gk[j][i] = ets.m_Gk[i][j];
      }
    }
    
    //boundary check
    
    double covY = pPRD->localCovariance()(1, 1);
    double stripCentre = pPRD->localPosition().y();
    double stripHalfLength = std::sqrt(3*covY);

    double dY = ets.m_Xk[1] - stripCentre;
    
    if(dY >  stripHalfLength) {
      ets.m_Xk[1] = stripCentre + stripHalfLength;
    }
    
    if(dY < -stripHalfLength) {
      ets.m_Xk[1] = stripCentre - stripHalfLength;
    }
  }
  else { //Annulus

    dchi2 = processHit(pPRD, shape, resid, invcov, H, ets);

    double CHT[10], Gain[10];
    
    for(int i=0;i<10;i++) {
      CHT[i] =  H[0]*ets.m_Gk[i][0] + H[1]*ets.m_Gk[i][1];
      Gain[i] = CHT[i] * invcov;
    }
    
    for(int i=0;i<10;i++) {
      ets.m_Xk[i] += Gain[i]*resid;
      for(int j=0;j<=i;j++) {
	ets.m_Gk[i][j] = ets.m_Gk[i][j] - Gain[i]*CHT[j];
	ets.m_Gk[j][i] = ets.m_Gk[i][j];
      }
    }
  }

  ets.AddHit(pPRD, dchi2, 1);

  return pPRD;
}

std::unique_ptr<TrigFTF_ExtendedTrackState> TrigInDetTrackFollowingTool::fitTheSeed(const std::vector<const Trk::SpacePoint*>& seed, MagField::AtlasFieldCache& fieldCache) const {

  //track parameters are estimated at the last point of the seed (inside-out approach)

  const Trk::SpacePoint& SPb = *seed.at(0);
  const Trk::SpacePoint& SPm = *seed.at(1);
  const Trk::SpacePoint& SPe = *seed.at(seed.size()-1);

  double pb[3] = {SPb.globalPosition().x(), SPb.globalPosition().y(), SPb.globalPosition().z()};
  double pm[3] = {SPm.globalPosition().x(), SPm.globalPosition().y(), SPm.globalPosition().z()};
  double pe[3] = {SPe.globalPosition().x(), SPe.globalPosition().y(), SPe.globalPosition().z()};
  
  double dsp[2] = {pe[0]-pb[0], pe[1]-pb[1]};
  double Lsp = std::sqrt(dsp[0]*dsp[0] + dsp[1]*dsp[1]);
  double cosA = dsp[0]/Lsp;
  double sinA = dsp[1]/Lsp;
  double tau0 = (pe[2]-pb[2])/(SPe.globalPosition().perp()-SPb.globalPosition().perp());

  double dxm = pm[0] - pb[0];
  double dym = pm[1] - pb[1];

  double x1 =  dxm*cosA + dym*sinA;
  double m1 = -dxm*sinA + dym*cosA;

  double a_parab = -2*m1/(x1*(Lsp-x1));
  double b_parab = -0.5*a_parab*Lsp;

  double Rx[3] = {0,b_parab,a_parab};
  double Ry[2] = {pb[2],tau0};

  double Cx[3][3];
  double Cy[2][2];

  memset(&Cx[0][0],0,sizeof(Cx));
  Cx[0][0] = 0.1;
  Cx[1][1] = 0.001;
  Cx[2][2] = 0.001;

  memset(&Cy[0][0],0,sizeof(Cy));
  Cy[0][0] = 0.25;
  Cy[1][1] = 0.001;

  double path = -Lsp;
  path = 0.0;
  double radius= SPb.globalPosition().perp();

  double Fx[3][3];
  double Fy[2][2];

  memset(&Fx[0][0],0,sizeof(Fx));
  Fx[0][0] = 1.0;
  Fx[1][1] = 1.0;
  Fx[2][2] = 1.0;

  memset(&Fy[0][0],0,sizeof(Fy));
  Fy[0][0] = 1.0;
  Fy[1][1] = 1.0;

  double sigma_x2 = std::pow(0.08,2);
  double sigma_y2 = std::pow(0.3,2);

  for(const auto& sp : seed) {

    //1. extrapolate

    double pk[3] = {sp->globalPosition().x(), sp->globalPosition().y(), sp->globalPosition().z()};

    double dx = pk[0] - pb[0];
    double dy = pk[1] - pb[1];

    double dist  = dx*cosA + dy*sinA;
    double measx =-dx*sinA + dy*cosA;
    double measy = pk[2];

    double ds = dist - path;

    path = dist;
    
    double rk = sp->globalPosition().perp();
    double dr = rk - radius;
    radius = rk;
    
    //update Jacobians
    Fx[0][1] = ds;
    Fx[0][2] = 0.5*ds*ds;
    Fx[1][2] = ds;

    Fy[0][1] = dr;

    Cx[1][1] += 1e-7;
    Cy[1][1] += 1e-7;

    double Rex[3] = {Rx[0], Rx[1], Rx[2]};

    Rex[0] += Fx[0][1]*Rx[1] + Fx[0][2]*Rx[2];
    Rex[1] += Fx[1][2]*Rx[2];

    double Cex[3][3], CFT[3][3];

    for(int i=0;i<3;i++) {
      for(int j=0;j<3;j++) {
	CFT[i][j] = 0;
	for(int m=0;m<3;m++) CFT[i][j] += Cx[i][m]*Fx[j][m];
      }
    }
    for(int i=0;i<3;i++) {
      for(int j=0;j<3;j++) {
	Cex[i][j] = 0;
	for(int m=0;m<3;m++) Cex[i][j] += Fx[i][m]*CFT[m][j];
      }
    }

    double Rey[2] = {Ry[0], Ry[1]};

    Rey[0] += Fy[0][1]*Ry[1];

    double Cey[3][3];

    for(int i=0;i<2;i++) {
      for(int j=0;j<2;j++) {
	CFT[i][j] = 0;
	for(int m=0;m<2;m++) CFT[i][j] += Cy[i][m]*Fy[j][m];
      }
    }
    for(int i=0;i<2;i++) {
      for(int j=0;j<2;j++) {
	Cey[i][j] = 0;
	for(int m=0;m<2;m++) Cey[i][j] += Fy[i][m]*CFT[m][j];
      }
    }

    //2. update

    double CHTx[3] = {Cex[0][0], Cex[0][1], Cex[0][2]};
    double Dx = 1/(Cex[0][0] + sigma_x2);

    double resid = measx - Rex[0];

    double Kx[3] = {Dx*CHTx[0], Dx*CHTx[1], Dx*CHTx[2]};

    for(int i=0;i<3;i++) Rx[i] = Rex[i] + Kx[i]*resid;
    for(int i=0;i<3;i++) {
      for(int j=0;j<3;j++) {
	Cx[i][j] = Cex[i][j] - Kx[i]*CHTx[j];
      }
    }

    double CHTy[2] = {Cey[0][0], Cey[0][1]};
    double Dy = 1/(Cey[0][0] + sigma_y2);

    resid = measy - Rey[0];

    double Ky[2] = {Dy*CHTy[0], Dy*CHTy[1]};

    for(int i=0;i<2;i++) Ry[i] = Rey[i] + Ky[i]*resid;
    for(int i=0;i<2;i++) {
      for(int j=0;j<2;j++) {
	Cy[i][j] = Cey[i][j] - Ky[i]*CHTy[j];
      }
    }
  } 

  //create initial track state at the last spacepoint of the seed
  
  double B0[3];
  
  fieldCache.getField(pe, B0);

  double P0[6];
  
  const Trk::PrepRawData* cle  = SPe.clusterList().first;
  
  const Trk::PlaneSurface* thePlane = static_cast<const Trk::PlaneSurface*>(&cle->detectorElement()->surface());
  
  if(thePlane == nullptr) return nullptr;

  P0[0] = cle->localPosition()[0];
  P0[1] = cle->localPosition()[1];
  P0[2] = std::atan2(sinA - Rx[1]*cosA, cosA - Rx[1]*sinA);//phi in the global c.s.
  P0[3] = std::atan2(1, Ry[1]);//theta in the global c.s.
  double coeff = 1.0/(300.0*B0[2]*std::sqrt(1+Ry[1]*Ry[1]));
  P0[4] = -Rx[2]*coeff;//qOverP estimate
  P0[5] = Cx[2][2]*coeff*coeff;//qOverP covariance

  return std::make_unique<TrigFTF_ExtendedTrackState>(P0, thePlane);

}

Trk::Track* TrigInDetTrackFollowingTool::getTrack(const std::vector<const Trk::SpacePoint*>& seed, const std::vector<const InDetDD::SiDetectorElement*>& road, const EventContext& ctx) const {

  //1. get magnetic field

  MagField::AtlasFieldCache fieldCache;

  SG::ReadCondHandle<AtlasFieldCacheCondObj> fieldCondObj{m_fieldCondObjInputKey, ctx};
  if (!fieldCondObj.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCondObjInputKey.key());
    return nullptr;
  }

  fieldCondObj->getInitializedCache (fieldCache);

  //2. get hits

  SG::ReadHandle<InDet::PixelClusterContainer> pixcontainer(m_pixcontainerkey, ctx);

  if (!pixcontainer.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve Pixel Cluster Container with key " << m_pixcontainerkey.key());
    return nullptr;
  }

  const InDet::PixelClusterContainer* p_pixcontainer = pixcontainer.ptr();
 
  SG::ReadHandle<InDet::SCT_ClusterContainer> sctcontainer(m_sctcontainerkey, ctx);

  if (!sctcontainer.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve Strip Cluster Container with key " << m_sctcontainerkey.key());
    return nullptr;
  }

  const InDet::SCT_ClusterContainer* p_sctcontainer = sctcontainer.ptr();
 
  //3. prepare the initial track state
  
  int nModules = road.size();

  std::vector<const Trk::PrepRawData*> assignedHits(nModules, nullptr);//assuming maximum one assigned hit per detector element

  std::vector<int>  moduleStatus(nModules, 0);//initial status: unchecked

  unsigned int seedSize = seed.size();

  std::vector<unsigned int> seedHashes(seedSize,0);

  for(unsigned int spIdx=0;spIdx<seedSize;spIdx++) {
    const Trk::PrepRawData* prd  = seed.at(spIdx)->clusterList().first;
    seedHashes[spIdx] = prd->detectorElement()->identifyHash();
  }

  //pre-assigning the hits contained in the input track seed

  int nUnassigned = seedSize;
 
  int startModuleIdx = -1;
  
  for(int moduleIdx = 0;moduleIdx<nModules;moduleIdx++) {
    unsigned int hash = road.at(moduleIdx)->identifyHash();
    for(unsigned int spIdx=0;spIdx<seedSize;spIdx++) {
      if(seedHashes[spIdx] != hash) continue;
      
      assignedHits[moduleIdx] = seed.at(spIdx)->clusterList().first;
      moduleStatus[moduleIdx] = 1;//seed hit assigned
      
      startModuleIdx = moduleIdx;
      --nUnassigned;
      break;
    }
    if(nUnassigned == 0) break;
  }

  if(nUnassigned > 0) return nullptr;//bad road or bad seed

  std::unique_ptr<TrigFTF_ExtendedTrackState> initialState = fitTheSeed(seed, fieldCache);

  if(initialState == nullptr) return nullptr;

  initialState->correctAngles();

  TrigFTF_ExtendedTrackState& theState = *initialState;

  std::vector<int> moduleIndexSequence(nModules);//the order in which modules will be explored to find hits

  int modCounter = 0;

  for(int moduleIdx = startModuleIdx + 1;moduleIdx<nModules;moduleIdx++, modCounter++) {//forward pass
    moduleIndexSequence[modCounter] = moduleIdx;
  }
  for(int moduleIdx = startModuleIdx;moduleIdx>=0;moduleIdx--, modCounter++) {//backward pass
    moduleIndexSequence[modCounter] = moduleIdx;
  }
  
  //4. The track following loop

  for(auto const moduleIdx : moduleIndexSequence) {

    if(theState.m_nHoles > m_nHolesMax) { //bailing-out early to save CPU time
      return nullptr;
    }
    
    if(moduleStatus[moduleIdx] < 0) continue;//checked and rejected
    
    const InDetDD::SiDetectorElement* de = road.at(moduleIdx);//next module

    //4a. extrapolation to the target surface

    const Trk::PrepRawData* pPRD = assignedHits[moduleIdx];

    if(moduleIdx == startModuleIdx) {
      theState.SwapTheEnds();//no extrapolation is needed (apart from the material effects)
    }
    else {

      const Trk::PlaneSurface* plane = static_cast<const Trk::PlaneSurface*>(&de->surface());
      
      if(pPRD == nullptr) {//no hit assigned yet, check if we can cross the module first
	//tentative extrapolation
	bool inBounds = checkIntersection(theState.m_Xk, theState.m_pS, plane, fieldCache);
	if(!inBounds) {
	  moduleStatus[moduleIdx] = -2;//miss
	  continue;
	}
      }

      //precise extrapolation
      int rkCode = extrapolateTrackState(theState, plane, fieldCache);

      if(rkCode!=0) {
	moduleStatus[moduleIdx] = -2;//miss
	if(pPRD != nullptr) {
	  theState.AddHole();//because we were expecting the pre-assigned hit
	}
	continue;
      }
    }

    //4b. search for the best hit and update the track state

    unsigned int moduleHash = de->identifyHash();

    unsigned int nHits = 0;

    const Trk::PrepRawData* selectedHit = nullptr;

    if(de->isPixel()) {//Pixel module

      const InDet::PixelClusterCollection *clustersOnElement = nullptr;
      const InDet::PixelCluster* pPixelHit = nullptr;

      if(pPRD == nullptr) {
	if(p_pixcontainer != nullptr) {
	  clustersOnElement = (*p_pixcontainer).indexFindPtr(moduleHash);
	  if(clustersOnElement != nullptr) {
	    nHits = clustersOnElement->size();
	  }
	}
      }
      else {
	pPixelHit = dynamic_cast<const InDet::PixelCluster*>(pPRD);
	nHits = 1;
      }
      
      if(nHits > 0) {
	selectedHit = updateTrackState(pPixelHit, clustersOnElement, theState);
      }

    }
    else {//Strip module

      const InDet::SCT_ClusterCollection *clustersOnElement = nullptr;
      const InDet::SCT_Cluster* pStripHit = nullptr;

      if(pPRD == nullptr) {
	if(p_sctcontainer != nullptr) {
	  clustersOnElement = (*p_sctcontainer).indexFindPtr(moduleHash);
	  if(clustersOnElement != nullptr) {
	    nHits = clustersOnElement->size();
	  }
	}
      }
      else {
	pStripHit = dynamic_cast<const InDet::SCT_Cluster*>(pPRD);
	nHits = 1;
      }

      if(nHits > 0) {
	selectedHit = updateTrackState(pStripHit, clustersOnElement, de->design().shape(), theState);
      }

    }

    if(nHits == 0) {// dead module?
      theState.AddHole();
      moduleStatus[moduleIdx] = -3;//dead module
      continue;
    }

    if(pPRD == nullptr) {
      if(selectedHit == nullptr) {
	theState.AddHole();      
	moduleStatus[moduleIdx] = -4;//all hits rejected
      }
      else {
	assignedHits[moduleIdx] = selectedHit;
	moduleStatus[moduleIdx] = de->isPixel() ? 2 : 3;//a new Pixel/Strip hit assigned
      }
    }
  } //end of the track following loop

  //5. create output track

  int nClusters  = theState.m_nClusters;
  int nHoles     = theState.m_nHoles;
  double chi2tot = theState.m_chi2;

  if(nClusters < m_nClustersMin) {
    return nullptr;
  }

  if(nHoles > m_nHolesMax) {
    return nullptr;
  }
  
  int rkCode = extrapolateTrackState(theState, nullptr, fieldCache);//to perigee
  
  if(rkCode !=0) {
    return nullptr;
  }
  
  int ndoftot   = theState.m_ndof;
  double qOverP = theState.m_Xk[4];
  double pt     = std::sin(theState.m_Xk[3])/qOverP;
  double phi0   = theState.m_Xk[2];
  double theta  = theState.m_Xk[3];
  
  double z0     = theState.m_Xk[1];
  double d0     = theState.m_Xk[0];

  bool bad_cov = false;

  auto cov = AmgSymMatrix(5){};
  
  for(int i=0;i<5;i++) {
    for(int j=0;j<=i;j++) {
      double c = theState.m_Gk[i][j];
      if (i == j && c < 0) {
	bad_cov = true;//Diagonal elements must be positive
	ATH_MSG_DEBUG("REGTEST: cov(" << i << "," << i << ") =" << c << " < 0, reject track");
	break;
      }
      cov.fillSymmetric(i,j, c);
    }
  }

  if((ndoftot<0) || (fabs(pt)<100.0) || (std::isnan(pt)) || bad_cov) {
    ATH_MSG_DEBUG("Track following failed - possibly floating point problem");
    return nullptr;
  }

  Trk::PerigeeSurface perigeeSurface;
  
  std::unique_ptr<Trk::TrackParameters> pPP = perigeeSurface.createUniqueTrackParameters(d0, z0, phi0, theta, qOverP, cov);

  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> perType;
  perType.set(Trk::TrackStateOnSurface::Perigee);
  
  auto pParVec    = std::make_unique<Trk::TrackStates>();

  pParVec->reserve(50);
  pParVec->push_back(new Trk::TrackStateOnSurface(nullptr, std::move(pPP), nullptr, perType));
  
  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> rioType(0);
  rioType.set(Trk::TrackStateOnSurface::Measurement);
  rioType.set(Trk::TrackStateOnSurface::Scatterer);

  for(const auto& ha : theState.m_track) {//loop over hit assignments

    const Trk::PrepRawData* pPRD = ha.m_pPRD;

    if(pPRD == nullptr) continue;//skip holes

    //create track states on surface

    int ndof = ha.m_ndof;
    
    const Trk::PlaneSurface* pPS = dynamic_cast<const Trk::PlaneSurface*>(&pPRD->detectorElement()->surface());

    if(pPS==nullptr) continue;
    
    const InDet::SiCluster* pCL = dynamic_cast<const InDet::SiCluster*>(pPRD);
    
    Trk::LocalParameters locPos = Trk::LocalParameters(pCL->localPosition());

    const Amg::MatrixX& cov = pCL->localCovariance();

    IdentifierHash hash = pPRD->detectorElement()->identifyHash();

    std::unique_ptr<Trk::MeasurementBase> pRIO{};

    if(ndof == 2) {
      const InDet::PixelCluster* pPixel = static_cast<const InDet::PixelCluster*>(pCL);
      if(pPixel) {
	pRIO = std::make_unique<InDet::PixelClusterOnTrack>(pPixel, std::move(locPos), Amg::MatrixX(cov), 
							    hash, pPixel->globalPosition(), pPixel->gangedPixel());
      }
    }
    else {
      const InDet::SCT_Cluster* pStrip = static_cast<const InDet::SCT_Cluster*>(pCL);
      if(pStrip) {
	pRIO = std::make_unique<InDet::SCT_ClusterOnTrack>(pStrip, std::move(locPos), Amg::MatrixX(cov), 
							   hash, pStrip->globalPosition());
      }      
    } 
    
    auto pM = AmgSymMatrix(5){};
    
    int idx = 0;
    for(int i=0;i<5;i++) {
      for(int j=0;j<=i;j++) {
	pM.fillSymmetric(i,j,ha.m_Ck[idx++]);
      }
    }
        
    std::unique_ptr<Trk::TrackParameters> pTP = pPS->createUniqueTrackParameters(ha.m_Xk[0], ha.m_Xk[1], ha.m_Xk[2], ha.m_Xk[3], ha.m_Xk[4], pM);
    
    Trk::FitQualityOnSurface FQ    = Trk::FitQualityOnSurface(ha.m_dchi2, ndof);
    
    Trk::TrackStateOnSurface* pTSS = new Trk::TrackStateOnSurface(FQ, std::move(pRIO), std::move(pTP), nullptr, rioType);
    
    pParVec->push_back(pTSS);
  }
  
  auto pFQ = std::make_unique<Trk::FitQuality>(chi2tot, ndoftot);

  Trk::TrackInfo info{};

  info.setParticleHypothesis(Trk::pion);
  info.setPatternRecognitionInfo(Trk::TrackInfo::strategyC);

  Trk::Track* foundTrack = new Trk::Track(info, std::move(pParVec), std::move(pFQ));

  return foundTrack;
}

int TrigInDetTrackFollowingTool::extrapolateTrackState(TrigFTF_ExtendedTrackState& ETS, const Trk::PlaneSurface* pN, MagField::AtlasFieldCache& fieldCache) const {

  double Re[5];
  
  memcpy(&Re[0], &ETS.m_Xk[0], sizeof(Re));

  double P[8]; //parameters + path
  double Jm[40];//Jacobian
 
  memset(&P[0],0,sizeof(P));

  const Amg::Transform3D& Trf = ETS.m_pS->transform();
  double Ax[3] = {Trf(0,0),Trf(1,0),Trf(2,0)}; //loc1-axis in the global frame 
  double Ay[3] = {Trf(0,1),Trf(1,1),Trf(2,1)}; //loc2-axis in the global frame
  
  double gP[3];
  gP[0] = Trf(0,3) + Ax[0]*Re[0] + Ay[0]*Re[1];
  gP[1] = Trf(1,3) + Ax[1]*Re[0] + Ay[1]*Re[1];
  gP[2] = Trf(2,3) + Ax[2]*Re[0] + Ay[2]*Re[1];
  
  double sinf, cosf;
  double sint, cost;

  sincos(Re[2], &sinf, &cosf);
  sincos(Re[3], &sint, &cost);
  
  double gV[3]  = {cosf*sint, sinf*sint, cost};
  
  memset(&Jm[0],0,sizeof(Jm));

  //Track state and Jacobian initialization
  
  P[6] = Re[4];
  P[7] = 0.0;
  
  for(int i=0;i<3;i++) {
    P[i]    = gP[i];
    P[i+3]  = gV[i];
    Jm[i]   = Ax[i];
    Jm[7+i] = Ay[i];
  }
  
  Jm[17] =-P[4];//17
  Jm[18] = P[3];//18
  Jm[24] = cosf*cost;//24
  Jm[25] = sinf*cost;//25
  Jm[26] =-sint;//26
  Jm[34] = 1.0;//34
  
  int code = RungeKutta34(P, Jm, pN, fieldCache, true);

  if(code!=0) return code;

  double J[21];
  memset(&J[0],0,sizeof(J));

  double BigP[45];
  memset(&BigP[0],0,sizeof(BigP));
    
  for(int i=0;i<7;i++)  BigP[i]   = P[i];
  for(int i=0;i<35;i++) BigP[i+7] = Jm[i];

  if(pN == nullptr) {//special case: to perigee
    Trk::PerigeeSurface perSurf;
    Trk::RungeKuttaUtils::transformGlobalToLocal(&perSurf, true, BigP, Re, J);
  }
  else {//from plane to plane
    Trk::RungeKuttaUtils::transformGlobalToLocal(pN, true, BigP, Re, J);
  }
    
  const double*  Az = Trf.matrix().col(2).data();
  
  // z-component of track direction vector in the local c.s.
  
  double lV = Az[0]*gV[0] + Az[1]*gV[1] + Az[2]*gV[2];
  
  double xOverX0 = 0.0;

  if(m_useDetectorThickness) {
    const InDetDD::SiDetectorElement* pDE = dynamic_cast<const InDetDD::SiDetectorElement*>(ETS.m_pS->associatedDetectorElement());
    if(pDE!=nullptr) {
      xOverX0 = pDE->design().thickness()/93.7;//Radiation length of silicon according to PDG
    }
  }
  else {
    xOverX0 = m_nominalRadLength;
  }

  double radLength = xOverX0/std::fabs(lV);
  
  double sigmaMS = 13.6 * std::fabs(Re[4]) * std::sqrt(radLength) * (1.0 + 0.038 * std::log(radLength));
  double s2 = sigmaMS * sigmaMS;
  double a = 1.0 / sint;
  double a2 = a * a;

  //multiple scattering

  ETS.m_Gk[2][2] += s2 * a2;
  ETS.m_Gk[3][3] += s2;
  ETS.m_Gk[2][3] += s2 * a;
  ETS.m_Gk[3][2] = ETS.m_Gk[2][3];
  
  //energy loss
  
  ETS.m_Gk[4][4] += Re[4] * Re[4] * radLength * (0.415 - 0.744 * radLength);

  //Sym. product J*C*J^T  

  double Be[5][5];//upper off-diagonal block
  
  memset(&Be[0][0],0,sizeof(Be));

  for(int i=0;i<4;i++) {
    for(int j=0;j<5;j++) {
      for(int k=0;k<5;k++) Be[i][j] += J[k+i*5]*ETS.m_Gk[k][j+5];
    }
  }
  for(int j=0;j<5;j++) {
    Be[4][j] = ETS.m_Gk[4][j+5];
  }

  double JC[5][5];
  double Ce[5][5];//"running" diagonal block

  memset(&JC[0][0],0,sizeof(JC));
  memset(&Ce[0][0],0,sizeof(Ce));

  for(int i=0;i<4;i++) {
    for(int j=0;j<5;j++) {
      for(int k=0;k<5;k++) JC[i][j] += J[k+i*5]*ETS.m_Gk[k][j];
    }
  }
  for(int j=0;j<5;j++) {
    JC[4][j] = ETS.m_Gk[4][j];
  }

  for(int i=0;i<5;i++) {
    for(int j=0;j<=i;j++) {
      if(j<4) {
	for(int k=0;k<5;k++) Ce[i][j] += JC[i][k]*J[k+j*5];
	Ce[j][i] = Ce[i][j];
      }
      else {
	Ce[i][4] = Ce[4][i] = JC[i][4];
      }
    }
  }
  
  //copy back to the state

  ETS.m_pS = pN;//moving forward

  for(int i=0;i<5;i++) {
    ETS.m_Xk[i] = Re[i];
    for(int j=0;j<5;j++) {
      ETS.m_Gk[i][j]   = Ce[i][j];
      ETS.m_Gk[i][j+5] = Be[i][j];
      ETS.m_Gk[j+5][i] = Be[i][j];
    }
  }
  return 0;
}

bool TrigInDetTrackFollowingTool::checkIntersection(double const* Rk, const Trk::PlaneSurface* pS, const Trk::PlaneSurface* pN, MagField::AtlasFieldCache& fieldCache) const {

  const double C = 299.9975;
  const double minStep = 100.0;
  const double bound_tol = -1.0;
  
  double Re[5];
  
  memcpy(&Re[0], &Rk[0], sizeof(Re));

  const Amg::Transform3D& Trf = pS->transform();

  double Ax[3] = {Trf(0,0),Trf(1,0),Trf(2,0)}; //loc1-axis in the global frame 

  double Ay[3] = {Trf(0,1),Trf(1,1),Trf(2,1)}; //loc2-axis in the global frame
  
  double gP[3];

  gP[0] = Trf(0,3) + Ax[0]*Re[0] + Ay[0]*Re[1];
  gP[1] = Trf(1,3) + Ax[1]*Re[0] + Ay[1]*Re[1];
  gP[2] = Trf(2,3) + Ax[2]*Re[0] + Ay[2]*Re[1];
  
  double sinf, cosf;
  double sint, cost;

  sincos(Re[2], &sinf, &cosf);
  sincos(Re[3], &sint, &cost);
  
  double gV[3]  = {cosf*sint, sinf*sint, cost};

  const Amg::Vector3D& normal = pN->normal();
  const Amg::Vector3D& center = pN->center();

  double D[4] = {normal[0], normal[1], normal[2], 0.0};
  
  for(int i=0;i<3;i++) D[3] += -normal[i]*center[i];

  double CQ = C*Re[4];

  double gB[3]; 

  fieldCache.getField(gP, gB);

  double c = D[0]*gP[0] + D[1]*gP[1] + D[2]*gP[2] + D[3];
  double b = D[0]*gV[0] + D[1]*gV[1] + D[2]*gV[2];
  double a = 0.5*CQ*(gB[0]*(D[1]*gV[2]-D[2]*gV[1]) + gB[1]*(D[2]*gV[0]-D[0]*gV[2]) + gB[2]*(D[0]*gV[1]-D[1]*gV[0]));
  
  double ratio = 4*a*c/(b*b);

  bool useExpansion = std::fabs(ratio)<0.1;

  double sl = 0.0;

  if(useExpansion) {
    sl = -c/b;
    sl = sl*(1-a*sl/b);
  }
  else {

    double discr = b*b-4.0*a*c;

    if(discr<0.0) {
      return false;
    }

    int signb = (b<0.0)?-1:1;
    sl = (-b + signb*std::sqrt(discr))/(2*a);
  }

  int nStepMax = 1;

  if(std::fabs(sl)>=minStep) {
    nStepMax = (int)(std::fabs(sl)/minStep)+1;
  }

  if((nStepMax<0)||(nStepMax>100)) {  
    return false;
  }

  double Av  = sl*CQ;
  double Ac  = 0.5*sl*Av;
  double DVx = gV[1]*gB[2] - gV[2]*gB[1];
  double DVy = gV[2]*gB[0] - gV[0]*gB[2];
  double DVz = gV[0]*gB[1] - gV[1]*gB[0];

  double E[3] = {gP[0]+gV[0]*sl+Ac*DVx, gP[1]+gV[1]*sl+Ac*DVy, gP[2]+gV[2]*sl+Ac*DVz};

  if(nStepMax == 1) {
    for(int i=0;i<3;i++) gP[i] = E[i];
  }
  else {
    double gBe[3];

    fieldCache.getField(E, gBe);

    double inv_step = 1/sl;

    double dBds[3] = {inv_step*(gBe[0]-gB[0]),inv_step*(gBe[1]-gB[1]),inv_step*(gBe[2]-gB[2])};

    int nStep = nStepMax;

    while(nStep > 0) {
      
      c = D[0]*gP[0] + D[1]*gP[1] + D[2]*gP[2] + D[3];
      b = D[0]*gV[0] + D[1]*gV[1] + D[2]*gV[2];
      a = 0.5*CQ*(gB[0]*(D[1]*gV[2]-D[2]*gV[1])+gB[1]*(D[2]*gV[0]-D[0]*gV[2])+gB[2]*(D[0]*gV[1]-D[1]*gV[0]));
      
      ratio = 4*a*c/(b*b);
      useExpansion = std::fabs(ratio) < 0.1;
      
      if(useExpansion) {
	sl = -c/b;
	sl = sl*(1-a*sl/b);
      }
      else {
	double discr=b*b-4.0*a*c;
	if(discr<0.0) {
	  return false;      
	}
	int signb = (b<0.0)?-1:1;
	sl = (-b+signb*std::sqrt(discr))/(2*a);
      }

      double ds = sl/nStep;
      Av = ds*CQ;
      Ac = 0.5*ds*Av;
      
      DVx = gV[1]*gB[2] - gV[2]*gB[1];
      DVy = gV[2]*gB[0] - gV[0]*gB[2];
      DVz = gV[0]*gB[1] - gV[1]*gB[0];
      
      E[0] = gP[0] + gV[0]*ds + Ac*DVx;
      E[1] = gP[1] + gV[1]*ds + Ac*DVy;
      E[2] = gP[2] + gV[2]*ds + Ac*DVz;
      
      double V[3];

      V[0] = gV[0] + Av*DVx;
      V[1] = gV[1] + Av*DVy;
      V[2] = gV[2] + Av*DVz;
      
      for(int i=0;i<3;i++) {	  
	gV[i] = V[i];gP[i] = E[i];
      }

      for(int i=0;i<3;i++) gB[i] += dBds[i]*ds;//field interpolation

      nStep--;
    }
  }
  
  //global to local

  const Amg::Transform3D& Trf2 = pN->transform();

  const double*  Ax2 = Trf2.matrix().col(0).data();
  const double*  Ay2 = Trf2.matrix().col(1).data();

  const double d[3] = { gP[0] - Trf2(0, 3), gP[1] - Trf2(1, 3), gP[2] - Trf2(2, 3) };

  double locX = d[0] * Ax2[0] + d[1] * Ax2[1] + d[2] * Ax2[2];
  double locY = d[0] * Ay2[0] + d[1] * Ay2[1] + d[2] * Ay2[2];
    
  return (pN->insideBounds(Amg::Vector2D(locX, locY), bound_tol, bound_tol));

}

double TrigInDetTrackFollowingTool::estimateRK_Step(const Trk::PlaneSurface* pN, double const * P) const {

  double Step = 1e8;

  if(pN == nullptr) { //step to perigee "surface" assuming the global c.s. origin at (0,0)

    Step = -(P[0]*P[3] + P[1]*P[4])/(1 - P[5]*P[5]);
    return Step;
  }

  const Amg::Vector3D& normal = pN->normal();
  const Amg::Vector3D& center = pN->center();

  double D = 0.0;// -(r0,n)
  
  for(int i=0;i<3;i++) D += -normal[i]*center[i];

  double Sum = D;
  double a = 0.0;

  for(int i=0;i<3;i++) {
    a   += normal[i]*P[i+3];
    Sum += normal[i]*P[i]; 
  }
  if(a==0.0) return Step;
 
  Step = -Sum/a;
 
  return Step;
}

int TrigInDetTrackFollowingTool::RungeKutta34(double* P, double* J, const Trk::PlaneSurface* pN, MagField::AtlasFieldCache& fieldCache, bool withJacobian) const {

  const double coeff            = 299.7;
  const double min_step         = 3.0;
  const double const_field_step = 30.0;
  const double maxPath          = 3000.0;
  const double minQp            = 0.01; //100 MeV cut
  const double minRad           = 300.0;

  const int maxIter = 10;

  double exStep = 0.0;

  if(std::fabs(P[6]) > minQp) return -1;
  
  double Step = estimateRK_Step(pN, P);

  if(Step > 1e7) return -2;

  double absStep   = fabs(Step);

  if(absStep <= min_step) {
    for(int i=0;i<3;i++) P[i] += Step*P[i+3];
    P[7] += Step; 
    return 0;
  }

  if(fabs(P[6]*Step) > minRad) {
    Step = (Step > 0.0 ? minRad : -minRad)/fabs(P[6]);
  }

  int nFlips = 0;

  double mom = P[6];

  double Y[6];

  memcpy(&Y[0], P, sizeof(Y));

  double B[3]; 

  fieldCache.getField(Y, B);

  for(int i=0;i<3;i++) B[i] *= coeff;

  if(exStep != 0) {
    if(absStep > fabs(exStep))
      Step = exStep;
  }

  for(int iter=0;iter<maxIter;iter++) {//solving single-value boundary problem

    bool useConstField = fabs(Step) < const_field_step;

    if(!useConstField) {
      fieldCache.getField(Y, B);
      for(int i=0;i<3;i++) B[i] *= coeff;
    }
    
    double B2[3], B3[3];

    double H = Step;
    double H3 = H/3;
    double H23 = 2*H3;
    double H4 = 0.25*H;
    double H34 = 3*H4;

    double H3mom =  mom*H3;
    double H23mom = mom*H23;
    double H4mom =  mom*H4;
    double H34mom = mom*H34;

    double YB[3];

    crossProduct(B, Y+3, YB);

    //second point

    double Y2[6];

    for(int i=0;i<3;i++) Y2[i] = Y[i] + H3*Y[i+3];
    for(int i=3;i<6;i++) Y2[i] = Y[i] + H3mom*YB[i-3];

    double YB2[3];
 
    if(useConstField) {
      crossProduct(B, Y2+3, YB2);
    }
    else {
      fieldCache.getField(Y2, B2);
      for(int i=0;i<3;i++) B2[i] *= coeff;
      crossProduct(B2, Y2+3, YB2);
    }

    //last point

    double Y3[6];

    for(int i=0;i<3;i++) Y3[i] = Y[i] + H23*Y2[i+3];
    for(int i=3;i<6;i++) Y3[i] = Y[i] + H23mom*YB2[i-3];

    double YB3[3];
    
    if(useConstField) {
      crossProduct(B, Y3+3, YB3);
    }
    else {
      fieldCache.getField(Y3, B3);
      for(int i=0;i<3;i++) B3[i] *= coeff;
      crossProduct(B3, Y3+3, YB3);
    }

    double Y1[6];

    for(int i=3;i<6;i++) Y1[i-3]  = Y[i-3] + H4*(Y[i] + 3*Y3[i]);
    for(int i=0;i<3;i++) Y1[i+3]  = Y[i+3] + H4mom*(YB[i] + 3*YB3[i]);

    if(fabs(Y1[5])>1) return -10;

    //Jacobian calculations go here ...

    if(withJacobian) {
      
      double J1C[9], L2C[9], J2C[9], L3C[9], J3C[9];
    
      double CqB3H34[3];
      double CqB2H23[3];
      double CqBH3[3];

      if(!useConstField) {    
	for(int i=0;i<3;i++) CqBH3[i]   = H3mom*B[i];
	for(int i=0;i<3;i++) CqB2H23[i] = H23mom*B2[i];
	for(int i=0;i<3;i++) CqB3H34[i] = H34mom*B3[i];
      }
      else {
	for(int i=0;i<3;i++) CqBH3[i]   = H3mom*B[i];
	for(int i=0;i<3;i++) CqB2H23[i] = H23mom*B[i];
	for(int i=0;i<3;i++) CqB3H34[i] = H34mom*B[i];
      }

      crossProduct(CqBH3, J+17, J1C);
      crossProduct(CqBH3, J+24, J1C+3);
      crossProduct(CqBH3, J+31, J1C+6);
      
      J1C[6] += H3*YB[0];
      J1C[7] += H3*YB[1];
      J1C[8] += H3*YB[2];
    
      L2C[0] = J[17] + J1C[0];
      L2C[1] = J[18] + J1C[1];
      L2C[2] = J[19] + J1C[2];

      L2C[3] = J[24] + J1C[3];
      L2C[4] = J[25] + J1C[4];
      L2C[5] = J[26] + J1C[5];

      L2C[6] = J[31] + J1C[6];
      L2C[7] = J[32] + J1C[7];
      L2C[8] = J[33] + J1C[8];
      
      crossProduct(CqB2H23, L2C,   J2C);
      crossProduct(CqB2H23, L2C+3, J2C+3);
      crossProduct(CqB2H23, L2C+6, J2C+6);

      J2C[6] += H23*YB2[0];
      J2C[7] += H23*YB2[1];
      J2C[8] += H23*YB2[2];
 
      L3C[0] = J[17] + J2C[0];
      L3C[1] = J[18] + J2C[1];
      L3C[2] = J[19] + J2C[2];

      L3C[3] = J[24] + J2C[3];
      L3C[4] = J[25] + J2C[4];
      L3C[5] = J[26] + J2C[5];

      L3C[6] = J[31] + J2C[6];
      L3C[7] = J[32] + J2C[7];
      L3C[8] = J[33] + J2C[8];
    
      crossProduct(CqB3H34, L3C,   J3C);
      crossProduct(CqB3H34, L3C+3, J3C+3);
      crossProduct(CqB3H34, L3C+6, J3C+6);

      J3C[6] += H34*YB3[0];
      J3C[7] += H34*YB3[1];
      J3C[8] += H34*YB3[2];
    
      for(int i=0;i<9;i++) J1C[i] = 0.75*J1C[i] + J3C[i];
      
      for(int i=0;i<9;i++) J2C[i] *= H34;
      
      J[14] += H*J[17];
      J[15] += H*J[18];
      J[16] += H*J[19];
      
      J[21] += H*J[24];
      J[22] += H*J[25];
      J[23] += H*J[26];
      
      J[28] += H*J[31];
      J[29] += H*J[32];
      J[30] += H*J[33];
      
      J[14] += J2C[0];
      J[15] += J2C[1];
      J[16] += J2C[2];

      J[21] += J2C[3];
      J[22] += J2C[4];
      J[23] += J2C[5];

      J[28] += J2C[6];
      J[29] += J2C[7];
      J[30] += J2C[8];

      J[17] += J1C[0];
      J[18] += J1C[1];
      J[19] += J1C[2];

      J[24] += J1C[3];
      J[25] += J1C[4];
      J[26] += J1C[5];

      J[31] += J1C[6];
      J[32] += J1C[7];
      J[33] += J1C[8];

    }

    P[7] += Step;
    
    if(fabs(P[7]) > maxPath) return -3;

    double norm = 1/std::sqrt(Y1[3]*Y1[3]+Y1[4]*Y1[4]+Y1[5]*Y1[5]);

    Y1[3] *= norm; Y1[4] *= norm; Y1[5] *= norm;

    double newStep = estimateRK_Step(pN, Y1);

    if(newStep > 1e7) return -4;
    
    double absNewStep = fabs(newStep);

    if(absNewStep <= min_step) {//the boundary is too close, using straight line
      
      if(withJacobian) {
	if(!useConstField) { 
	  crossProduct(B3, Y1+3, J+35);
	}
	else {
	  crossProduct(B, Y1+3, J+35);
	}
      }

      for(int i=0;i<3;i++) {
	P[i+3] = Y1[i+3];
	P[i] = Y1[i] + newStep*Y1[i+3];
      }
      P[7] += newStep; 

      return 0;  
    }

    double absStep = fabs(Step);

    if(Step*newStep < 0.0) {//the boundary is overshot
      if(++nFlips > 2) return -5;//oscillations
      Step =  absNewStep < absStep ? newStep : -Step;
    }
    else {
      if(absNewStep < absStep) Step = newStep;
    }
    
    for(int i=0;i<6;i++) Y[i] = Y1[i];
  }
  
  return -11;//max. number of iteration reached

}

void TrigInDetTrackFollowingTool::crossProduct(double const * B, double const * V, double* A) const {
  A[0] = -B[1]*V[2] + B[2]*V[1];
  A[1] =  B[0]*V[2] - B[2]*V[0];
  A[2] = -B[0]*V[1] + B[1]*V[0];
}
