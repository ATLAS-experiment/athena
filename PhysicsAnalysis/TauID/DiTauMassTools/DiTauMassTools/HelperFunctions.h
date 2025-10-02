/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Asg wrapper around the MissingMassCalculator
// author Michael Huebner <michael.huebner@no.spam.cern.ch>
#ifndef DITAUMASSTOOLS_HELPERFUNCTIONS_H
#define DITAUMASSTOOLS_HELPERFUNCTIONS_H

// Framework includes

// EDM include(s)
#include "xAODTau/TauJet.h"

// local include(s)
#include "TH1F.h"
#include "TF1.h"
#include "TKey.h"
#include "TDirectory.h"
#include "TROOT.h"
#include "Math/VectorUtil.h"
#include "Math/Vector2D.h"

namespace DiTauMassTools{
  using ROOT::Math::XYVector;
  using ROOT::Math::VectorUtil::Phi_mpi_pi;

namespace MaxHistStrategy
{
  enum e { MAXBIN=0,MAXBINWINDOW, SLIDINGWINDOW, FIT,MAXMAXHISTSTRATEGY };
}

namespace HistInfo
{
  enum e { PROB=0, INTEGRAL, CHI2, DISCRI, TANTHETA, TANTHETAW, FITLENGTH, RMS, RMSVSDISCRI, MEANBIN, MAXHISTINFO };
}

namespace MMCCalibrationSet
{
  enum e { MMC2015HIGHMASS=0, UPGRADE, LFVMMC2012, MMC2019, MMC2024, MAXMMCCALIBRATIONSET };
  const std::string name[MAXMMCCALIBRATIONSET]={ "MMC2015HIGHMASS", "UPGRADE", "LFVMMC2012", "MMC2019", "MMC2024"};
}
    
namespace MMCFitMethod
{
  enum e { MAXW=0, MLM, MLNU3P,MAX};
  const std::string name[MAX]={ "MAXW=MaximumWeight", "MLM=MostLikelyMass", "MLNU3P=MostLikelyNeUtrino3Momentum"};
  const std::string shortName[MAX]={ "MAXW", "MLM", "MLNU3P"};
}

namespace TauTypes
{
  enum e {ll=0, lh, hh};
}

// define ignore template to suppress warnings in MissingMassProb
// see source file of MissingMassProb for further reasoning
template <typename T> void ignore(T &&){}

template <typename VectorType1, typename VectorType2>
double Angle(const VectorType1& vec1, const VectorType2& vec2) {
    // Calculate the dot product and magnitudes (similar to but faster than ::Angle())
    double dotProduct = vec1.Px() * vec2.Px() + vec1.Py() * vec2.Py() + vec1.Pz() * vec2.Pz();
    double magnitude1 = vec1.P();
    double magnitude2 = vec2.P();

    // Calculate and return the angle
    return acos(dotProduct / (magnitude1 * magnitude2));
}

template <typename VectorType>
double mT(const VectorType & vec,const XYVector & met_vec) {
  double mt=0.0;
  double dphi=std::abs(Phi_mpi_pi(vec.Phi()-met_vec.Phi()));
  double cphi=1.0-cos(dphi);
  if(cphi>0.0) mt=sqrt(2.0*vec.Pt()*met_vec.R()*cphi);
  return mt;
}

//________________________________________________________________________

bool updateDouble  (const double in, double & out) ;
void fastSinCos (const double & phi, double & sinPhi, double & cosPhi);
double fixPhiRange (const double & phi);
double MaxDelPhi(int tau_type, double Pvis, double dRmax_tau);
int getLFVMode( const xAOD::IParticle* p1, const xAOD::IParticle* p2, int mmcType1, int mmcType2);
int mmcType(const xAOD::IParticle* part); // returns particle type as required by MMC
void readInParams(TDirectory* dir, MMCCalibrationSet::e aset, std::vector<TF1*>& lep_numass, std::vector<TF1*>& lep_angle, std::vector<TF1*>& lep_ratio, std::vector<TF1*>& had_angle, std::vector<TF1*>& had_ratio);
} // namespace DiTauMassTools

#endif // not DITAUMASSTOOLS_HELPERFUNCTIONS_H
