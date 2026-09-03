/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_CONEJETMAKER_H
#define TRIGGEPPERF_CONEJETMAKER_H


#include "xAODTrigger/jFexSRJetRoIContainer.h"

#include "./IJetMaker.h"
#include "./Jet.h"
#include "./Cluster.h"

#include <string>
#include <vector>
#include <ostream>


namespace Gep
{
  class ConeJetMaker : public Gep::IJetMaker
  {
  public:
   
    ConeJetMaker(float jetR,
		 const xAOD::jFexSRJetRoIContainer& seeds,
		 float seedEtThreshold = 5.e3 /*MeV*/,
		 const std::string& recombScheme = "EScheme");
    
    std::string toString() const override;
    virtual std::vector<Gep::Jet> makeJets(const std::vector<Gep::Cluster> &clusters) const override;

    float getJetR() const { return m_jetR; }
    float getSeedEtThreshold() const  { return m_seedEtThreshold; }
    std::string recombSchemeAsString() const;

    
  private:
    float m_jetR;
    const xAOD::jFexSRJetRoIContainer& m_seeds;
    float m_seedEtThreshold;

    enum class RecombScheme {EScheme, SeedScheme};

    std::vector<std::pair<std::string, RecombScheme>> m_knownSchemes {
    {"EScheme", RecombScheme::EScheme},
      {"SeedScheme", RecombScheme::SeedScheme}
  };
    
    RecombScheme m_recombScheme {RecombScheme::EScheme};

    RecombScheme string2RecombScheme(const std::string&) const;

  };


}

#endif //TRIGGEPPERF_CONEJETMAKER_H
