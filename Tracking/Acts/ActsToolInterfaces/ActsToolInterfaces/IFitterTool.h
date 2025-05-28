/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IFITTERTOOL_H
#define ACTSTOOLINTERFACES_IFITTERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include "ActsGeometry/ATLASSourceLink.h"

#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/Seed.h"

#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "TrkTrack/Track.h"
namespace ActsTrk {
  /** @brief Generic interface class to fit xAOD::Uncalibrated measurements to (multi)-trajectories. Depending on the fitter 
   *         in use, the methods either construct a single track trajectory or a bunch of trajectories sharing subsets of 
   *         measurements with each other. By convention, the creation of the returned trajectory container is not guaranteed
   *         and usually the fitters return nullptrs in case of fit failures. */
  class IFitterTool : virtual public IAlgTool {
  public:
    DeclareInterfaceID(IFitterTool, 1, 0);
    /** @brief Tries to fit a short-line track from an initial ITk/ID seed made up out of three spacepoints.
     *         For a better fit convergence, this method also takes an external estimate of the track parameters    
     *  @param seed: Reference to the riplet of ITk/ID space points to fit.
     *  @param initialParams: Rough parameter estimate used to start the fit 
     *  @param tgContext: Geometry context to fetch the aligned positions of each surface
     *  @param mfContext: Reference to the magnetic field context having the ATLAS magnetic field wrapped
     *  @param calContext: Reference to the CalibrationContext which is wrapping the pointer to the Gaudi-event context    
     *  @return: In case of fit-failure, a nullptr otherwise a unique_ptr to the fitted trajectories from the fit */
    virtual std::unique_ptr<MutableTrackContainer> fit(const Seed &seed,
                                                       const Acts::BoundTrackParameters& initialParams,
                                                       const Acts::GeometryContext& tgContext,
                                                       const Acts::MagneticFieldContext& mfContext,
                                                       const Acts::CalibrationContext& calContext) const = 0;
                                               
    /** @brief Attempt to fit a trajectory from a list of uncalibrated measurements. The measurements need
     *         to be sorted such that the associated surfaces are passed in consecutive order by the 
     *         constructed trajectory. For a betterfit convergence, the method takes an external estimate
     *         of the track parameters.    
     *  @param measList: List of pointers to the uncalibrated measurements to consider for fitting
     *  @param initialParams: Rough parameter estimate used to start the fit 
     *  @param tgContext: Geometry context to fetch the aligned positions of each surface
     *  @param mfContext: Reference to the magnetic field context having the ATLAS magnetic field wrapped
     *  @param calContext: Reference to the CalibrationContext which is wrapping the pointer to the Gaudi-event context     
     *  @return: In case of fit-failure, a nullptr otherwise a unique_ptr to the fitted trajectories from the fit */
    virtual std::unique_ptr< MutableTrackContainer > fit(const std::vector<const xAOD::UncalibratedMeasurement* > & measList,
                                                         const Acts::BoundTrackParameters& initialParams,
                                                         const Acts::GeometryContext& tgContext,
                                                         const Acts::MagneticFieldContext& mfContext,
                                                         const Acts::CalibrationContext& calContext,
                                                         const Acts::Surface* targetSurface = nullptr) const = 0;

    virtual StatusCode fit(const EventContext& ctx,
                           const TrackContainer::ConstTrackProxy& track,          
                           MutableTrackContainer& trackContainer) const = 0;
    
  };
  

}

#endif
