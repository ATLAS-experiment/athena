/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GepAlgoHypothesisPortsIn.h"

using namespace GlobalSim;

std::ostream&
operator<< (std::ostream& os,
	    const GlobalSim::GepAlgoHypothesisPortsIn & ports_in) {
  os << "Entity_GepAlgoHypothesis port_in data:\n" <<
    "I_GEPEmTobs " << *ports_in.m_I_GEPEmTobs << '\n' <<
    "I_GEPTauTobs " << *ports_in.m_I_GEPTauTobs << '\n' <<
    "I_GEPJetTobs " << *ports_in.m_I_GEPJetTobs << '\n' <<
    "I_eEmTobs " << *ports_in.m_I_eEmTobs << '\n' <<
    "I_eTauTobs " << *ports_in.m_I_eTauTobs << '\n' <<
    "I_jJetTobs " << *ports_in.m_I_jJetTobs << '\n' <<
    "I_jLJetTobs " << *ports_in.m_I_jLJetTobs << '\n' <<
    "I_jTauTobs " << *ports_in.m_I_jTauTobs << '\n' <<
    "I_cTauTobs " << *ports_in.m_I_cTauTobs << '\n' <<
    "I_jEmTobs " << *ports_in.m_I_jEmTobs << '\n' <<
    "I_MuonTobs " << *ports_in.m_I_MuonTobs << '\n' <<
    "I_MetTobs " << *ports_in.m_I_MetTobs << '\n' <<
    "I_jSumEtTobs " << *ports_in.m_I_jSumEtTobs << '\n' <<
    "I_EnergyTobs " << *ports_in.m_I_EnergyTobs << '\n' <<
    "I_gLJetTobs " << *ports_in.m_I_gLJetTobs << '\n' <<
    "I_gJetTobs " << *ports_in.m_I_gJetTobs << '\n' <<

    "I_GEPEmTobs_dv " << *ports_in.m_I_GEPEmTobs_dv << '\n' <<
    "I_GEPTauTobs_dv " << *ports_in.m_I_GEPTauTobs_dv << '\n' <<
    "I_GEPJetTobs_dv " << *ports_in.m_I_GEPJetTobs_dv << '\n' <<
    "I_eEmTobs_dv " << *ports_in.m_I_eEmTobs_dv << '\n' <<
    "I_eTauTobs_dv " << *ports_in.m_I_eTauTobs_dv << '\n' <<
    "I_jJetTobs_dv " << *ports_in.m_I_jJetTobs_dv << '\n' <<
    "I_jLJetTobs_dv " << *ports_in.m_I_jLJetTobs_dv << '\n' <<
    "I_jTauTobs_dv " << *ports_in.m_I_jTauTobs_dv << '\n' <<
    "I_cTauTobs_dv " << *ports_in.m_I_cTauTobs_dv << '\n' <<
    "I_jEmTobs_dv " << *ports_in.m_I_jEmTobs_dv << '\n' <<
    "I_MuonTobs_dv " << *ports_in.m_I_MuonTobs_dv << '\n' <<
    "I_MetTobs_dv " << *ports_in.m_I_MetTobs_dv << '\n' <<
    "I_jSumEtTobs_dv " << *ports_in.m_I_jSumEtTobs_dv << '\n' <<
    "I_EnergyTobs_dv " << *ports_in.m_I_EnergyTobs_dv << '\n' <<
    "I_gLJetTobs_dv " << *ports_in.m_I_gLJetTobs_dv << '\n' <<
    "I_gJetTobs_dv " << *ports_in.m_I_gJetTobs_dv << '\n';

  return os;
}



