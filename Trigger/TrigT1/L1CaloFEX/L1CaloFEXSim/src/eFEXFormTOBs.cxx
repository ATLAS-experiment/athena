/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                                  eFEXFormTOBs
//                              -------------------
//     begin                : 30 04 2021
//     email                : nicholas.andrew.luongo@cern.ch
//  ***************************************************************************/

#include "eFEXFormTOBs.h"

namespace LVL1 {

  // default constructor for persistency

eFEXFormTOBs::eFEXFormTOBs(const std::string& type, const std::string& name, const IInterface* parent):
  AthAlgTool(type, name, parent)
  {
    declareInterface<eFEXFormTOBs>(this);
  }

/** Desctructor */
eFEXFormTOBs::~eFEXFormTOBs()
{
}

StatusCode eFEXFormTOBs::initialize()
{
  return StatusCode::SUCCESS;
}

uint32_t eFEXFormTOBs::formTauTOBWord(int fpga, int eta, int phi, unsigned int et, unsigned int rhad, unsigned int rcore, unsigned int seed, unsigned int und, unsigned int ptMinTopo, unsigned int algoVersion) const
{
  uint32_t tobWord = 0;

  if(algoVersion>0) {
      und = 0; // no und bit in BDT TOBs
  }

  //rescale from eFEX scale (25 MeV) to TOB scale (100 MeV)
  // Do this using a bit shift to keep this 100% integer
  unsigned int etTob = (et>>m_tobETshift);

  // If ET < minimum value return empty TOB
  if (etTob < ptMinTopo) return tobWord;

  // Truncate at 12 bits, set to max value of 4095, 0xfff, or 111111111111
  if (etTob > 0xfff) etTob = 0xfff;

  // Create tob word with et, eta, phi, and fpga index, bitshifted to the appropriate locations
  tobWord = (fpga << m_fpgaShift) + (eta << m_etaShift) + (phi << m_phiShift) + (rhad << m_taurhadShift) + (rcore << m_taurcoreShift) + (seed << m_seedShift) + (und << m_undShift) + (0x1 << m_seedMaxShift) + etTob + (algoVersion << m_algoVersionShift);

  if(algoVersion==0) {
      ATH_MSG_DEBUG("Tau tobword: " << std::bitset<32>(tobWord));
  } else {
      ATH_MSG_DEBUG("Tau BDT tobword: " << std::bitset<32>(tobWord) );
  }

  return tobWord;
}

std::vector<uint32_t>  eFEXFormTOBs::formTauxTOBWords(int efexid, int fpga, int eta, int phi, unsigned int et, unsigned int rhad, unsigned int rcore, unsigned int seed, unsigned int und, unsigned int ptMinTopo, unsigned int algoVersion, unsigned int bdtScore) const
{
  std::vector<uint32_t> tobWords = {0, 0};

  // If ET < minimum return empty xTOB. Threshold is at TOB scale, so rescale ET before applying
  if ((et >> m_tobETshift) < ptMinTopo) return tobWords;

  if(algoVersion>0) {
      und = 0; // no und bit in BDT xTOBs
  }

  // Truncate ET at 16 bits, set to max value of 0xffff
  unsigned int etTob = (et < 0xffff ? et : 0xffff);

  // Calculate shelf and eFEX numbers from the efex index
  uint8_t shelf = int(efexid/12);
  uint8_t efex  = efexid%12;

  // Create tob word 0 with eta, phi, and fpga index, bitshifted to the appropriate locations
  tobWords[0] = (fpga << m_fpgaShift) + (eta << m_etaShift) + (phi << m_phiShift) + (rhad << m_taurhadShift) + (rcore << m_taurcoreShift) + (seed << m_seedShift) + (und << m_undShift) + (0x1 << m_seedMaxShift) + (algoVersion << m_algoVersionShift);

  // Create tob word 1 with et, efex and shelf indices, bitshifted to the appropriate locations
  tobWords[1] = (shelf << m_shelfShift) + (efex << m_efexShift) + etTob;

  if (algoVersion == 0) {
      ATH_MSG_DEBUG("Tau xtobwords: " << std::bitset<32>(tobWords[0]) << ", " << std::bitset<32>(tobWords[1]));
  } else {
      tobWords[0] += bdtScore; // put score in the LSBs of xtob word0
      ATH_MSG_DEBUG("Tau BDT xtobwords: " << std::bitset<32>(tobWords[0]) << ", " << std::bitset<32>(tobWords[1]));
  }

  return tobWords;
}


uint32_t eFEXFormTOBs::formEmTOBWord(int fpga, int eta, int phi, unsigned int rhad, unsigned int wstot, unsigned int reta, unsigned int seed, unsigned int und, unsigned int et, unsigned int ptMinTopo, unsigned int algoVersion) const
{
  uint32_t tobWord = 0;

  // rescale from eFEX scale (25 MeV) to TOB scale (100 MeV)
  // Do this using a bit shift to keep this 100% integer
  unsigned int etTob = (et>>m_tobETshift);

  // If ET < minimum value return empty TOB
  if (etTob < ptMinTopo) return tobWord;

  // Truncate at 12 bits, set to max value of 4095, 0xfff, or 111111111111
  if (etTob > 0xfff) etTob = 0xfff;

  // Create bare minimum tob word with et, eta, phi, and fpga index, bitshifted to the appropriate locations
  tobWord = (fpga << m_fpgaShift) + (eta << m_etaShift) + (phi << m_phiShift) + (rhad << m_rhadShift) + (wstot << m_wstotShift) + (reta << m_retaShift) + (seed << m_seedShift) + (und << m_undShift) + (0x1 << m_seedMaxShift) + etTob + (algoVersion << m_algoVersionShift);

  ATH_MSG_DEBUG("EM tobword: " << std::bitset<32>(tobWord) );

  return tobWord;
}


std::vector<uint32_t> eFEXFormTOBs::formEmxTOBWords(int efexid, int fpga, int eta, int phi, unsigned int rhad, unsigned int wstot, unsigned int reta, unsigned int seed, unsigned int und, unsigned int et, unsigned int ptMinTopo, unsigned int algoVersion) const
{
  std::vector<uint32_t> tobWords = {0, 0};

  // If ET < minimum return empty xTOB. Threshold is at TOB scale, so rescale ET before applying
  if ((et >> m_tobETshift) < ptMinTopo) return tobWords;

  // Truncate ET at 16 bits, set to max value of 0xffff
  unsigned int etTob = (et < 0xffff ? et : 0xffff);

  // Calculate shelf and eFEX numbers from the efex index
  uint8_t shelf = int(efexid/12);
  uint8_t efex  = efexid%12;

  // Create tob word 0 with eta, phi, and fpga index, bitshifted to the appropriate locations
  tobWords[0] = (fpga << m_fpgaShift) + (eta << m_etaShift) + (phi << m_phiShift) + (rhad << m_rhadShift) + (wstot << m_wstotShift) + (reta << m_retaShift) + (seed << m_seedShift) + (und << m_undShift) + (0x1 << m_seedMaxShift) + (algoVersion << m_algoVersionShift);

  // Create tob word 1 with et, efex and shelf indices, bitshifted to the appropriate locations
  tobWords[1] = (shelf << m_shelfShift) + (efex << m_efexShift) + etTob;

  ATH_MSG_DEBUG("EM xtobwords: " << std::bitset<32>(tobWords[0]) << ", " << std::bitset<32>(tobWords[1]));

  return tobWords;
}

} // end of namespace bracket
