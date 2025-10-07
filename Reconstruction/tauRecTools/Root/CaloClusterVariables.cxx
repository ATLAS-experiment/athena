/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/CaloClusterVariables.h"

#include "xAODCaloEvent/CaloVertexedTopoCluster.h"

#include <cmath>

const double CaloClusterVariables::DEFAULT = -1111.;

//****************************************
// constructor
//****************************************

CaloClusterVariables::CaloClusterVariables() :
  m_numConstit(static_cast<int>(DEFAULT)),
  m_effNumConstit_int(static_cast<int>(DEFAULT)),
  m_effNumConstit(DEFAULT),
  m_aveRadius(DEFAULT),
  m_aveEffRadius(DEFAULT),
  m_totMass(DEFAULT),
  m_effMass(DEFAULT),
  m_totEnergy(DEFAULT),
  m_effEnergy(DEFAULT) {
  }

//*******************************************
// update/fill the cluster based variables
//*******************************************

bool CaloClusterVariables::update(const xAOD::TauJet& pTau) {
    
  const auto& vertexedClusterList = pTau.vertexedClusters();

  std::vector<TLorentzVector> clusterP4Vector;
  clusterP4Vector.reserve(vertexedClusterList.size());

  for (const xAOD::CaloVertexedTopoCluster& vertexedCluster : vertexedClusterList) {
    clusterP4Vector.push_back(vertexedCluster.p4());
  }

  this->m_numConstit = std::ssize(clusterP4Vector);

  // Order constituents by energy
  sort(clusterP4Vector.begin(), clusterP4Vector.end(), CaloClusterCompare());

  //****************************************
  // Looping over all constituents
  //****************************************

  double sum_px = 0.;
  double sum_py = 0.;
  double sum_pz = 0.;
  double sum_e = 0.;
  double sum_of_E2 = 0.;
  double sum_radii = 0.;
  TLorentzVector centroid = calculateTauCentroid(this->m_numConstit, clusterP4Vector);

  for (const TLorentzVector& clusterP4 : clusterP4Vector) {

    sum_of_E2 += clusterP4.E()*clusterP4.E();
    sum_radii += clusterP4.DeltaR(centroid);
    sum_e += clusterP4.E();
    sum_px += clusterP4.Px();
    sum_py += clusterP4.Py();
    sum_pz += clusterP4.Pz();

  }

  // Sum up the energy for constituents
  this->m_totEnergy = sum_e;

  // Calculate the mass of the constituents
  if (this->m_numConstit < 2) this->m_totMass = DEFAULT;
  else {
    double mass2 = sum_e * sum_e - (sum_px * sum_px + sum_py * sum_py + sum_pz * sum_pz);
    this->m_totMass = mass2 > 0 ? std::sqrt(mass2) : -std::sqrt(-mass2);
  }

  // Calculate the average radius of the constituents wrt the tau centroid
  this->m_aveRadius = this->m_numConstit > 0 ? sum_radii / this->m_numConstit : DEFAULT;

  // Effective number of constituents
  this->m_effNumConstit = sum_of_E2 > 0 ? (sum_e * sum_e) / (sum_of_E2) : DEFAULT;

  this->m_effNumConstit_int = int(ceil(this->m_effNumConstit));

  // A problem!
  if (this->m_effNumConstit_int > this->m_numConstit) return false;

  // Avoid segfault, happens when we try to iterate below if sum_of_E2 was 0 or negative
  if (this->m_effNumConstit_int < 0) return false;

  //****************************************
  // Now: Looping over effective constituents
  //****************************************

  sum_px = 0.;
  sum_py = 0.;
  sum_pz = 0.;
  sum_e = 0.;
  sum_of_E2 = 0.;
  sum_radii = 0.;
  centroid = calculateTauCentroid(this->m_effNumConstit_int, clusterP4Vector);

  int icount = this->m_effNumConstit_int;
  for (const TLorentzVector& clusterP4 : clusterP4Vector) {
    if (icount <= 0) break;
    --icount;

    sum_radii += clusterP4.DeltaR(centroid);
    sum_e += clusterP4.E();
    sum_px += clusterP4.Px();
    sum_py += clusterP4.Py();
    sum_pz += clusterP4.Pz();

  }

  // Sum up the energy for effective constituents
  this->m_effEnergy = sum_e;

  // Calculate the mass of the constituents
  if (this->m_effNumConstit_int < 2) this->m_effMass = DEFAULT;
  else {
    double mass2 = sum_e * sum_e - (sum_px * sum_px + sum_py * sum_py + sum_pz * sum_pz);
    this->m_effMass = mass2 > 0 ? std::sqrt(mass2) : -std::sqrt(-mass2);
  }

  // Calculate the average radius of the constituents wrt the tau centroid
  this->m_aveEffRadius = this->m_effNumConstit_int > 0 ? sum_radii / this->m_effNumConstit_int : DEFAULT;

  return true;
}


//***********************************************************
// Calculate the geometrical center of the tau constituents
//***********************************************************
TLorentzVector CaloClusterVariables::calculateTauCentroid(int nConst, const std::vector<TLorentzVector>& clusterP4Vector) const {

  double px = 0.;
  double py = 0.;
  double pz = 0.;
  double modulus = 0.;

  for (const TLorentzVector& clusterP4: clusterP4Vector) {
    if (nConst <= 0) break;
    --nConst;

    modulus = std::sqrt((clusterP4.Px() * clusterP4.Px()) + (clusterP4.Py() * clusterP4.Py()) + (clusterP4.Pz() * clusterP4.Pz()));

    px += clusterP4.Px() / modulus;
    py += clusterP4.Py() / modulus;
    pz += clusterP4.Pz() / modulus;

  }

  TLorentzVector centroid(px, py, pz, 1);
  return centroid;
}
