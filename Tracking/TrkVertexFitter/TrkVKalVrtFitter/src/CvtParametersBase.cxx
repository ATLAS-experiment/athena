/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Convert TrackParameters and NeutralParameters to internal VKalVrt parameters
// and sets up common reference system for ALL tracks
// even if in the beginning in was different
//------------------------------------------------------------------
// Header include
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
//-------------------------------------------------
// Other stuff
//----
#include  "TrkParameters/TrackParameters.h"
#include <iostream>

namespace Trk {

  //--------------------------------------------------------------------
  //  Extract TrackParameters
  //

  StatusCode
  TrkVKalVrtFitter::CvtTrackParameters(const std::vector<const TrackParameters*>& InpTrk,
				       int& ntrk,
				       State& state) const
  {

    std::vector<const TrackParameters*>::const_iterator i_pbase;
    AmgVector(5) VectPerig;
    VectPerig.setZero();
    Amg::Vector3D perGlobalPos,perGlobalVrt;
    const Trk::Perigee* mPer=nullptr;

    double tmp_refFrameX = 0, tmp_refFrameY = 0, tmp_refFrameZ = 0;
    double rxyMin = 1000000.;

    //
    // ----- Set reference frame to (0.,0.,0.) == ATLAS frame
    // ----- Magnetic field is taken in reference point
    //
    state.m_refFrameX=state.m_refFrameY=state.m_refFrameZ=0.;
    state.m_fitField.setAtlasMagRefFrame( 0., 0., 0.);

    if( m_InDetExtrapolator == nullptr ){
      if(msgLvl(MSG::WARNING))msg()<< "No InDet extrapolator given. Can't use TrackParameters!!!" << endmsg;
      return StatusCode::FAILURE;
    }

    //
    //  Cycle to determine common reference point for the fit
    //
    int counter =0;
    state.m_trkControl.clear();
    state.m_trkControl.reserve(InpTrk.size());
    for (i_pbase = InpTrk.begin(); i_pbase != InpTrk.end(); ++i_pbase) {
      // Global position of hit
      perGlobalPos = (*i_pbase)->position();
      // Crazy user protection
      if(!(state.m_allowUltraDisplaced) && std::abs(perGlobalPos.z()) > m_IDsizeZ) return StatusCode::FAILURE;
      if(!(state.m_allowUltraDisplaced) && perGlobalPos.perp() > m_IDsizeR) return StatusCode::FAILURE;
      tmp_refFrameX += perGlobalPos.x();
      tmp_refFrameY += perGlobalPos.y();
      tmp_refFrameZ += perGlobalPos.z();

      // Here we create structure to control material effects
      TrkMatControl tmpMat;
      tmpMat.trkSavedLocalVertex.setZero();
      tmpMat.trkRefGlobPos = Amg::Vector3D(perGlobalPos.x(),
					   perGlobalPos.y(),
					   perGlobalPos.z());
      // First measured point strategy
      tmpMat.extrapolationType = m_firstMeasuredPoint ? 0 : 1;
      tmpMat.TrkPnt = (*i_pbase);
      tmpMat.prtMass = ParticleConstants::chargedPionMassInMeV;
      if(counter < (int)state.m_MassInputParticles.size()){
	tmpMat.prtMass = state.m_MassInputParticles[counter];
      }
      tmpMat.TrkID = counter;
      state.m_trkControl.push_back(tmpMat);
      counter++;

      if(perGlobalPos.perp() < rxyMin){
	rxyMin=perGlobalPos.perp();
	state.m_globalFirstHit=(*i_pbase);
      }
    }

    if(counter == 0) return StatusCode::FAILURE;
    // Reference frame for the fit based on hits positions
    tmp_refFrameX /= counter;
    tmp_refFrameY /= counter;
    tmp_refFrameZ /= counter;
    Amg::Vector3D refGVertex(tmp_refFrameX, tmp_refFrameY, tmp_refFrameZ);

    double fx, fy, fz;

    //
    //  Common reference frame is ready. Start extraction of parameters for fit.
    //  TracksParameters are extrapolated to common point and converted to Perigee
    //  This is needed for VKalVrtCore engine.
    //

    double CovVertTrk[15];
    std::fill(CovVertTrk, CovVertTrk+15, 0.);

    for (i_pbase = InpTrk.begin(); i_pbase != InpTrk.end(); ++i_pbase) {
       long int TrkID=ntrk;
       const TrackParameters* trkparO = (*i_pbase);

       if(trkparO){
         const Trk::TrackParameters* trkparN =
	   m_fitPropagator->myExtrapWithMatUpdate(TrkID,
						  trkparO,
						  &refGVertex,
						  state);
         if(trkparN == nullptr) return StatusCode::FAILURE;
         mPer = dynamic_cast<const Trk::Perigee*>(trkparN);
         if(mPer == nullptr) {
           delete trkparN;
           return StatusCode::FAILURE;
         }

         VectPerig = mPer->parameters();
         // Global position of perigee point
         perGlobalPos =  mPer->position();
         // Global position of reference point
         perGlobalVrt =  mPer->associatedSurface().center();
         // VK no good covariance matrix!
         if( !convertAmg5SymMtx(mPer->covariance(), CovVertTrk) ) return StatusCode::FAILURE;
         delete trkparN;
       }

       state.m_refFrameX = state.m_refFrameY = state.m_refFrameZ = 0.;
       // Restore ATLAS frame for safety
       state.m_fitField.setAtlasMagRefFrame(0., 0., 0.);
       // Magnetic field at perigee point
       state.m_fitField.getMagFld(perGlobalPos.x(), perGlobalPos.y(), perGlobalPos.z(),
				  fx, fy, fz);
       double effectiveBMAG=state.m_fitField.getEffField(fx, fy, fz, VectPerig[2], VectPerig[3]);
       if(std::abs(effectiveBMAG) < 0.01) effectiveBMAG = 0.01;

       VKalTransform(effectiveBMAG,
		     (double)VectPerig[0], (double)VectPerig[1],
		     (double)VectPerig[2], (double)VectPerig[3],
		     (double)VectPerig[4], CovVertTrk,
                     state.m_ich[ntrk], &state.m_apar[ntrk][0],
		     &state.m_awgt[ntrk][0]);

       // Neutral track
       if( trkparO==nullptr ) {
         state.m_ich[ntrk]=0;
         if(state.m_apar[ntrk][4]<0){
           // Charge=0 is always equal to Charge=+1
           state.m_apar[ntrk][4]  = -state.m_apar[ntrk][4];
           state.m_awgt[ntrk][10] = -state.m_awgt[ntrk][10];
           state.m_awgt[ntrk][11] = -state.m_awgt[ntrk][11];
           state.m_awgt[ntrk][12] = -state.m_awgt[ntrk][12];
           state.m_awgt[ntrk][13] = -state.m_awgt[ntrk][13];
         }
       }
       ntrk++;
       if(ntrk>=NTrMaxVFit) return StatusCode::FAILURE;
    }

    //-------------- Finally setting new reference frame common for ALL tracks
    state.m_refFrameX = tmp_refFrameX;
    state.m_refFrameY = tmp_refFrameY;
    state.m_refFrameZ = tmp_refFrameZ;
    state.m_fitField.setAtlasMagRefFrame(state.m_refFrameX, state.m_refFrameY, state.m_refFrameZ);

    return StatusCode::SUCCESS;
  }


  StatusCode
  TrkVKalVrtFitter::CvtNeutralParameters(const std::vector<const NeutralParameters*>& InpTrk,
                                         int& ntrk,
                                         State& state) const
  {

    std::vector<const NeutralParameters*>::const_iterator i_pbase;
    AmgVector(5) VectPerig;
    Amg::Vector3D perGlobalPos,perGlobalVrt;
    const NeutralPerigee* mPerN = nullptr;
    double CovVertTrk[15];
    double tmp_refFrameX = 0, tmp_refFrameY = 0, tmp_refFrameZ = 0;
    double rxyMin = 1000000.;

    //
    // ----- Set reference frame to (0.,0.,0.) == ATLAS frame
    // ----- Magnetic field is taken in reference point
    //
    state.m_refFrameX = state.m_refFrameY = state.m_refFrameZ = 0.;
    state.m_fitField.setAtlasMagRefFrame(0., 0., 0.);

    if( m_InDetExtrapolator == nullptr ){
      if(msgLvl(MSG::WARNING))msg()<< "No InDet extrapolator given. Can't use TrackParameters!!!" << endmsg;
      return StatusCode::FAILURE;
    }

    //
    //  Cycle to determine common reference point for the fit
    //
    int counter = 0;
    state.m_trkControl.clear();
    state.m_trkControl.reserve(InpTrk.size());
    for (i_pbase = InpTrk.begin(); i_pbase != InpTrk.end(); ++i_pbase) {
      // Global position of hit
      perGlobalPos = (*i_pbase)->position();
      // Crazy user protection
      if(std::abs(perGlobalPos.z()) > m_IDsizeZ) return StatusCode::FAILURE;
      if(perGlobalPos.perp() > m_IDsizeR)return StatusCode::FAILURE;

      tmp_refFrameX += perGlobalPos.x() ;
      tmp_refFrameY += perGlobalPos.y() ;
      tmp_refFrameZ += perGlobalPos.z() ;

      // Here we create structure to control material effects
      TrkMatControl tmpMat;
      tmpMat.trkSavedLocalVertex.setZero();
      // on track extrapolation
      tmpMat.trkRefGlobPos = Amg::Vector3D(perGlobalPos.x(), perGlobalPos.y(), perGlobalPos.z());
      // First measured point strategy
      tmpMat.extrapolationType = 0;
      //No reference point for neutral track for the moment !!!
      tmpMat.TrkPnt = nullptr;
      tmpMat.prtMass = ParticleConstants::chargedPionMassInMeV;
      if(counter<(int)state.m_MassInputParticles.size()){
        tmpMat.prtMass = state.m_MassInputParticles[counter];
      }
      tmpMat.TrkID = counter;
      state.m_trkControl.push_back(tmpMat);
      counter++;
      if(perGlobalPos.perp()<rxyMin){
        rxyMin = perGlobalPos.perp();
        state.m_globalFirstHit=nullptr;
      }
    }

    if(counter == 0) return StatusCode::FAILURE;
    // Reference frame for the fit based on hits positions
    tmp_refFrameX /= counter;
    tmp_refFrameY /= counter;
    tmp_refFrameZ /= counter;
    Amg::Vector3D refGVertex (tmp_refFrameX, tmp_refFrameY, tmp_refFrameZ);

    double fx,fy,fz;
    //
    //  Common reference frame is ready. Start extraction of parameters for fit.
    //  TracksParameters are extrapolated to common point and converted to Perigee
    //  This is needed for VKalVrtCore engine.
    //

    for (i_pbase = InpTrk.begin(); i_pbase != InpTrk.end(); ++i_pbase) {
      const Trk::NeutralParameters* neuparO = (*i_pbase);
      if(neuparO == nullptr) return StatusCode::FAILURE;
      const Trk::NeutralParameters* neuparN = m_fitPropagator->myExtrapNeutral(neuparO, &refGVertex);
      mPerN = dynamic_cast<const Trk::NeutralPerigee*>(neuparN);
      if(mPerN == nullptr) {
        delete neuparN;
        return StatusCode::FAILURE;
      }

      VectPerig = mPerN->parameters();
      // Global position of perigee point
      perGlobalPos = mPerN->position();
      // Global position of reference point
      perGlobalVrt = mPerN->associatedSurface().center();
      // VK no good covariance matrix!
      if( !convertAmg5SymMtx(mPerN->covariance(), CovVertTrk) ) return StatusCode::FAILURE;
      delete neuparN;

      state.m_refFrameX = state.m_refFrameY = state.m_refFrameZ = 0.;
      // restore ATLAS frame for safety
      state.m_fitField.setAtlasMagRefFrame( 0., 0., 0.);
      // Magnetic field at perigee point
      state.m_fitField.getMagFld(perGlobalPos.x(), perGlobalPos.y(), perGlobalPos.z(),
                                 fx, fy, fz);
      double effectiveBMAG=state.m_fitField.getEffField(fx, fy, fz, VectPerig[2], VectPerig[3]);
      if(std::abs(effectiveBMAG) < 0.01) effectiveBMAG = 0.01;

      VKalTransform(effectiveBMAG,
                    (double)VectPerig[0], (double)VectPerig[1],
                    (double)VectPerig[2], (double)VectPerig[3],
                    (double)VectPerig[4], CovVertTrk,
                    state.m_ich[ntrk], &state.m_apar[ntrk][0],
                    &state.m_awgt[ntrk][0]);

      state.m_ich[ntrk]=0;
      if(state.m_apar[ntrk][4]<0){
        // Charge=0 is always equal to Charge=+1
        state.m_apar[ntrk][4]  = -state.m_apar[ntrk][4];
        state.m_awgt[ntrk][10] = -state.m_awgt[ntrk][10];
        state.m_awgt[ntrk][11] = -state.m_awgt[ntrk][11];
        state.m_awgt[ntrk][12] = -state.m_awgt[ntrk][12];
        state.m_awgt[ntrk][13] = -state.m_awgt[ntrk][13];
      }
      ntrk++;
      if(ntrk>=NTrMaxVFit) return StatusCode::FAILURE;
    }

    //-------------- Finally setting new reference frame common for ALL tracks
    state.m_refFrameX = tmp_refFrameX;
    state.m_refFrameY = tmp_refFrameY;
    state.m_refFrameZ = tmp_refFrameZ;
    state.m_fitField.setAtlasMagRefFrame(state.m_refFrameX, state.m_refFrameY, state.m_refFrameZ);

    return StatusCode::SUCCESS;
  }

} // end of namespace bracket
