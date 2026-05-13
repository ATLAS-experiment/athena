/*  -*- C++ -*- */

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PILEUPCOMPS_FLATBM
#define PILEUPCOMPS_FLATBM 1
/** @file FlatBM.h
 * @brief dummy (flat beam) implementation of IBeamIntensity service
 *
 * @author Paolo Calafiura - ATLAS Collaboration
 */
#include "PileUpTools/IBeamIntensity.h"
#include "AthenaBaseComps/AthService.h"

class FlatBM : public extends<AthService, IBeamIntensity>
{
public:
  /// \name Constructor
  //@{
  FlatBM(const std::string& name,ISvcLocator* svc);
  //@}
  /// \name IBeamIntensity methods
  //@{
  inline virtual float normFactor(int /*iXing*/) const override final { return 1.0; }
  inline virtual float largestElementInPattern() const override final { return 1.0; }
  inline virtual void selectT0(const EventContext& /*ctx*/) override {}
  inline virtual unsigned int getCurrentT0BunchCrossing() const override final
  {
    //The first filled bunch crossing is always BCID 1
    return 1;
  }
  inline virtual unsigned int getBeamPatternLength() const override final { return 1; }
  //@}
};
#endif
