
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimGenScanBinDesc.cxx
 * @author Elliot Lipeles
 * @date Feb 13, 2025
 * @brief See header file.
 */

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "FPGATrackSimBinning/IFPGATrackSimBinDesc.h"
#include <GaudiKernel/StatusCode.h>
#include "FPGATrackSimBinning/FPGATrackSimBinStep.h"

StatusCode FPGATrackSimBinStep::initialize()
{
  // Dump the configuration to make sure it propagated through right
  const std::vector<Gaudi::Details::PropertyBase*> props = this->getProperties();
  for( Gaudi::Details::PropertyBase* prop : props ) {
    if (prop->ownerTypeName()==this->type()) {      
      ATH_MSG_DEBUG("Property:\t" << prop->name() << "\t : \t" << prop->toString());
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimBinStep::setRanges(FPGATrackSimBinStep *prev,
                                          const ParSet &parMin,
                                          const ParSet &parMax) {

  m_prev = prev;
  if (prev) {
    m_stepNum = prev->m_stepNum+1;
  } else {
    m_stepNum = 0;    
  }
  m_parMin = parMin;
  m_parMax = parMax;
  m_parBins = std::vector<unsigned>(m_parBinsConfig);
  for (unsigned par = 0; par < FPGATrackSimTrackPars::NPARS; par++) {
    if (m_parBins[par] <= 0) {
      ATH_MSG_FATAL("Every dimension must be at least one bin");
      return StatusCode::FAILURE;
    }
    m_parStep[par] = (m_parMax[par] - m_parMin[par]) / m_parBins[par];
    if (m_parBins[par] <= 0)
    {
      ATH_MSG_FATAL("Every dimension must be at least one bin (set #bins=1 for not binning in that parameter)");
    }
    if (prev) {
      if (m_parBins[par] < prev->m_parBins[par]) {
        ATH_MSG_FATAL("Number of bins can only increase with each step");
        return StatusCode::FAILURE;
      }
      if (m_parBins[par] % prev->m_parBins[par] !=0) {
        ATH_MSG_FATAL("Number of bins must be integer multiple of bins in previous step");
        return StatusCode::FAILURE;
      }
      if (m_parBins[par] != prev->m_parBins[par]) {
        // This step involves this parameter
        m_pars.push_back(par);
      }
    } else {
      if (m_parBins[par] != 1) {
        // This step involves this parameter
        m_pars.push_back(par);
      }
    }
  }

  ATH_MSG_DEBUG("Parameters = " << m_pars);

  return StatusCode::SUCCESS;
}

ParSet FPGATrackSimBinStep::binLowEdge(const IdxSet &idx) const
{
    ParSet parset;
    for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++)
    {
        parset[i] = binLowEdge(i, idx[i]);
    }
    return parset;
}

ParSet FPGATrackSimBinStep::binCenter(const IdxSet &idx) const
{
    ParSet parset;
    for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++)
    {
        parset[i] = binCenter(i, idx[i]);
    }
    return parset;
}

IdxSet FPGATrackSimBinStep::binIdx(const ParSet &pars) const
{
    IdxSet retv;
    for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++)
    {
        retv[i] = binIdx(i, pars[i]);
    }
    return retv;
}



// Convert to previous steps idx
IdxSet FPGATrackSimBinStep::convertToPrev(const IdxSet &cur) const {
  IdxSet retv{};
  if (m_prev) {
    for (unsigned par =0; par < FPGATrackSimTrackPars::NPARS; par++) {
      retv[par] = int(cur[par]*((const FPGATrackSimBinStep*)m_prev)->m_parBins[par]/m_parBins[par]);
    }
  } else {
    ATH_MSG_FATAL("convertToPrev called, but no previous");
  }
  return retv;
}


const std::vector<unsigned> FPGATrackSimBinStep::stepIdx(IdxSet idx) const {
       return FPGATrackSimBinUtil::subVec(m_pars, idx);}
  
const std::vector<unsigned> FPGATrackSimBinStep::stepBins() const {
    return FPGATrackSimBinUtil::subVec(m_pars, m_parBins);}

void FPGATrackSimBinStep::setValidBin(const std::vector<unsigned>& idx) {
  m_validBinFull[idx] = true;
  m_validBinLocal[stepIdx(idx)] = true;
  if (m_prev) m_prev->setValidBin(convertToPrev(idx));
}

void FPGATrackSimBinStep::initValidBins() {
  m_validBinFull.setsize(m_parBins, false);
  m_validBinLocal.setsize(stepIdx(m_parBins), false);
}

void FPGATrackSimBinStep::printValidBin() const {
  // count valid bins
  int validBinsFull = 0;
  
  for (FPGATrackSimBinArray<int>::ConstIterator bin : m_validBinFull) {
    if (bin.data())
      validBinsFull++;
  }
  ATH_MSG_INFO("Step" << name() << "Valid Bins Full: " << validBinsFull);

  // count valid bins local
  int validBinsLocal = 0;
  for (FPGATrackSimBinArray<int>::ConstIterator bin : m_validBinLocal) {
  if (bin.data())
    validBinsLocal++;
  }
  ATH_MSG_INFO("Step" << name() <<  "Valid Bins Local: " << validBinsLocal);
    
}

