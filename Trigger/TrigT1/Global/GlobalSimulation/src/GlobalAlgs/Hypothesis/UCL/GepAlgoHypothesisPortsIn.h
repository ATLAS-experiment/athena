/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_GEPALGOHYPTHESISPORTSIN_H
#define GLOBALSIM_GEPALGOHYPTHESISPORTSIN_H

#include "AlgoConstants.h"
#include <bitset>
#include <ostream>
#include <memory>


#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim {

  struct GepAlgoHypothesisPortsIn {

    using BSPtr72 = std::shared_ptr<std::bitset<72>>;
    
    BSPtr72 m_I_GEPEmTobs {
      std::make_shared<std::bitset<72>>()};

    // std::shared_ptr<std::bitset<72>> m_I_GEPEmTobs {
    //   std::make_shared<std::bitset<72>>()};
    
    BSPtr72 m_I_GEPTauTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_GEPJetTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_eEmTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_eTauTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_jJetTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_jLJetTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_jTauTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_cTauTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_jEmTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_MuonTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_MetTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_jSumEtTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_EnergyTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_gLJetTobs{std::make_shared<std::bitset<72>>()};
    BSPtr72 m_I_gJetTobs{std::make_shared<std::bitset<72>>()};

    // _dv: 1 -> data is valid, data is invalid otherwise

    using BSPtr1 = std::shared_ptr<std::bitset<1>>;
    
    BSPtr1 m_I_GEPEmTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_GEPTauTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_GEPJetTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_eEmTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_eTauTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_jJetTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_jLJetTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_jTauTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_cTauTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_jEmTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_MuonTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_MetTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_jSumEtTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_EnergyTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_gLJetTobs_dv{std::make_shared<std::bitset<1>>()};
    BSPtr1 m_I_gJetTobs_dv{std::make_shared<std::bitset<1>>()};
  };


  // note - FIFO nature is not being nodelled - there are more suitable
  // containers for that. Here, GepAlgoHypothesisFIFO is simply
  // a collection of GepAlgoHypothesisPortsIn objects.
  using GepAlgoHypothesisFIFO = std::vector<GepAlgoHypothesisPortsIn>;

  std::string eEmInputTOBToString(const std::bitset<72>&);

}

std::ostream&
operator<< (std::ostream&,
	    const GlobalSim::GepAlgoHypothesisPortsIn&);


CLASS_DEF( GlobalSim::GepAlgoHypothesisFIFO , 31392099 , 1 )


#endif 
  
