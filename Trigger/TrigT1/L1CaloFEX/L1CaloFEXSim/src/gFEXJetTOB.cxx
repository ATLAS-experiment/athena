/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXJetTOB - Forms the Jet TOBs for gFEX
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#include "L1CaloFEXSim/gFEXJetTOB.h"


void LVL1::gFEXJetTOB::setEta(unsigned int eta) {
  m_eta = eta;
}

void LVL1::gFEXJetTOB::setPhi(unsigned int phi) {
  m_phi = phi;
}

void LVL1::gFEXJetTOB::setET(unsigned int et) {
  m_ET = et;
}

void LVL1::gFEXJetTOB::setStatus(unsigned int st) {
  m_st = st;
}

void LVL1::gFEXJetTOB::setTobID(unsigned int tobID) {
  m_tobID = tobID;
}

void LVL1::gFEXJetTOB::setWord(uint32_t word) {
  m_word = word;
}
