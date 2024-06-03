/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVKalVrtCore/VKvFast.h"
//-------------------------------------------------
//
#include "TrkTrack/TrackInfo.h"
#include <vector>
#include <algorithm> //for nth_element, max_element
#include <cmath> //for abs

namespace{
  double
  median(std::vector<double> & v){ //modifies the vector v
    //assume we don't need to check for empty vector
    const auto n = v.size();
    const auto halfway =n/2;
    //its a contiguous iterator, just use addition
    std::vector<double>::iterator it = v.begin() + halfway;
    std::nth_element(v.begin(), it, v.end());
    if (n & 1){//it has an odd number of elements
      return *it;//so just return the middle element
    } else {
      const double mid2 = *it;
      //the following works because v is partially sorted by std::nth_element
      const double mid1 = *std::max_element(v.begin(), it);
      return (mid1 + mid2) *0.5; //return the average of the two neighbouring mid elements
    }
  }
}

//
//__________________________________________________________________________
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


namespace Trk{

//-----------------------------------------------------------------------------------------
//  Standard (old) code
//


  StatusCode TrkVKalVrtFitter::VKalVrtFitFast(std::span<const xAOD::TrackParticle* const> InpTrk,
                                              Amg::Vector3D& Vertex,
                                              IVKalState& istate) const
  {
    double minDZ=0.;
    return VKalVrtFitFast(InpTrk,Vertex,minDZ,istate);
  }


  StatusCode TrkVKalVrtFitter::VKalVrtFitFast(std::span<const xAOD::TrackParticle* const> InpTrk,
                                              Amg::Vector3D& Vertex, double & minDZ,
                                              IVKalState& istate) const
  {
    assert(dynamic_cast<State*> (&istate)!=nullptr);
    State& state = static_cast<State&> (istate);
//
//  Convert particles and setup reference frame
//
    int ntrk=0;
    StatusCode sc = CvtTrackParticle(InpTrk,ntrk,state);
    if(sc.isFailure() || ntrk<1 ) return StatusCode::FAILURE;
    double fx,fy,BMAG_CUR;
    state.m_fitField.getMagFld(0.,0.,0.,fx,fy,BMAG_CUR);
    if(fabs(BMAG_CUR) < 0.1) BMAG_CUR=0.1;
//
//------ Variables and arrays needed for fitting kernel
//
    double out[3];
    std::vector<double> xx,yy,zz,difz;
    Vertex[0]=Vertex[1]=Vertex[2]=0.;
//
//
    double xyz0[3]={ -state.m_refFrameX, -state.m_refFrameY, -state.m_refFrameZ};
    if(ntrk==2){
      minDZ=Trk::vkvFastV(&state.m_apar[0][0],&state.m_apar[1][0], xyz0, BMAG_CUR, out);
    } else {
      for(int i=0;i<ntrk-1; i++){
        for(int j=i+1; j<ntrk;   j++){
          double dZ=Trk::vkvFastV(&state.m_apar[i][0],&state.m_apar[j][0], xyz0, BMAG_CUR, out);
          xx.push_back(out[0]);
          yy.push_back(out[1]);
          zz.push_back(out[2]);
          difz.push_back(dZ);
        }
      }
      out[0] = median(xx);
      out[1] = median(yy);
      out[2] = median(zz);
      minDZ  = median(difz);
    }
    Vertex[0]= out[0] + state.m_refFrameX;
    Vertex[1]= out[1] + state.m_refFrameY;
    Vertex[2]= out[2] + state.m_refFrameZ;


    return StatusCode::SUCCESS;
  }



  StatusCode TrkVKalVrtFitter::VKalVrtFitFast(const std::vector<const TrackParameters*>& InpTrk,
                                              Amg::Vector3D& Vertex,
                                              IVKalState& istate) const
  {
    assert(dynamic_cast<State*> (&istate)!=nullptr);
    State& state = static_cast<State&> (istate);
//
//  Convert particles and setup reference frame
//
    int ntrk=0;
    StatusCode sc = CvtTrackParameters(InpTrk,ntrk,state);
    if(sc.isFailure() || ntrk<1 ) return StatusCode::FAILURE;
    double fx,fy,BMAG_CUR;
    state.m_fitField.getMagFld(0.,0.,0.,fx,fy,BMAG_CUR);
    if(fabs(BMAG_CUR) < 0.1) BMAG_CUR=0.1;
//
//------ Variables and arrays needed for fitting kernel
//
    double out[3];
    std::vector<double> xx,yy,zz;
    Vertex[0]=Vertex[1]=Vertex[2]=0.;
//
//
    double xyz0[3]={ -state.m_refFrameX, -state.m_refFrameY, -state.m_refFrameZ};
    if(ntrk==2){
      Trk::vkvFastV(&state.m_apar[0][0],&state.m_apar[1][0], xyz0, BMAG_CUR, out);
    } else {
      for(int i=0;i<ntrk-1; i++){
	      for(int j=i+1; j<ntrk;   j++){
          Trk::vkvFastV(&state.m_apar[i][0],&state.m_apar[j][0], xyz0, BMAG_CUR, out);
	        xx.push_back(out[0]);
	        yy.push_back(out[1]);
	        zz.push_back(out[2]);
	      }
      }
      out[0] = median(xx);
      out[1] = median(yy);
      out[2] = median(zz);

    }
    Vertex[0]= out[0] + state.m_refFrameX;
    Vertex[1]= out[1] + state.m_refFrameY;
    Vertex[2]= out[2] + state.m_refFrameZ;


    return StatusCode::SUCCESS;
  }


}

