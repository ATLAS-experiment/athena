// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef IFPGATrackSimBinDesc_H
#define IFPGATrackSimBinDesc_H

/**
 * @file IFPGATrackSimBinDesc.h
 * @author Elliot Lipeles
 * @date Sept 10th, 2024
 * @brief Defines Parameters used for binning
 *
 * Declarations in this file (there are a series of small classes):
 *      class FPGATrackSimBinDesc
 *      
 * 
 * Overview of stucture:
 *    -- 
 * 

 * 
 * References:
 *
 */
#include "GaudiKernel/IAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"

#include "FPGATrackSimBinning/FPGATrackSimBinUtil.h"

#include <string>

class FPGATrackSimBinStep;

//-------------------------------------------------------------------------------------------------------
// BinnedHits
//       Nomenclature:
//          parameter = parameter of track e.g. (pT,eta,phi,d0,z0) but could be
//          in different coordinates
//             such at in the keylayer format where its
//             (z_in,z_out,phi_in,phi_out,x_m)
//          bin = a bin in the full parameters space (upto 5-d)
//          subbin = a binning used as part of the total binning e.g. only in
//          (pT,phi) or only in (z_in,z_out)
//-------------------------------------------------------------------------------------------------------
class IFPGATrackSimBinDesc : virtual public IAlgTool {
public:
  DeclareInterfaceID(IFPGATrackSimBinDesc, 1, 0);

  //--------------------------------------------------------------------------------------------------
  //
  // Virtual methods that are overloaded to define the binning
  //
  //--------------------------------------------------------------------------------------------------

  // Specification of parameters
  virtual const std::string &parNames(unsigned i) const = 0;

  // convert back and forth from pT, eta, phi, d0, z0 and internal paramater set
  virtual const FPGATrackSimBinUtil::ParSet trackParsToParSet(const FPGATrackSimTrackPars &pars) const = 0;
  virtual const FPGATrackSimTrackPars parSetToTrackPars(const FPGATrackSimBinUtil::ParSet &parset) const = 0;

  // calculate the distance in phi or eta from a track defined by parset to a hit
  // these can be implemented as any variable in the r-phi or r-eta plane (not necessarily eta and phi).
  virtual double phiResidual(const FPGATrackSimBinUtil::ParSet &parset, FPGATrackSimHit const *hit) const = 0;
  virtual double etaResidual(const FPGATrackSimBinUtil::ParSet &parset,  FPGATrackSimHit const *hit) const = 0;

  // idx should be with the definition specifed in the step
  // NOTE: the stored hit may be modified!
  virtual bool hitInBin(const FPGATrackSimBinStep &step,
                        const FPGATrackSimBinUtil::IdxSet &idx,
                        FPGATrackSimBinUtil::StoredHit &storedhit) const = 0;

  // Write the relevant LUT tables for firmware
  // Implementation is optional. This is not needed for operation,
  // but to generate constants for the firmware
  virtual void writeLUTs([[maybe_unused]] const FPGATrackSimBinStep &step) const {}

  // Set truthbin for debugging
  // This does not need to be set but can be useful for debuging
  void setTruthBin(const std::vector<FPGATrackSimBinUtil::IdxSet>& truthbin){ m_truthbin=truthbin;}

protected:
  std::vector<FPGATrackSimBinUtil::IdxSet> m_truthbin{};

};

#endif // IFPGATrackSimBinDesc_H
