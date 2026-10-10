/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRUTHCLUSTERSPLITTINGTOOL_IMPL_H
#define ACTSTRACKRECONSTRUCTION_TRUTHCLUSTERSPLITTINGTOOL_IMPL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "StoreGate/ReadHandleKey.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"

#include "ActsToolInterfaces/IPixelOnTrackCalibratorTool.h"
#include "PixelClusterCalibrationToolBase.h"

#include "ActsEvent/MeasurementToTruthParticleAssociation.h"
#include "TrkTruthTrackInterfaces/IAthSelectionTool.h"


namespace ActsTrk::detail {

   template <typename traj_t>
   class TruthClusterSplittingCalibrator;

   /// @brief Options for the truth cluster splitting calibrator
   struct TruthClusterSplittingCalibratorOptions {
        const ActsTrk::MeasurementToTruthParticleAssociation* m_measToTruth{};
        const IAthSelectionTool* m_truthSelectionTool{};
   };

   /// @brief the Truth Cluster Splitting Calibrator
  template <typename traj_t>
  class TruthClusterSplittingCalibrator
     : public PixelClusterCalibratorBase<TruthClusterSplittingCalibrator<traj_t>, traj_t>
  {
  public:
     using BASE=PixelClusterCalibratorBase<TruthClusterSplittingCalibrator<traj_t>, traj_t>;
     friend BASE;

     TruthClusterSplittingCalibrator(PixelClusterCalibratorOptionsBase &&base_options,
                                  TruthClusterSplittingCalibratorOptions &&options)
        : BASE(std::move(base_options)),
          m_options(std::move(options))
     {}

     std::tuple<typename TruthClusterSplittingCalibrator<traj_t>::BASE::Pos,
                typename TruthClusterSplittingCalibrator<traj_t>::BASE::Cov,
                unsigned int>
     calibrate(const EventContext& ctx,
               const Acts::GeometryContext& gctx,
               const Acts::CalibrationContext& cctx,
               const xAOD::PixelCluster& cluster,
               const InDetDD::SiDetectorElement& detElement,
               const std::pair<float, float>& angles,
               const Acts::Vector2& predicted_local_position) const;

   protected:
     TruthClusterSplittingCalibratorOptions m_options;
  };

   /// @brief the tool to create the truth clustering calibrator.
   ///
   /// Determines whether a cluster is created by one or more truth particles.
   /// The definition of a "reconstructable" truth particles follows the IDPVM definition 
   /// for efficiency plots (see config loaded `Tracking/Acts/ActsConfig/python/ActsMeasurementCalibrationConfig.py:68`)
  template <typename traj_t>
  class TruthClusterSplittingToolImpl
     : public PixelClusterCalibrationToolBase<traj_t> {
  public:
    using BASE = PixelClusterCalibrationToolBase<traj_t>;
    using BASE::BASE;

    virtual StatusCode initialize() override;

    virtual std::unique_ptr<PixelOnBoundStateCalibratorBase >  create(const EventContext &ctx) const override {
       return createOnTrackCalibrator(ctx);
    }

    virtual std::unique_ptr<PixelOnTrackCalibratorBase<traj_t> > createOnTrackCalibrator(const EventContext &ctx) const override {
       return std::make_unique<TruthClusterSplittingCalibrator<traj_t> >(this->createBaseOptions(ctx),
                                                                                   this->createOptions(ctx));
    }

  protected:

      virtual TruthClusterSplittingCalibratorOptions createOptions(const EventContext &ctx) const {
         TruthClusterSplittingCalibratorOptions options{
            .m_measToTruth=getAssociationMap(ctx),
            .m_truthSelectionTool=getTruthSelectionTool()
         };
         return options;
      }

      // Read handle key and tool handles
      SG::ReadHandleKey<ActsTrk::MeasurementToTruthParticleAssociation> m_associationMap_key
         {this,"AssociationMapOut","ITkPixelClustersToTruthParticles", "Association map between measurements and truth particles"};
      const ActsTrk::MeasurementToTruthParticleAssociation* getAssociationMap(const EventContext &ctx) const;

      ToolHandle<IAthSelectionTool> m_truthSelectionTool{this, "TruthSelectionTool","AthTruthSelectionTool", "Truth selection tool (for efficiencies and resolutions)"};
      const IAthSelectionTool* getTruthSelectionTool() const;
  };

} // namespace ActsTrk::detail

#include "src/detail/TruthClusterSplittingToolImpl.icc"

#endif
