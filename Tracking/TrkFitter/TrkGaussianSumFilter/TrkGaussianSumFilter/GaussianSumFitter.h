/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   GaussianSumFitter.h
 * @date   Monday 7th March 2005
 * @author Tom Athkinson, Anthony Morley, Christos Anastopoulos
 * @brief  Class for fitting according to the Gaussian Sum Filter
 * Formalism Implements Algorithm C described in
 * "Track fitting with non-Gaussian noise" R.Fruhwirth.
 */
#ifndef TrkGaussianSumFitter_H
#define TrkGaussianSumFitter_H

#include "TrkGaussianSumFilter/IMultiStateExtrapolator.h"
#include "TrkGaussianSumFilter/GSFTsos.h"
//
#include "TrkGaussianSumFilterUtils/GsfMeasurementUpdator.h"
#include "TrkGaussianSumFilterUtils/MultiComponentStateCombiner.h"
//
#include "TrkTrack/MultiComponentStateOnSurface.h"
//
#include "TrkCaloCluster_OnTrack/CaloCluster_OnTrack.h"
#include "TrkDetElementBase/TrkDetElementBase.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkEventUtils/TrkParametersComparisonFunction.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkFitterUtils/FitterTypes.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/Surface.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
//
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ToolHandle.h"
//
#include <atomic>

namespace Trk {
class FitQuality;
class Track;

class GaussianSumFitter final
  : virtual public ITrackFitter
  , public AthAlgTool
{
public:
  /** Constructor with parameters to be passed to AlgTool */
  GaussianSumFitter(const std::string&, const std::string&, const IInterface*);

  /** Virtual destructor */
  virtual ~GaussianSumFitter() = default;

  /** AlgTool initialise method */
  virtual StatusCode initialize() override final;

  /// @{
  /// @name Inteface method. The main one are the ones taking
  /// Measurement or Raw Data inputs. The rest are implemented on top
  /// of these two.
  /// We follow the ITrackFitter interface but not all arguements
  /// are meaningfull
  ///
  /// We do not implement outlier remove.
  /// Actually by default we try to
  /// re-integrate possible outliers from non-GSF fits.
  ///
  /// The material effects actually applied are determined
  /// by the dedicated configured GsfExtrapolator.

  /** @brief Fit a collection of 'PrepRawData' objects using the Gaussian Sum Filter
      - This requires that an trackParameters object be supplied also as an
     initial guess */
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const PrepRawDataSet&,
    const TrackParameters&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis /*Not used*/) const override final;

  /** @brief Fit a collection of 'RIO_OnTrack' objects using the Gaussian Sum Filter
      - This requires that an trackParameters object be supplied also as an
     initial guess */
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const MeasurementSet&,
    const TrackParameters&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis particleHypothesis /*Not used*/) const override final;

  /** @brief Refit a track*/
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const Track&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis /*Not used*/) const override final;

  /** @brief Refit a track adding a PrepRawDataSet*/
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const Track&,
    const PrepRawDataSet&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis /*Not used*/) const override final;

  /** @brief Refit a track adding a measurement base set*/
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const Track&,
    const MeasurementSet&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis /*Not used*/) const override final;

  /** @brief Combine two tracks by refitting their measurements*/
  virtual std::unique_ptr<Track> fit(
    const EventContext& ctx,
    const Track&,
    const Track&,
    const RunOutlierRemoval /*Not used*/,
    const ParticleHypothesis /*Not used*/) const override final;
  /// @}
 // Internally we can use a simple std::vector
 using GSFTrajectory = std::vector<GSFTsos>;

private:
 /// @{
 /// @name Helper methods

 /** @brief Helper to convert the GSFTrajectory to a Trk::Track */
 std::unique_ptr<MultiComponentStateOnSurfaceDV> convertTrajToTrack(
     GSFTrajectory& trajectory) const;

 /** @brief Produces a perigee from a smoothed trajectory */
 GSFTsos makePerigee(
     const EventContext& ctx,
     Trk::IMultiStateExtrapolator::Cache&,
     const GSFTrajectory& smoothedTrajectory) const;
/** Methof to add the CaloCluster onto the track */
 bool  addCCOT(const EventContext& ctx,
               const Trk::CaloCluster_OnTrack* ccot,
               GSFTrajectory& smoothedTrajectory) const;
 /// @}

 /// @{
 /// @name Actual implementation of the GSF formalism

 /** @brief Forward GSF fit */
 template <typename T>
 GSFTrajectory forwardFit(
     const EventContext& ctx,
     IMultiStateExtrapolator::Cache& cache,
     const T& inputSet,
     const TrackParameters& estimatedTrackParametersNearOrigin) const;

 /** @brief Progress one step along the forward fit */
 template <typename T>
 bool stepForwardFit(
     const EventContext& ctx,
     IMultiStateExtrapolator::Cache&,
     GSFTrajectory& forwardTrajectory,
     const T* measurement,
     const Surface& surface,
     MultiComponentState& updatedState) const;

 /** @brief Gsf smoothed trajectory. This method can handle additional info like
  * calorimeter cluster constraints. It also produces what we actually store in
  * Trk::Tracks.*/
 GSFTrajectory smootherFit(
     const EventContext& ctx, Trk::IMultiStateExtrapolator::Cache&,
     GSFTrajectory& forwardTrajectory,
     const CaloCluster_OnTrack* ccot = nullptr) const;

 /// @}
 private:
 ToolHandle<IMultiStateExtrapolator> m_extrapolator{
     this, "ToolForExtrapolation", "Trk::GsfExtrapolator/GsfExtrapolator", ""};

 ToolHandle<IRIO_OnTrackCreator> m_rioOnTrackCreator{
     this, "ToolForROTCreation", "",
     "Tool for converting Raw Data to measurements"};

 Gaudi::Property<bool> m_reintegrateOutliers{this, "ReintegrateOutliers", true,
                                             "Reintegrate Outliers"};

 Gaudi::Property<bool> m_refitOnMeasurementBase{
     this, "RefitOnMeasurementBase", true, "Refit On Measurement Base"};

 Gaudi::Property<bool> m_combineWithFitter{
     this, "CombineStateWithFitter", false,
     "Combine with forwards state during Smoothing"};

 Gaudi::Property<bool> m_useMode{
     this, "useMode", true,
     "Collapse MultiComponent States using Mode rather than Mean"};

 Gaudi::Property<bool> m_slimTransientMTSOS{
     this, "slimTransientMTSOS", true,
     "Slim the transient MTSOS . Keeping just the combined state and not all "
     "components"};

 Gaudi::Property<double> m_cutChiSquaredPerNumberDOF{
     this, "StateChi2PerNDOFCut", 50., "Cut on Chi2 per NDOF"};

 Gaudi::Property<unsigned int> m_maximumNumberOfComponents{
     this, "MaximumNumberOfComponents", 12, "Maximum number of components"};

 DoubleArrayProperty m_smootherCovFactors{
     this,
     "smootherStartCovFactors",
     {15., 5., 15., 5., 15.},
     "Inflation factors for the covariance at the start of smoothing [LocX, LocY, phi, "
     "theta, q / p]"};

 TrkParametersComparisonFunction m_trkParametersComparisonFunction;
 Trk::ParticleHypothesis m_particleHypothesis = Trk::electron;
};

} // end Trk namespace
#include "TrkGaussianSumFilter/GaussianSumFitter.icc"

#endif
