/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXtauTOB.cxx  -  
//                              -------------------
//     begin                : 17 01 2020
//     email                : nicholas.andrew.luongo@cern.ch
//  **************************************************************************


#include "L1CaloFEXSim/eFEXtauTOB.h"


void LVL1::eFEXtauTOB::setEta(unsigned int eta)
{
	m_eta = eta;
}

void LVL1::eFEXtauTOB::setPhi(unsigned int phi)
{
	m_phi = phi;
}

void LVL1::eFEXtauTOB::setEt(unsigned int et)
{
	m_et = et;
}

void LVL1::eFEXtauTOB::setBitwiseEt(unsigned int bitwise_et)
{
	m_bitwise_et = bitwise_et;
}

void LVL1::eFEXtauTOB::setIso(unsigned int iso)
{
	m_iso = iso;
}

void LVL1::eFEXtauTOB::setFPGAID(unsigned int fpgaid)
{
	m_fpga_id = fpgaid; 
}

void LVL1::eFEXtauTOB::seteFEXID(unsigned int efexid)
{
	m_efex_id = efexid; 
}

void LVL1::eFEXtauTOB::setSeedUnD(bool seedund)
{
	m_seed_und = seedund;
}

void LVL1::eFEXtauTOB::setSeed(unsigned int seed)
{
	m_seed = seed;
}

void LVL1::eFEXtauTOB::setRcoreCore(unsigned int rcorecore)
{
	m_rcore_core = rcorecore;
}

void LVL1::eFEXtauTOB::setRcoreEnv(unsigned int rcoreenv)
{
	m_rcore_env = rcoreenv;
}

void LVL1::eFEXtauTOB::setRhadCore(unsigned int rhadcore)
{
	m_rhad_core = rhadcore;
}

void LVL1::eFEXtauTOB::setRhadEnv(unsigned int rhadenv)
{
	m_rhad_env = rhadenv;
}

void LVL1::eFEXtauTOB::setBDTScore(unsigned int bdtscore)
{
	m_bdt_score = bdtscore;
}

void LVL1::eFEXtauTOB::setIsBDTAlgo(unsigned int is_bdt_algo)
{
	m_is_bdt_algo = is_bdt_algo;
}

void LVL1::eFEXtauTOB::setTobword(uint32_t tobword) {
  m_tobword = tobword;
}

void LVL1::eFEXtauTOB::setxTobword0(uint32_t tobword) {
  m_xtobword0 = tobword;
}

void LVL1::eFEXtauTOB::setxTobword1(uint32_t tobword) {
  m_xtobword1 = tobword;
}

