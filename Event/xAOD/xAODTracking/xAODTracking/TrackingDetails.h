/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRACKING_TRACKINGDETAILS_H
#define XAODTRACKING_TRACKINGDETAILS_H

#include "xAODTracking/TrackingPrimitives.h"
#include "Math/Vector4D.h"
#include <span>

namespace xAOD
{
  namespace TrackingDetails
  {
    /// @file TrackingDetails.h
    ///
    /// Helper functions for TrackingDetails accessors
    ///
    /// This file contains helper functions for more complex accessors
    /// in xAOD::TrackParticle, and TrackingHelpers that are shared with
    /// the columnar environment.  These are all implemented as inline
    /// standalone functions that get all the input variables passed in.
    /// That completely separates them from the xAOD classes/functions,
    /// as well as from the columnar environment.
    ///
    /// This is implemented as header-only inline code, which should
    /// give it large flexibility in how it can be used without adding
    /// additional overhead.  If any of these functions is too heavy it
    /// can later on be turned into a non-inlined function.
    ///
    /// @warn None of these functions are meant to be called by the user
    /// directly.  These are meant as the backend implementations of the
    /// corresponding accessors in xAOD::TrackingDetails and the
    /// corresponding columnar accessors.

    /// Base 4 Momentum type for TrackParticle
    using GenVecFourMom_t = ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double> >;

    enum covMatrixIndex{d0_index=0, z0_index=1, phi_index=2, th_index=3, qp_index=4};
    constexpr std::size_t COVMATRIX_OFFDIAG_VEC_COMPR_SIZE = 6;
    constexpr std::array< std::pair<covMatrixIndex,covMatrixIndex>, COVMATRIX_OFFDIAG_VEC_COMPR_SIZE > covMatrixComprIndexPairs {{
      {d0_index,phi_index}, {z0_index,th_index}, {d0_index,qp_index},
      {z0_index,qp_index}, {phi_index,qp_index}, {th_index,qp_index} }};


    [[nodiscard]] inline float charge (float qOverP) {
      return (qOverP > 0) ? 1 : ((qOverP < 0) ? -1 : 0);
    }


    [[nodiscard]] inline GenVecFourMom_t genvecP4(float qOverP, float thetaT, float phiT, double m) {
      using namespace std;
      float p = 10.e6; // 10 TeV (default value for very high pt muons, with qOverP==0)
      if (fabs(qOverP)>0.) p = 1/fabs(qOverP);
      float sinTheta= sin(thetaT);
      float px = p*sinTheta*cos(phiT);
      float py = p*sinTheta*sin(phiT);
      float pz = p*cos(thetaT);
      return GenVecFourMom_t(px, py, pz, m);
    }



    [[nodiscard]] inline bool definingParametersCovMatrixOffDiagCompr(std::span< const float > covMatrixOffDiag) {

      bool flag = false;
      flag = (static_cast< int >(covMatrixOffDiag.size())==COVMATRIX_OFFDIAG_VEC_COMPR_SIZE);
      return flag;
    }



    [[nodiscard]] inline xAOD::ParametersCovMatrix_t definingParametersCovMatrix( std::span< const float > covMatrixDiag, std::span< const float > covMatrixOffDiag, bool& valid ) {

      // Set up the result matrix.
      xAOD::ParametersCovMatrix_t cov;
      cov.setZero();
      valid = true;

      // Set the diagonal elements of the matrix.
      if( ( static_cast< int >( covMatrixDiag.size() ) == cov.rows() ) ) {

        // Access the "raw" variable.
        // Set the diagonal elements using the raw variable.
        for( int i = 0; i < cov.rows(); ++i ) {
          cov( i, i ) = covMatrixDiag[ i ];
        }
      } else {
        valid = false;
        // If the variable is not available/set, set the matrix to identity.
        cov.setIdentity();
      }

      bool offDiagCompr = definingParametersCovMatrixOffDiagCompr(covMatrixOffDiag);

      // Set the off-diagonal elements of the matrix.
      if(!offDiagCompr){

        if( ( static_cast< int >( covMatrixOffDiag.size() ) == ( ( ( cov.rows() - 1 ) * cov.rows() ) / 2 ) ) ) {

          // Set the off-diagonal elements using the raw variable.
          std::size_t vecIndex = 0;
          for( int i = 1; i < cov.rows(); ++i ) {
            for( int j = 0; j < i; ++j, ++vecIndex ) {
              float offDiagCoeff = cov(i,i)>0 && cov(j,j)>0 ? covMatrixOffDiag[vecIndex]*sqrt(cov(i,i)*cov(j,j)) : 0;
              cov.fillSymmetric( i, j, offDiagCoeff );
            }
          }
        }

        else valid = false;

      }

      else{ //Compressed case

        if( ( static_cast< int >( covMatrixOffDiag.size() ) == COVMATRIX_OFFDIAG_VEC_COMPR_SIZE ) ) {
          // Set the off-diagonal elements using the raw variable.

          const auto& vecPairIndex = covMatrixComprIndexPairs;

          for(unsigned int k=0; k<COVMATRIX_OFFDIAG_VEC_COMPR_SIZE; ++k){
            std::pair<covMatrixIndex,covMatrixIndex> pairIndex = vecPairIndex[k];
            covMatrixIndex i = pairIndex.first;
            covMatrixIndex j = pairIndex.second;
            float offDiagCoeff = cov(i,i)>0 && cov(j,j)>0 ? covMatrixOffDiag[k]*sqrt(cov(i,i)*cov(j,j)) : 0;
            cov.fillSymmetric( i, j, offDiagCoeff );
          }

        }

        else valid = false;

      }


      // Return the filled matrix.
      return cov;

    }
  }
}

#endif
