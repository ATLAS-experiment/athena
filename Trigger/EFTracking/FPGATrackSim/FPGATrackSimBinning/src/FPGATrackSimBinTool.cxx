// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimGenScanTool.cxx
 * @author Elliot Lipeles
 * @date Feb 13, 2025
 * @brief See header file.
 */

#include "FPGATrackSimBinning/FPGATrackSimBinTool.h"

FPGATrackSimBinTool::FPGATrackSimBinTool(const std::string &algname, const std::string &name,
  const IInterface *ifc)
: AthAlgTool(algname, name, ifc) {
}


// ----------------------------------------------------------------------------------------
//  AthTool Methods
// ----------------------------------------------------------------------------------------

StatusCode FPGATrackSimBinTool::initialize() {
  // Dump the configuration to make sure it propagated through right
  const std::vector<Gaudi::Details::PropertyBase*> props = this->getProperties();
  for( Gaudi::Details::PropertyBase* prop : props ) {
    if (prop->ownerTypeName()==this->type()) {      
      ATH_MSG_DEBUG("Property:\t" << prop->name() << "\t : \t" << prop->toString());
    }
  }

  // Retrieve
  ATH_MSG_INFO("Using " << m_steps.size() << " steps");
  ATH_CHECK(m_steps.retrieve());

  if (m_steps.empty()) {
    ATH_MSG_FATAL("initialize() Step list empty");
    return StatusCode::FAILURE;
  }

  m_parMin = std::vector<double>(m_parMinConfig);
  m_parMax = std::vector<double>(m_parMaxConfig);

  FPGATrackSimBinStep* prev = 0;
  for (auto &step : m_steps) {
    ATH_MSG_INFO("Got Binning Step " << step->stepName());
    m_stepNames.push_back(step->stepName());
    if (!step->setRanges(prev, m_parMin, m_parMax)) {
      ATH_MSG_FATAL("Failed to setRange on step");
      return StatusCode::FAILURE;
    }
    prev = step.get();
  }

  ATH_MSG_DEBUG("Finished initializing BinTool");

  return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------------------------
//  Other methods
// ----------------------------------------------------------------------------------------

bool FPGATrackSimBinTool::FPGATrackSimBinTool::inRange(
    const ParSet &parset) const {
  for (unsigned par = 0; par < FPGATrackSimTrackPars::NPARS; par++) {
    if (!inRange(par,parset[par]))
      return false;
  }
  return true;
}

// ----------------------------------------------------------------------------------------
//  Valid Bin Function Implementation
// ----------------------------------------------------------------------------------------

void FPGATrackSimBinTool::setValidBin(const std::vector<unsigned>& idx) {
  lastStep()->setValidBin(idx);
}

void FPGATrackSimBinTool::initValidBins() {
  for (auto& step: m_steps) step->initValidBins();
}

void FPGATrackSimBinTool::printValidBin() const {
  for (auto &step : m_steps) {
    step->printValidBin();
  }
}

// Compute which bins correspond to track parameters that are in the region
// i.e. the pT, eta, phi, z0 and d0 bounds
void FPGATrackSimBinTool::computeValidBins(const IFPGATrackSimEventSelectionSvc* evtSel) {
  // determine which bins are valid
  
  FPGATrackSimTrackPars min_padded;
  FPGATrackSimTrackPars max_padded;
  FPGATrackSimTrackPars padding;
  padding[FPGATrackSimTrackPars::ID0] = m_d0FractionalPadding;
  padding[FPGATrackSimTrackPars::IZ0] = m_z0FractionalPadding;
  padding[FPGATrackSimTrackPars::IETA] = m_etaFractionalPadding;
  padding[FPGATrackSimTrackPars::IPHI] = m_phiFractionalPadding;
  padding[FPGATrackSimTrackPars::IHIP] = m_qOverPtFractionalPadding;
  for (unsigned par = 0; par < FPGATrackSimTrackPars::NPARS; par++)
  {
    min_padded[par] = evtSel->getMin()[par] - padding[par] * (evtSel->getMax()[par]-evtSel->getMin()[par]);
    max_padded[par] = evtSel->getMax()[par] + padding[par] * (evtSel->getMax()[par]-evtSel->getMin()[par]);
    if (par==FPGATrackSimTrackPars::IHIP) {
      // working in units of GeV internally
      min_padded[par] *= 1000;
      max_padded[par] *= 1000;
    }
    ATH_MSG_INFO("Padded Parameter Range: " << FPGATrackSimTrackPars::parName(par)
                                            << " min=" << min_padded[par] << " max=" << max_padded[par]);
  }

  // iterator over the finest binning used which is the laststep
  // setting lastSteps valid bins sets all the previous steps
  auto laststep = lastStep();
  for (FPGATrackSimBinArray<int>::Iterator bin : laststep->m_validBinFull) 
  {
    // this finds the parameters at all 2^5 corners of the bin and then finds the min and max of those
    std::vector<IdxSet> idxsets = FPGATrackSimBinUtil::makeVariationSet(std::vector<unsigned>({0,1,2,3,4}),bin.idx());
    FPGATrackSimTrackPars minvals = m_binDesc->parSetToTrackPars(laststep->binCenter(bin.idx()));
    FPGATrackSimTrackPars maxvals = m_binDesc->parSetToTrackPars(laststep->binCenter(bin.idx()));
    for (IdxSet & idxset : idxsets) {
      FPGATrackSimTrackPars trackpars = m_binDesc->parSetToTrackPars(laststep->binLowEdge(idxset));
      for (unsigned par =0; par < FPGATrackSimTrackPars::NPARS; par++) {
        minvals[par] = std::min(minvals[par],trackpars[par]);
        maxvals[par] = std::max(maxvals[par],trackpars[par]);
      }
    }

    // make sure bin overlaps with active region
    bool inRange = true;
    for (unsigned par =0; par < FPGATrackSimTrackPars::NPARS; par++) {
      inRange = inRange && (minvals[par] < max_padded[par]) && (maxvals[par] > min_padded[par]);
    }
    if (inRange)
    {
      setValidBin(bin.idx()); 
    }

    if (bin.data() == false)
    {
      ATH_MSG_VERBOSE("Invalid bin: " << bin.idx() << " :" << m_binDesc->parSetToTrackPars(laststep->binCenter(bin.idx()))
      << " minvals: " << minvals << " maxvals: " << maxvals );
    }
  }
}

ParSet FPGATrackSimBinTool::center() const
{
    ParSet parset;
    for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++)
    {
        parset[i] = parCenter(i);
    }
    return parset;
}