/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSEVENTCNV_ActsToTrkWrappingTool_H
#define ACTSEVENTCNV_ActsToTrkWrappingTool_H

// ATHENA
#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IFitterTool.h"

#include "ActsEvent/ContextUtility.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "ActsToolInterfaces/ITrackConverterTool.h"
#include "ActsGeometryInterfaces/IGeometryRealmConvTool.h"

namespace ActsTrk {
/** @class ActsToTrkFitterWrapTool

    Tool wrapping an ACTS track fitter tool to expose the Trk::ITrackFitter 
    interface. The EDM objects are piped to the Acts::IFitterTool interface
    and processed by the ACTS infrastructure. The successful fit result is 
    then converted back to a Trk::Track object. The wrapper leverages the
    usage of the ACTS fitters in the legacy reconstruction framework and is
    mainly meant to be used for validation and testing purposes.

*/

class ActsToTrkFitterWrapTool : public extends<AthAlgTool, Trk::ITrackFitter> {

  public:
    using base_class::base_class;
    ~ActsToTrkFitterWrapTool() = default;

    StatusCode initialize() override;

    /** @brief Re-fit a track using the ACTS fitter tool.
     *  @param ctx The event context.
     *  @param track The input track.
     *  @param runOutlier Whether to run outlier removal.
     *  @param hypothesis The particle hypothesis.
     *  @return A unique pointer to the fitted track. */
    virtual std::unique_ptr<Trk::Track> 
        fit (const EventContext& ctx,
             const Trk::Track& track,
             const Trk::RunOutlierRemoval runOutlier = false,
             const Trk::ParticleHypothesis hypothesis = Trk::nonInteracting) const override;

    /** @brief Re-fit a track adding a fittable measurement set
     *  @param ctx The event context.
     *  @param track The input track.
     *  @param measSet The measurement set to add.
     *  @param runOutlier Whether to run outlier removal.
     *  @param hypothesis The particle hypothesis.
     *  @return A unique pointer to the fitted track. */
    virtual std::unique_ptr<Trk::Track> 
        fit(const EventContext& ctx,
            const Trk::Track& track,
            const Trk::MeasurementSet& measSet,
            const Trk::RunOutlierRemoval runOutlier = false,
            const Trk::ParticleHypothesis hypothesis = Trk::nonInteracting) const override;
    
    /** @brief Re-fit a track adding a set of prepared raw data
     *  @param ctx The event context.
     *  @param track The input track.
     *  @param prepRawSet The set of prepared raw data to add.
     *  @param runOutlier Whether to run outlier removal.
     *  @param hypothesis The particle hypothesis.
     *  @return A unique pointer to the fitted track. */
    virtual std::unique_ptr<Trk::Track> 
        fit(const EventContext& ctx,
            const Trk::Track& track,
            const Trk::PrepRawDataSet& prepRawSet,
            const Trk::RunOutlierRemoval runOutlier = false,
            const Trk::ParticleHypothesis hypothesis = Trk::nonInteracting) const override;    
    
    /** @brief Fit a track to a set of prepared raw data
     *  @param ctx The event context.
     *  @param prepRawSet The set of prepared raw data.
     *  @param params The initial track parameters.
     *  @param runOutlier Whether to run outlier removal.
     *  @param hypothesis The particle hypothesis.
     *  @return A unique pointer to the fitted track. */
    virtual std::unique_ptr<Trk::Track> 
        fit(const EventContext& ctx,
            const Trk::PrepRawDataSet& prepRawSet,
            const Trk::TrackParameters& params,
            const Trk::RunOutlierRemoval runOutlier = false,
            const Trk::ParticleHypothesis hypothesis = Trk::nonInteracting) const override;

    /** @brief Fit a track to a set of measurements
     *  @param ctx The event context.
     *  @param measSet The measurement set.
     *  @param params The initial track parameters.
     *  @param runOutlier Whether to run outlier removal.
     *  @param hypothesis The particle hypothesis.
     *  @return A unique pointer to the fitted track. */
    virtual std::unique_ptr<Trk::Track> 
        fit(const EventContext& ctx,
            const Trk::MeasurementSet& measSet,
            const Trk::TrackParameters& params,
            const Trk::RunOutlierRemoval runOutlier = false,
            const Trk::ParticleHypothesis hypothesis = Trk::nonInteracting) const override;
    
    /** @brief Combines two tracks by fitting them together
     *  @param ctx The event context.
     *  @param track1 The first track.
     *  @param track2 The second track.
     *  @param runOutlier Whether to run outlier removal.
     *  @param hypothesis The particle hypothesis.
     *  @return A unique pointer to the combined track. */
    virtual std::unique_ptr<Trk::Track> 
        fit(const EventContext& ctx,
            const Trk::Track& track1,
            const Trk::Track& track2,
            const Trk::RunOutlierRemoval runOutlier = false,
            const Trk::ParticleHypothesis hypothesis = Trk::nonInteracting) const override;
    
  private:
    /** @brief Implementation of the logic for re-fitting a track adding a 
     *         set of measurements passed as source links.
     *  @param ctx The event context.
     *  @param track The input track.
     *  @param measColl The collection of measurements.
     *  @param hypothesis The particle hypothesis.
     *  @param useScaledCov Whether to use scaled covariance.
     *  @return A unique pointer to the re-fitted track. */
    std::unique_ptr<Trk::Track> 
        reFitImpl(const EventContext& ctx,
                  const Trk::Track& track,
                  const std::vector<Acts::SourceLink>& measColl,
                  const Trk::ParticleHypothesis hypothesis,
                  const bool useScaledCov = false) const;
    
    /** @brief Implementation of the logic for fitting a track to a set of
     *         measurements passed as source links.
     *  @param ctx The event context.
     *  @param measColl The collection of measurements.
     *  @param params The initial track parameters.
     *  @return A unique pointer to the fitted track. */
    std::unique_ptr<Trk::Track> 
        fitImpl(const EventContext& ctx,
                const std::vector<Acts::SourceLink>& measColl,
                const Acts::BoundTrackParameters& params) const;
                                          

    /** @brief The underlying Acts fitter tool */
    ToolHandle<IFitterTool> m_actsFitterTool{this, "ActsFitterTool", ""};
    
    /** @brief Track converter tool for converting Acts tracks to ATLAS tracks */
    ToolHandle<ITrackConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", ""};
    /** @brief Geometry realm converter tool */
    PublicToolHandle<IGeometryRealmConvTool> m_geometryConvTool{this, "GeometryRealmConvTool", ""};
    /** @brief Auxiliary class to access the magnetic field, geometry and calibration context */
    ContextUtility m_ctxProvider{this};
    /** @brief Property for the seed covariance scale factor */
    Gaudi::Property< double > m_option_seedCovarianceScale {this, "SeedCovarianceScale", 1.,
      "Scale factor for the input seed covariance when doing refitting"};

};

}
#endif
