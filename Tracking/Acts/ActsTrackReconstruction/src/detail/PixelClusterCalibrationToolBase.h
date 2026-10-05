/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_PIXELCLUSTERCALIBRATIONTOOLBASE_H
#define ACTSTRACKRECONSTRUCTION_PIXELCLUSTERCALIBRATIONTOOLBASE_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelConditionsData/ITkPixelOfflineCalibData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"

#include "ActsToolInterfaces/IPixelOnTrackCalibratorTool.h"

#include "Acts/EventData/TrackStateType.hpp"

namespace ActsTrk::detail {
   /// @brief the base options the options of the options of every PixelClusterCalibrator must be based on
   struct PixelClusterCalibratorOptionsBase {
      const ISiLorentzAngleTool *m_lorentzAngleTool;
      const PixelID* m_pixelID;
   };

   /// @brief the common base class of a PixelClusterCalibrator.
   /// the class provides common convenience functions to get the detector element for a certain module,
   /// and Lorentz angle corrected incidence angles relative to the module reference frame, provides
   /// the pixel ID helper and the shift due to the Lorentz angle.
   template <typename traj_t>
   class PixelClusterCalibratorCommon :  public ActsTrk::PixelOnTrackCalibratorBase<traj_t>
   {
   public:
      explicit PixelClusterCalibratorCommon(PixelClusterCalibratorOptionsBase &&base_options)
         : m_baseOptions(std::move(base_options))
      {
         assert(m_baseOptions.m_pixelID);
         assert(m_baseOptions.m_lorentzAngleTool);
      }

   protected:
      using Pos = xAOD::MeasVector<2>; // the coordinates of a pixel cluster
      using Cov = xAOD::MeasMatrix<2>; // the corresponding covariance

      /// @brief convenience method to get the detector element for a module
      /// @param surface the surface of a module
      const InDetDD::SiDetectorElement& getDetectorElement(const Acts::Surface &surface) const;

      /// @brief get the Pixel ID helper.
      const PixelID &pixelID() const { return *m_baseOptions.m_pixelID; }

      /// @breif get the lorentz shift for the given module.
      double getLorentzShift(const IdentifierHash& elementHash, const EventContext& ctx) const {
         return m_baseOptions.m_lorentzAngleTool->getLorentzShift(elementHash, ctx);
      }
      /// @brief single calibrate method serving OnTrack and OnBoundState cluster calibration
      /// @param gctx the geometry context
      /// @param cctx the calibration context
      /// @param cluster the uncalibrated pixel cluster
      /// @param detElement the detector element of the corresponding module
      /// @param tan_incident_angles tan of the Lorentz angle corrected incidence angles in local x
      ///        and local y which are the projections of the trajectory on the surface.
      /// @param predicted_local_position local position (loc0, loc1) of the trajectory on the surface,
      ///        e.g. to choose between the positions of the particles which created a merged cluster.
      /// this is the method the derived class should overload. The default method
      /// is equivalent to the passThrough calibrator, with likely more overhead.
      /// @return calibrated positions and corresponding covariance.
      std::pair<typename PixelClusterCalibratorCommon<traj_t>::Pos,
                typename PixelClusterCalibratorCommon<traj_t>::Cov>
      calibrate([[maybe_unused]] const EventContext &ctx,
                [[maybe_unused]] const Acts::GeometryContext& gctx,
                [[maybe_unused]] const Acts::CalibrationContext& cctx,
                const xAOD::PixelCluster& cluster,
                [[maybe_unused]] const InDetDD::SiDetectorElement& detElement,
                [[maybe_unused]] const std::pair<float, float>& tan_incident_angles,
                [[maybe_unused]] const Acts::Vector2& predicted_local_position) const {
         return std::make_pair(cluster.template localPosition<2>(),
                               cluster.template localCovariance<2>());
      }

      ///@brief compute tan of Lorentz angle corrected incidence angles in local-x and local-y direction.
      std::pair<float, float>
      tanAnglesOfIncidence(const EventContext& ctx,
                           const Acts::GeometryContext& gctx,
                           const Acts::Surface &surface,
                           const InDetDD::SiDetectorElement& element,
                           const Acts::Vector3& direction) const;

   protected:
      PixelClusterCalibratorOptionsBase m_baseOptions; // the common options of a pixel calibrator
   };

   template <typename derived_t, typename traj_t>
   class PixelClusterCalibratorBase : public PixelClusterCalibratorCommon<traj_t>
   {
   public:
      using BASE=PixelClusterCalibratorCommon<traj_t>;
      using BASE::BASE;
      using OnTrackCalibrator=OnTrackCalibratorBase<xAOD::PixelCluster,2,traj_t>::OnTrackCalibrator;
      using TrackStateProxy = OnTrackCalibratorBase<xAOD::PixelCluster,2,traj_t>::TrackStateProxy;
      using Calibrator=OnBoundStateCalibratorBase<xAOD::PixelCluster,2>::Calibrator;
      using Pos = BASE::Pos;
      using Cov = BASE::Cov;

      const derived_t &derived() const
         requires(std::is_base_of_v<std::remove_cvref_t<decltype( *this )>, derived_t >)
      {
         return *static_cast<const derived_t *>(this);
      }

      void calibrate(const Acts::GeometryContext&,
                     const Acts::CalibrationContext&,
                     const xAOD::PixelCluster&,
                     TrackStateProxy&) const;

      std::tuple<Pos, Cov, unsigned int> calibrate(const Acts::GeometryContext&,
                                                   const Acts::CalibrationContext&,
                                                   const Acts::Surface&,
                                                   const xAOD::PixelCluster&,
                                                   const Acts::BoundTrackParameters&) const;

      /// @brief connect the calibrator (derived class) to the given OnTrack calaibrator delegate
      virtual void connectOnTrackCalibrator(OnTrackCalibrator& calibrator) const override;

      /// @brief connect the calibrator (derived class) to the given OnBoundState calaibrator delegate
      virtual void connectCalibrator(Calibrator& calibrator) const override;
   };

   /// @brief base class of a Pixel cluster calibration tool
   /// In addition to some common functionality provied by @ref PixelClusterCalibrationCommon
   /// this class implements the methods to connect the calibrator to the calibrator delegates, and the
   /// OnTrack and OnBoundState interfaces which will call a single calibrate method.
   /// The derived class should overload  calibrate(gctx,cctx,cluster,detElement,tan_incident_angles).
   /// The derived tool has still to implement the method createOnTrackCalibrator(ctx).
   /// If the derived class overloads initialize, initialize of this class must be called.
   template <typename traj_t>
   class PixelClusterCalibrationToolBase
      : public extends<AthAlgTool, ActsTrk::IPixelOnTrackCalibratorTool<traj_t>> {
   public:
      using base_class = typename extends<AthAlgTool, ActsTrk::IPixelOnTrackCalibratorTool<traj_t>>::base_class;
      using base_class::base_class;

      /// @brief initializes this base class (must be called by the derived class)
      virtual StatusCode initialize() override;

      /// @brief convenience class to create an OnBoundState calibrator from an OnTrack calibrator.
      virtual std::unique_ptr<PixelOnBoundStateCalibratorBase > create(const EventContext &ctx) const override {
         return this->createOnTrackCalibrator(ctx);
      }

      /// @brief test whether the calibration should be applied after measurement selection (faster)
      virtual bool calibrateAfterMeasurementSelection() const override;

   protected:
      /// @brief create options needed by the calibrator base class.
      PixelClusterCalibratorOptionsBase createBaseOptions(const EventContext &/*ctx*/) const {
         return PixelClusterCalibratorOptionsBase{
            .m_lorentzAngleTool=&(*m_lorentzAngleTool),
            .m_pixelID=m_pixelID,
         };
      }

      // @TODO should use the Lorentz angle conditions data directly
      ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool {this, "PixelLorentzAngleTool", "",
                                                          "Tool to retreive Lorentz angle"
      };

      Gaudi::Property<bool> m_postCalibration{this, "CalibrateAfterMeasurementSelection", true};

      const PixelID *m_pixelID{}; // The helper object to interpret identifiers
   };

} // namespace ActsTrk::detail

#include "src/detail/PixelClusterCalibrationToolBase.icc"

#endif
