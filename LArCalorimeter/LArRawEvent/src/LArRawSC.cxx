/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawEvent/LArRawSC.h"

// set method
void LArRawSC::setEnergies(const std::vector<int>&& energies)
{
  m_energies = std::move(energies);
}

void LArRawSC::setBCIds(const std::vector<unsigned short>&& bcids)
{
  m_BCId = std::move(bcids);
}

void LArRawSC::setTauEnergies( const std::vector < int >&& tauEnergies)
{
  m_tauEnergies = std::move(tauEnergies);
}

void LArRawSC::setPassTauSelection( const std::vector < bool >&& pass)
{
  m_passTauSelection = std::move(pass);
}

void LArRawSC::setHardwareId(const HWIdentifier hwid) {
  m_hardwareID = hwid;
}

void LArRawSC::setChannel(const unsigned chan) {
  m_chan = chan;
}

void LArRawSC::setSourceId(const unsigned sourceId) {
  m_sourceId = sourceId;
}

void LArRawSC::setSaturation( const std::vector < bool >&& satur) {
  m_satur = std::move(satur);

}
