/*
 *   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include "./Jet.h"
#include "./ConeJetMaker.h"

#include <sstream>

#include <cassert>
#include <algorithm>

Gep::ConeJetMaker::ConeJetMaker(float jetR,
				const xAOD::jFexSRJetRoIContainer& seeds,
				float seedEtThreshold,
				const std::string& scheme) :
  m_jetR{jetR},
  m_seeds{seeds},
  m_seedEtThreshold{seedEtThreshold},
  m_recombScheme{string2RecombScheme(scheme)}{
}


Gep::ConeJetMaker::RecombScheme
Gep::ConeJetMaker::string2RecombScheme(const std::string& scheme) const {
  auto schemeMatches =
    [&scheme](const std::pair<std::string, RecombScheme>& p){return (p.first) == scheme;};
  
  auto scheme_it = std::find_if(m_knownSchemes.cbegin(),
				m_knownSchemes.cend(),
				schemeMatches);
  
  if (scheme_it == m_knownSchemes.end()){
    throw std::runtime_error("GEP ConeJetMaker unknown recombinaton scheme " +
			     scheme);
  }
  
  return scheme_it->second;
}

std::string
Gep::ConeJetMaker::recombSchemeAsString() const {
  auto schemeMatches =
    [&scheme=m_recombScheme](const std::pair<std::string, RecombScheme>& p){
      return (p.second) == scheme;};
  
  auto scheme_it = std::find_if(m_knownSchemes.cbegin(),
				m_knownSchemes.cend(),
				schemeMatches);
  
  if (scheme_it == m_knownSchemes.end()){
    throw std::runtime_error("GEP ConeJetMaker unknown recombinaton scheme " +
			     recombSchemeAsString());
  }
  
  return scheme_it->first;
}

double deltaR (double eta_1, double eta_2, double phi_1, double phi_2);

std::vector<Gep::Jet>
Gep::ConeJetMaker::makeJets( const std::vector<Gep::Cluster> &clusters) const
{
  std::vector<Gep::Jet> jets;
  for (const auto seed: m_seeds) {

    float seedEt = seed->et();

    // skip seeds with Et below threshold
    if(seedEt < m_seedEtThreshold) continue; 
    float seedEta = seed->eta();
    float seedPhi = seed->phi();

    Gep::Jet jet;

    jet.radius = m_jetR;
    jet.seedEta = seedEta;
    jet.seedPhi = seedPhi;
    jet.seedEt = seedEt;

    TLorentzVector jetVec;
    float px{0}, py{0};
    int clusterIndex {0};
    
    //build jet with clusters within dR from seed
    for (const auto &cl: clusters) {
      float dR_seed_cl = deltaR(seedEta, cl.vec.Eta(), seedPhi, cl.vec.Phi());

      if (dR_seed_cl < m_jetR) {
        jetVec += cl.vec;
        px += cl.vec.Px();
        py += cl.vec.Py();
	
        jet.constituentsIndices.push_back(clusterIndex);
      }
      clusterIndex++;
    }


    // skip cone jets with 0 constituents
    if (jet.constituentsIndices.empty()) {continue;}
    

    // recombination scheme
    if (m_recombScheme == RecombScheme::EScheme) {
      // default option: add four-vectors of constituents
      jet.vec = jetVec;
    } else if (m_recombScheme == RecombScheme::SeedScheme) {
      // massless jet, correct pt, re-using seed (eta,phi)
      float m = 0;
      float pt = std::sqrt(px*px + py*py);
      jet.vec.SetPtEtaPhiM(pt, seedEta, seedPhi, m);
    } else {
      throw std::runtime_error("GEP ConeJetMaker recombinaton scheme error");
    }
    jets.emplace_back(std::move(jet));
  }

  return jets;
}


std::string Gep::ConeJetMaker::toString() const {
  std::stringstream ss;
  ss << "ConeJetMaker. seed thresholds: " << m_seedEtThreshold
     << " rad: " << m_jetR << " recombinaton scheme: " << recombSchemeAsString();
  return ss.str();
}


double deltaR (double eta1, double eta2, double phi1, double phi2) {
  double deltaPhi = TVector2::Phi_mpi_pi(phi1 - phi2);
  double deltaEta = eta1 - eta2;
  return std::sqrt( deltaEta*deltaEta + deltaPhi*deltaPhi );
}


