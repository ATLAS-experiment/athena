/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetTagTools/VertexSignificance.h"

#include <algorithm>
#include <cmath>

namespace Analysis {

  double get3DSignificance(const xAOD::Vertex& priVertex,
                           const std::vector<const xAOD::Vertex*>& secVertex,
                           const Amg::Vector3D& jetDirection) {

    std::vector<Amg::Vector3D> positions;
    std::vector<AmgSymMatrix(3)> weightMatrices;
    // If multiple secondary vertices were reconstructed, then a common (weighted) position will be used
    // in the signed decay length significance calculation
    Amg::Vector3D weightTimesPosition(0.,0.,0.);
    AmgSymMatrix(3) sumWeights;
    sumWeights.setZero();

    for (const auto& vtx : secVertex) {
      positions.push_back(vtx->position());
      weightMatrices.push_back(vtx->covariancePosition().inverse());
      weightTimesPosition += weightMatrices.back()*positions.back();
      sumWeights += weightMatrices.back();
    }

    // now we have the sum of the weights, let's invert this matrix to get the mean covariance matrix
    bool invertible;
    AmgSymMatrix(3) meanCovariance;
    meanCovariance.setZero();
    sumWeights.computeInverseWithCheck(meanCovariance, invertible);
    if (!invertible) return 0.;

    // calculate the weighted mean secondary vertex position
    Amg::Vector3D meanPosition = meanCovariance*weightTimesPosition;

    // add the mean covariance matrix of the secondary vertices to that of the primary vertex
    // this is the covariance matrix for the decay length
    AmgSymMatrix(3) covariance = meanCovariance + priVertex.covariancePosition();

    const Amg::Vector3D decayVector = meanPosition - priVertex.position();
    const double decaylength = decayVector.norm();
    if (decaylength == 0.) return 0.;  //Safety

    const Amg::Vector3D gradient = decayVector / decaylength;
    const double decaylength_err2 = gradient.dot(covariance * gradient);
    if (decaylength_err2 <= 0.) return 0.;  //Something is wrong

    double decaylength_significance = decaylength / std::sqrt(decaylength_err2);

    // get sign from projection on jet axis
    if (decayVector.dot(jetDirection) < 0.) decaylength_significance *= -1.;

    return decaylength_significance;
  }

  double get3DSignificanceCorr(const xAOD::Vertex& priVertex,
                               const std::vector<const xAOD::Vertex*>& secVertex,
                               const Amg::Vector3D& jetDirection) {

    std::vector<double> Sig3D(0);
    bool success=true;
    AmgSymMatrix(3) Wgt;

    for (const auto & svrt : secVertex)
      {
         Amg::Vector3D SVmPV = svrt->position()-priVertex.position();
         AmgSymMatrix(3) SVmPVCov = svrt->covariancePosition()+priVertex.covariancePosition();
         SVmPVCov.computeInverseWithCheck(Wgt, success);
         if( !success || Wgt(0,0)<=0. || Wgt(1,1)<=0. || Wgt(2,2)<=0. )continue;     //Inversion failure
         double significance = SVmPV.transpose()*Wgt*SVmPV;
         if(significance <= 0.) continue;                          //Something is still wrong!
         significance = std::sqrt(significance);
         if(SVmPV.dot(jetDirection)<0.) significance *= -1.;
         Sig3D.push_back(significance);
      }

    if(Sig3D.size()==0) return 0.;

    return *std::max_element(Sig3D.begin(),Sig3D.end());
  }

}
