/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IPixelToTPIDTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef TRK_IPIXELTOTPIDTOOL_H
#define TRK_IPIXELTOTPIDTOOL_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include <vector>

namespace Trk {
  class Track;

  /** @brief abstract interface for identification of particles based on

      @author Thijs Cornelissen <thijs.cornelissen -at- cern.ch>
   */

  class IPixelToTPIDTool : virtual public IAlgTool {
  public:
    DeclareInterfaceID(IPixelToTPIDTool, 1, 0);

   /** @brief particle identification function returning a probability.
       @param[in] track the track to be identified
       @returns   probability
     */
    virtual float dEdx(const EventContext& ctx,
                       const Trk::Track& track,
                       int& nUsedHits,
                       int& nUsedIBLOverflowHits) const = 0;

    virtual std::vector<float> getLikelihoods(const EventContext& ctx,
                                              double dedx,
                                              double p,
                                              int nGoodPixels) const = 0;

    virtual float getMass(const EventContext& ctx,
                          double dedx,
                          double p,
                          int nGoodPixels) const = 0;

  };

} // end of namespace

#endif 
