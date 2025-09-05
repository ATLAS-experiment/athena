/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>



#include "SiSPSeededTrackFinderData/ITkSiSpacePointForSeed.h"

#include "InDetPrepRawData/SiCluster.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "TrkSpacePoint/SpacePoint.h"
#include "TrkSurfaces/Surface.h"
#include "CxxUtils/restrict.h"
namespace ITk
{

  SiSpacePointForSeed::SiSpacePointForSeed
  (const Trk::SpacePoint* sp,std::span<float const, 15> r) 
  {
    set(sp,r); m_param = 0.;
  }

  SiSpacePointForSeed::SiSpacePointForSeed
  (const Trk::SpacePoint* sp,std::span<float const, 15> r,std::span<float const, 15> sc) 
  {
    set(sp,r,sc); m_param = 0.;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Set
  /////////////////////////////////////////////////////////////////////////////////

  void SiSpacePointForSeed::set
  (const Trk::SpacePoint* sp,std::span<float const, 15> r)
  {
    spacepoint = sp  ;
    m_x        = r[0];
    m_y        = r[1];
    m_z        = r[2];
    m_r        =std::sqrt(m_x*m_x+m_y*m_y);
    m_q        = 100000.;

    const InDet::SiCluster*           c  = static_cast<const InDet::SiCluster*>(sp->clusterList().first);
    const InDetDD::SiDetectorElement* de = c ->detectorElement();

    if( de->isPixel() ) {
      
      const Amg::MatrixX& v =  c->localCovariance();
      float f22 = float(v(1,1) );
      float wid = float(c->width().z());
      float cov = wid*wid*.08333f; if(cov < f22) cov = f22;
      m_sn = nullptr;
      cov*=6.;
      m_covz = cov*(r[3]*r[3]+r[4]*r[4]); 
      m_covr = cov*(r[5]*r[5]);
    }
    else                {

      const Amg::MatrixX& v = sp->localCovariance();
      float f22 = float(v(1,1));
      if(de->isBarrel()) {m_covz = 8.f*f22; m_covr = .1f;} 
      else               {m_covr = 8.f*f22; m_covz = .1f;} 
      m_sn =  &sp->clusterList().second->detectorElement()->surface();
    
      for(int i=0; i!=3; ++i) {
        m_b0[i]=r[3 +i]; m_b1[i]=r[6 +i]; m_dr[i]=r[9 +i]; m_r0[i]=r[12+i];
      }
    }
    m_su = &sp->clusterList().first->detectorElement()->surface();
  } 


  /////////////////////////////////////////////////////////////////////////////////
  // Set with error correction 
  // sc[0] - barrel pixels error correction
  // sc[1] - endcap pixels 
  // sc[2] - barrel sct
  // sc[3] - endcap sct 
  /////////////////////////////////////////////////////////////////////////////////

  void SiSpacePointForSeed::set
  (const Trk::SpacePoint* sp,std::span<float const, 15> r, std::span<float const, 15> sc)
  {
    m_sn = nullptr;
    spacepoint = sp  ;
    m_x        = r[0];
    m_y        = r[1];
    m_z        = r[2];
    m_r        =std::sqrt(m_x*m_x+m_y*m_y);
    m_q        = 100000.;

    const InDet::SiCluster*           c  = static_cast<const InDet::SiCluster*>(sp->clusterList().first);
    const InDetDD::SiDetectorElement* de = c ->detectorElement();

    if( de->isPixel() ) {
      const Amg::MatrixX& v =  c->localCovariance();
      float f22 = float(v(1,1));
      float wid = float(c->width().z());
      float cov = wid*wid*.08333f; if(cov < f22) cov = f22;
      cov*=6.;
      m_covz = cov*(r[3]*r[3]+r[4]*r[4]); 
      m_covr = cov*(r[5]*r[5]);
    } else {
      const Amg::MatrixX& v = sp->localCovariance();
      float f22 = float(v(1,1));
      if(de->isBarrel()) {m_covz = 8.f*f22*sc[2]; m_covr = .1f;} 
      else               {m_covr = 8.f*f22*sc[3]; m_covz = .1f;} 
      for(int i=0; i!=3; ++i) {m_b0[i]=r[3 +i]; m_b1[i]=r[6 +i]; m_dr[i]=r[9 +i]; m_r0[i]=r[12+i];}
    }
    m_su = &sp->clusterList().first->detectorElement()->surface();
  }

  void  SiSpacePointForSeed::setQuality(float q)
  {
    if(q <= m_q) m_q = q;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Coordinate of cross points two SCT strip calculation for given direction
  // d - input direction
  // r - output coordinates
  // true if cross point is inside detector elements 
  /////////////////////////////////////////////////////////////////////////////////

  bool SiSpacePointForSeed::coordinates(const float* d,float* ATH_RESTRICT r)
  {
    float d1[3] = {m_b0[1]*d[2]-m_b0[2]*d[1],m_b0[2]*d[0]-m_b0[0]*d[2],m_b0[0]*d[1]-m_b0[1]*d[0]};
    float bd    =  m_b1[0]*d1[0]+m_b1[1]*d1[1]+m_b1[2]*d1[2];     

    if(std::abs(m_dr[0]*d1[0]+m_dr[1]*d1[1]+m_dr[2]*d1[2]) > std::abs(bd)*1.1) return false;

    float s0    = m_dr[0]*(m_b1[1]*d[2]-m_b1[2]*d[1])+
	          m_dr[1]*(m_b1[2]*d[0]-m_b1[0]*d[2])+
                  m_dr[2]*(m_b1[0]*d[1]-m_b1[1]*d[0]);
    if(std::abs(s0) > std::abs(bd)*1.1) return false;

    s0/=bd;
    r[0] = m_r0[0]+m_b0[0]*s0;
    r[1] = m_r0[1]+m_b0[1]*s0;
    r[2] = m_r0[2]+m_b0[2]*s0;
    return true;
  }

} // end of name space ITk
