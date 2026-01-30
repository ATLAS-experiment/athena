/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//  Convert TrkTrack parameters to internal VKalVrt parameters
// and sets up common reference system for ALL tracks
// even if in the beginning in was different

//------------------------------------------------------------------
// Header include
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVKalVrtCore/TrkVKalVrtCore.h"

#include <iostream>

namespace Trk{

  //--------------------------------------------------------------------
  //
  // Use perigee ONLY!!!
  // Then in normal conditions reference frame is always (0,0,0)
  //

  StatusCode
  TrkVKalVrtFitter::CvtPerigee(const std::vector<const Perigee*>& InpPerigee,
			       int& ntrk,
			       State& state) const
  {

    double tmp_refFrameX = 0, tmp_refFrameY = 0, tmp_refFrameZ = 0;

    //
    // ----- Set reference frame to (0.,0.,0.) == ATLAS frame
    // ----- Magnetic field is taken in reference point
    //
    state.m_refFrameX = state.m_refFrameY = state.m_refFrameZ = 0.;
    state.m_fitField.setAtlasMagRefFrame( 0., 0., 0.);

    //
    //  Cycle to determine common reference point for the fit
    //
    int counter =0;
    state.m_trkControl.clear();
    for (const auto& mPer : InpPerigee) {
      if( mPer == nullptr ){ continue; }

      // Global position of perigee point
      Amg::Vector3D perGlobalPos =  mPer->position();
      // Crazy user protection
      if(!(state.m_allowUltraDisplaced) && std::abs(perGlobalPos.z()) > m_IDsizeZ) return StatusCode::FAILURE;
      if(!(state.m_allowUltraDisplaced) && perGlobalPos.perp() > m_IDsizeR) return StatusCode::FAILURE;

      // Reference system calculation
      // Use hit position itself to get more precise magnetic field
      tmp_refFrameX += perGlobalPos.x() ;
      tmp_refFrameY += perGlobalPos.y() ;
      tmp_refFrameZ += perGlobalPos.z() ;

      TrkMatControl tmpMat;
      tmpMat.trkRefGlobPos = Amg::Vector3D(perGlobalPos.x(),
					   perGlobalPos.y(),
					   perGlobalPos.z());
      // Perigee point strategy
      tmpMat.extrapolationType = 2;
      tmpMat.TrkPnt = mPer;
      tmpMat.prtMass = ParticleConstants::chargedPionMassInMeV;
      if(counter < static_cast<int>(state.m_MassInputParticles.size())){
	tmpMat.prtMass = state.m_MassInputParticles[counter];
      }
      tmpMat.trkSavedLocalVertex.setZero();
      tmpMat.TrkID=counter;
      state.m_trkControl.push_back(tmpMat);
      counter++;
    }

    if(counter == 0) return StatusCode::FAILURE;

    // Reference frame for the fit
    tmp_refFrameX /= counter;
    tmp_refFrameY /= counter;
    tmp_refFrameZ /= counter;

    //
    //  Common reference frame is ready. Start extraction of parameters for fit.
    //
    double fx = 0., fy = 0., fz = 0.;
    for (const auto& mPer : InpPerigee) {
      if(mPer == nullptr){ continue; }
      AmgVector(5) VectPerig = mPer->parameters();
      // Global position of perigee point
      Amg::Vector3D perGlobalPos =  mPer->position();
      // Global position of reference point
      Amg::Vector3D perGlobalVrt =  mPer->associatedSurface().center();
      // Restore ATLAS frame
      state.m_refFrameX = state.m_refFrameY = state.m_refFrameZ = 0.;
      state.m_fitField.setAtlasMagRefFrame(0., 0., 0.);
      // Magnetic field at perigee point
      state.m_fitField.getMagFld(perGlobalPos.x(),
				 perGlobalPos.y(),
				 perGlobalPos.z(),
				 fx, fy, fz);
      double effectiveBMAG=state.m_fitField.getEffField(fx, fy, fz, VectPerig[2], VectPerig[3]);
      if(std::abs(effectiveBMAG) < 0.01) effectiveBMAG = 0.01;

      double CovVertTrk[15];
      std::fill(CovVertTrk,CovVertTrk+15,0.);
      // No good covariance matrix!
      if(!convertAmg5SymMtx(mPer->covariance(), CovVertTrk)) return StatusCode::FAILURE;
      VKalTransform(effectiveBMAG,
                    static_cast<double>(VectPerig(0)),
                    static_cast<double>(VectPerig(1)),
                    static_cast<double>(VectPerig(2)),
                    static_cast<double>(VectPerig(3)),
                    static_cast<double>(VectPerig(4)),
                    CovVertTrk,
                    state.m_ich[ntrk], &state.m_apar[ntrk][0],
                    &state.m_awgt[ntrk][0]);

      // Check if propagation to common reference point is needed and make it
      // initial track reference position
      state.m_refFrameX=perGlobalVrt.x();
      state.m_refFrameY=perGlobalVrt.y();
      state.m_refFrameZ=perGlobalVrt.z();
      state.m_fitField.setAtlasMagRefFrame(state.m_refFrameX,
					   state.m_refFrameY,
					   state.m_refFrameZ);
      double dX = tmp_refFrameX-perGlobalVrt.x();
      double dY = tmp_refFrameY-perGlobalVrt.y();
      double dZ = tmp_refFrameZ-perGlobalVrt.z();
      if(std::abs(dX)+std::abs(dY)+std::abs(dZ) != 0.) {
	double pari[5], covi[15];
	double vrtini[3] = {0.,0.,0.};
	double vrtend[3] = {dX,dY,dZ};
	for(int i=0; i<5; i++) pari[i] = state.m_apar[ntrk][i];
	for(int i=0; i<15;i++) covi[i] = state.m_awgt[ntrk][i];
	long int Charge = (long int) mPer->charge();
	long int TrkID = ntrk;
	Trk::vkalPropagator::Propagate(TrkID, Charge, pari, covi,
			       vrtini, vrtend, &state.m_apar[ntrk][0],
			       &state.m_awgt[ntrk][0],
			       &state.m_vkalFitControl);
      }

      ntrk++;
      if(ntrk>=NTrMaxVFit) return StatusCode::FAILURE;
    }

    //-------------- Finally setting new reference frame common for ALL tracks
    state.m_refFrameX = tmp_refFrameX;
    state.m_refFrameY = tmp_refFrameY;
    state.m_refFrameZ = tmp_refFrameZ;
    state.m_fitField.setAtlasMagRefFrame(state.m_refFrameX,
					 state.m_refFrameY,
					 state.m_refFrameZ);

    return StatusCode::SUCCESS;
  }

  std::unique_ptr<Perigee>
  TrkVKalVrtFitter::CreatePerigee(const std::vector<double>& VKPerigee,
				  const std::vector<double>& VKCov,
				  IVKalState& istate) const
  {
    assert(dynamic_cast<const State*> (&istate)!=nullptr);
    State& state = static_cast<State&> (istate);
    return CreatePerigee(0., 0., 0., VKPerigee, VKCov, state);
  }

  // Function creates a Trk::Perigee on the heap
  //  Don't forget to remove it after use
  //  vX,vY,vZ are in LOCAL SYSTEM with respect to refGVertex
  std::unique_ptr<Perigee>
  TrkVKalVrtFitter::CreatePerigee(double vX, double vY, double vZ,
				  const std::vector<double>& VKPerigee,
				  const std::vector<double>& VKCov,
				  State& state) const
  {

    // ------  Magnetic field access
    double fx = 0., fy = 0., fz = 0.;
    state.m_fitField.getMagFld(vX,vY,vZ,fx,fy,fz);
    double effectiveBMAG=state.m_fitField.getEffField(fx, fy, fz, VKPerigee[3], VKPerigee[2]);
    if(std::abs(effectiveBMAG) < 0.01) effectiveBMAG=0.01;  //safety

    double TrkP3 = 0., TrkP4 = 0., TrkP5 = 0.;
    VKalToTrkTrack(effectiveBMAG, VKPerigee[2], VKPerigee[3], VKPerigee[4],
		   TrkP3, TrkP4, TrkP5);
    double TrkP1 = -VKPerigee[0];   /*!!!! Change of sign !!!!*/
    double TrkP2 = VKPerigee[1];
    TrkP5 = -TrkP5;                  /*!!!! Change of sign of charge!!!!*/

    AmgSymMatrix(5) CovMtx;
    double Deriv[5][5],CovMtxOld[5][5];
    for(int i=0; i<5; i++){
      for(int j=0; j<5; j++){
	Deriv[i][j]=0.;
	CovMtxOld[i][j]=0.;
      }
    }
    Deriv[0][0] = -1.;
    Deriv[1][1] =  1.;
    Deriv[2][3] =  1.;
    Deriv[3][2] =  1.;
    Deriv[4][2] =  (std::cos(VKPerigee[2])/(m_CNVMAG*effectiveBMAG)) * VKPerigee[4];
    Deriv[4][4] = -(std::sin(VKPerigee[2])/(m_CNVMAG*effectiveBMAG));

    CovMtxOld[0][0]                   = VKCov[0];
    CovMtxOld[0][1] = CovMtxOld[1][0] = VKCov[1];
    CovMtxOld[1][1]                   = VKCov[2];
    CovMtxOld[0][2] = CovMtxOld[2][0] = VKCov[3];
    CovMtxOld[1][2] = CovMtxOld[2][1] = VKCov[4];
    CovMtxOld[2][2]                   = VKCov[5];
    CovMtxOld[0][3] = CovMtxOld[3][0] = VKCov[6];
    CovMtxOld[1][3] = CovMtxOld[3][1] = VKCov[7];
    CovMtxOld[2][3] = CovMtxOld[3][2] = VKCov[8];
    CovMtxOld[3][3]                   = VKCov[9];
    CovMtxOld[0][4] = CovMtxOld[4][0] = VKCov[10];
    CovMtxOld[1][4] = CovMtxOld[4][1] = VKCov[11];
    CovMtxOld[2][4] = CovMtxOld[4][2] = VKCov[12];
    CovMtxOld[3][4] = CovMtxOld[4][3] = VKCov[13];
    CovMtxOld[4][4]                   = VKCov[14];

    for(int i=0; i<5; i++){
     for(int j=i; j<5; j++){
       double tmp=0.;
       for(int ik=4; ik>=0; ik--){
         if(Deriv[i][ik]==0.)continue;
         for(int jk=4; jk>=0; jk--){
           if(Deriv[j][jk]==0.)continue;
           tmp += Deriv[i][ik]*CovMtxOld[ik][jk]*Deriv[j][jk];
	 }
       }
       CovMtx(i,j) = CovMtx(j,i)=tmp;
     }
    }

    auto surface = PerigeeSurface(Amg::Vector3D(state.m_refFrameX+vX,
						state.m_refFrameY+vY,
						state.m_refFrameZ+vZ));

    return  std::make_unique<Perigee>(TrkP1, TrkP2, TrkP3, TrkP4, TrkP5,
				      surface,
				      std::move(CovMtx));
  }

} // end of namespace



