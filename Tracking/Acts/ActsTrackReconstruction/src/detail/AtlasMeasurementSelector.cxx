/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

#include "ActsEvent/TrackParameters.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/ATLASSourceLink.h"

#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Utilities/VectorHelpers.hpp"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/Utilities/Result.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFinding/TrackStateCreator.hpp"

#include "src/detail/MeasurementSelector.h"
#include "ActsEvent/TrackContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"

#include "src/detail/AtlasUncalibSourceLinkAccessor.h"
#include "src/IMeasurementSelector.h"
#include "src/detail/CalibratorRegistry.h"
#include "src/detail/MeasurementCalibrator2.h"

#include "boost/container/small_vector.hpp"

#include <tuple>
#include <type_traits>
#include <span>
#include <ranges>

#include "src/detail/AtlasMeasurementSelector.h"

template <std::size_t NMeasMax, typename traj_t, typename measurement_container_variant_t>
struct AtlasMeasurementSelector;

static constexpr bool s_fullPreCalibration=true;

// need an "Eigen Map" which is default constructable and assignable
// and where an assignment changes the data pointer not the contents of
// the data it was pointing to.
template <std::size_t DIM>
struct ConstVectorMapWithInvalidDef : public xAOD::ConstVectorMap<DIM> {
   ConstVectorMapWithInvalidDef() : xAOD::ConstVectorMap<DIM>{nullptr} {}
   ConstVectorMapWithInvalidDef<DIM> &operator=(ConstVectorMapWithInvalidDef<DIM> &&a) {
      this->m_data = a.m_data;
      return *this;
   }
   ConstVectorMapWithInvalidDef<DIM> &operator=(const ConstVectorMapWithInvalidDef<DIM> &a) {
      this->m_data = a.m_data;
      return *this;
   }
   ConstVectorMapWithInvalidDef<DIM> &operator=(const xAOD::ConstVectorMap<DIM> &a) {
      this->m_data = a.data();
      return *this;
   }
   using xAOD::ConstVectorMap<DIM>::ConstVectorMap;
};

// see above
template <std::size_t DIM>
struct ConstMatrixMapWithInvalidDef : public xAOD::ConstMatrixMap<DIM> {
   ConstMatrixMapWithInvalidDef() : xAOD::ConstMatrixMap<DIM>{nullptr} {}
   ConstMatrixMapWithInvalidDef<DIM> &operator=(ConstMatrixMapWithInvalidDef<DIM> &&a) {
      this->m_data = a.m_data;
      return *this;
   }
   ConstMatrixMapWithInvalidDef<DIM> &operator=(const ConstMatrixMapWithInvalidDef<DIM> &a) {
      this->m_data = a.m_data;
      return *this;
   }
   ConstMatrixMapWithInvalidDef<DIM> &operator=(const xAOD::ConstMatrixMap<DIM> &a) {
      this->m_data = a.data();
      return *this;
   }
   using xAOD::ConstMatrixMap<DIM>::ConstMatrixMap;
};

/// Types to be used for the measurement selection
/// measurement and covariance types during measurement selection and for the final calibration
/// are different to directly use the value of the uncalibrated measurement
/// @TODO a first assessment does not indicate any performance improvement
template <std::size_t NMeasMax, typename traj_t, typename measurement_container_variant_t>
struct MeasurementSelectorTraits<  AtlasMeasurementSelector<NMeasMax, traj_t, measurement_container_variant_t> >
{
   // the measurement type after the selection e.g. a Matrix<N,1>
   template <std::size_t N>
   using CalibratedMeasurement = xAOD::MeasVector<N>;

   // the  measurement covariance type after the selection e.g. a Matrix<N,N>
   template <std::size_t N>
   using CalibratedMeasurementCovariance = xAOD::MeasMatrix<N>;

   // the measurement type before the selection e.g. an Eigen::Map< Matrix<N,1> > if
   // the calibration is performed after the selection
   template <std::size_t N>
   using PreSelectionMeasurement = std::conditional< s_fullPreCalibration,
                                                     CalibratedMeasurement<N>,
                                                     ConstVectorMapWithInvalidDef<N> >::type;

   // the measurement covariance type before the selection e.g. an Eigen::Map<Matrix<N,N> > if
   // the calibration is performed after the selection
   template <std::size_t N>
   using PreSelectionMeasurementCovariance = std::conditional< s_fullPreCalibration,
                                                               CalibratedMeasurementCovariance<N>,
                                                               ConstMatrixMapWithInvalidDef<N> >::type;
   // e.g. the same as CalibratedMeasurement
   template <std::size_t N>
   using Predicted = CalibratedMeasurement<N>;

   // e.g. the same as CalibratedMeasurementCovariance
   template <std::size_t N>
   using PredictedCovariance = CalibratedMeasurementCovariance<N>;

   // e.g. helper template to get the value_type from the measurement range iterator type
   template <typename T_MeasurementRangeIterator>
   struct MeasurementContainerTraits {
      using value_type = typename T_MeasurementRangeIterator::value_type;
   };

   using abstract_measurement_range_t = std::ranges::iota_view<unsigned int, unsigned int>;

   // the trajectory type to which states for selected measurements are to be added
   using trajectory_t = traj_t;
   // the track state type for new track states
   using TrackStateProxy = typename traj_t::TrackStateProxy;

   // the value type usd for matrices
   using MatrixFloatType = double;
   using BoundTrackParameters = Acts::BoundTrackParameters;
   using BoundMatrix = Acts::BoundMatrix;

   using BoundState = std::tuple<BoundTrackParameters, BoundMatrix, double>;

   // maximum dimension of measurements determined from the measurement "container" variant
  static const std::size_t s_dimMax = ActsTrk::detail::AtlasMeasurementContainerList::getMeasurementDimMax();

   // must be the same as what is used for the CKF
   static constexpr std::size_t s_maxBranchesPerSurface = 10;
};

/// ATALS specific Implementation of the member functions needed by the measurement selector
template <std::size_t NMeasMax, typename traj_t, typename measurement_container_variant_t>
struct AtlasMeasurementSelector
   : public MeasurementSelectorBaseImpl<NMeasMax,
                                        AtlasMeasurementSelector<NMeasMax, traj_t, measurement_container_variant_t> ,
                                        measurement_container_variant_t >
{
   // @TODO can these redundent definitions been avoided ?
   using BASE = MeasurementSelectorBaseImpl<NMeasMax,
      AtlasMeasurementSelector<NMeasMax, traj_t, measurement_container_variant_t> ,
                                            measurement_container_variant_t >;
   // BASE::trats are MeasurementSelectorTraits< ... decltype(*this) >
   using traits = typename BASE::traits;

   template <std::size_t DIM>
   using Measurement = typename traits::template CalibratedMeasurement<DIM>;

   using abstract_measurement_range_t = BASE::abstract_measurement_range_t;

   // the delegate used for the final calibration
   template <std::size_t DIM, typename measurement_t>
   using Calibrator = Acts::Delegate<
      std::pair < typename traits::template CalibratedMeasurement<DIM>,
                  typename traits::template CalibratedMeasurementCovariance<DIM> >
                (const Acts::GeometryContext&,
                 const Acts::CalibrationContext&,
                 const measurement_t &,
                 const typename traits::BoundTrackParameters &)>;

   // helper to determine the calibrator type from the measurement container tyoe
   // and to define the types used for the calibrated measurement and covariance
   struct CalibratedMeasurementTraits {
      // the types used for the calibrated measurement and covariance
      template <std::size_t DIM>
      using Measurement = typename traits::template CalibratedMeasurement<DIM>;
      template <std::size_t DIM>
      using MeasurementCovariance = typename traits::template CalibratedMeasurementCovariance<DIM>;

      // helper to determine the measurement value type from the container type
      template <typename T_Container>
      using MeassurementContainerValueType = typename traits::template MeasurementContainerTraits<T_Container>::value_type;
   };

   const ActsTrk::detail::MeasurementRangeList *m_measurementRanges{};
   // Helper to provide the mapping between bound parameters and coordinates
   // @TODO is the default projector always good enough or is there some dependency
   //       on the geoemtry ?
   ActsTrk::MeasurementParameterMap m_projector{};
   CalibratorRegistry< CalibratedMeasurementTraits, typename traits::BoundTrackParameters, measurement_container_variant_t>  m_calibrators {};
   struct Empty {};
   std::conditional<s_fullPreCalibration,
      CalibratorRegistry< CalibratedMeasurementTraits, typename traits::BoundTrackParameters, measurement_container_variant_t>,
                    Empty>::type m_preCalibrators {};


   AtlasMeasurementSelector(typename BASE::Config &&config,
                            const ActsTrk::detail::MeasurementRangeList &measurementRanges)
      : BASE{std::move(config)},
        m_measurementRanges(&measurementRanges)
   {}

   // register  a calibrator for the given measurement type and the measurement dimension i.e. number of coordinates
   template <std::size_t DIM, typename T_ValueType>
   void setCalibrator(const Calibrator<DIM, T_ValueType> &calibrator) {
      m_calibrators.template setCalibrator<DIM, T_ValueType>(calibrator);
   }

   template <std::size_t DIM>
   static constexpr bool s_CanPreCalibrate =    std::is_same< typename traits::template PreSelectionMeasurement<DIM>,
                                                              typename traits::template CalibratedMeasurement<DIM> >::value
                                             && std::is_same< typename traits::template PreSelectionMeasurementCovariance<DIM>,
                                                              typename traits::template CalibratedMeasurementCovariance<DIM> >::value;

   template < std::size_t DIM, typename T_ValueType >
   void setPreCalibrator(typename std::enable_if<s_CanPreCalibrate<DIM>, const Calibrator<DIM, T_ValueType> &>::type calibrator) {
      m_preCalibrators.template setCalibrator<DIM, T_ValueType>(calibrator);
   }

   // helper to create a Acts::SourceLink from an uncalibrated measurement pointer
   template <typename T_Value>
   static Acts::SourceLink makeSourceLink(T_Value &&value) {
      // value is pointer
      static_assert( !std::is_same<std::remove_pointer_t<T_Value>, T_Value>::value );
      // ... and pointer to xAOD::UncalibgratedMeasurement
      static_assert(std::is_base_of_v< xAOD::UncalibratedMeasurement, std::remove_cv_t<std::remove_pointer_t<T_Value> > > );
      return Acts::SourceLink{ ActsTrk::makeATLASUncalibSourceLink(value) };
   }

   // helper to provide a map from bound parameters to coordinates
   template <std::size_t DIM>
   ParameterMapping::type<DIM>
   parameterMap(const Acts::GeometryContext& geometryContext,
                const Acts::CalibrationContext& calibrationContext,
                const Acts::Surface& surface) const {
      return m_projector.parameterMap<DIM>(geometryContext,calibrationContext,surface);
   }

   // helper which returns a delegate to perform the post calibration for the given measurement type and dimension
   template <std::size_t DIM, typename measurement_t>
   const Calibrator<DIM, measurement_t> &
   postCalibrator() const {
      return m_calibrators.template calibrator<DIM,measurement_t>();
   }

   // helper which returns a delegate or lambda to get the measurement and covariance used during the selection
   template <std::size_t DIM, typename measurement_t>
   auto
   preCalibrator() const {
      if constexpr(s_fullPreCalibration) {
         // full calibration during selection
         return m_preCalibrators.template calibrator<DIM,measurement_t>();
      }
      else  {
         // no calibration is performed before the measurement selection, so just Eigen maps to the stored location
         // and covariance are returned.
         // @TODO unfortunately the Eigen Maps cannot be used directly because they are incompatible with the
         //       temporary measurement storage used in the measurement selector, so need to convert to the
         //       above ConstVectorMapWithInvalidDef etc. Does this introduce some overhead ?
         return []( [[maybe_unused]] const Acts::GeometryContext&,
                    [[maybe_unused]] const Acts::CalibrationContext&,
                    const measurement_t &measurement,
                    [[maybe_unused]] const typename traits::BoundTrackParameters &) {
               return std::make_pair( measurement.template localPosition<DIM>(), measurement.template localCovariance<DIM>() );
             };
      }
   }

   std::tuple<const measurement_container_variant_t *, abstract_measurement_range_t >
   containerAndRange(const Acts::Surface &surface) const {

      const ActsTrk::detail::MeasurementRangeList::const_iterator
         range_iter = m_measurementRanges->find(surface.geometryId().value());
      if (range_iter == m_measurementRanges->end())
      {
         return {nullptr, abstract_measurement_range_t{}};
      }
      else {
         abstract_measurement_range_t range{range_iter->second.elementBeginIndex(),
                                            range_iter->second.elementEndIndex()};
         assert( !range_iter->second.isMeasurementExpected() || range.begin() <= range.end());
         // if surface marked as defect
         return { range_iter->second.isMeasurementExpected() ? &(m_measurementRanges->container(range_iter->second.containerIndex())) : nullptr,
                 std::move(range)};
      }
   }

   bool expectMeasurements([[maybe_unused]] const Acts::Surface &surface,
                           [[maybe_unused]] const measurement_container_variant_t *container_variant_ptr,
                           const abstract_measurement_range_t &abstract_range) const {
      return (abstract_range.begin()<=abstract_range.end());
   }

   template <typename measurement_container_t>
   auto
   rangeForContainer(const measurement_container_t &concrete_container,
                     const abstract_measurement_range_t &abstract_range) const {
      unsigned int begin_idx = abstract_range.front();
      auto begin_iter = concrete_container.container().begin() + begin_idx;
      auto end_iter = begin_iter + static_cast<unsigned int>(abstract_range.size());
      return  std::ranges::subrange(begin_iter, end_iter);
   }
};

namespace {
   // the track back-end used during track finding
   using RecoTrackContainer = Acts::TrackContainer<Acts::VectorTrackContainer,
                                                   Acts::VectorMultiTrajectory>;

   static constexpr std::size_t gAbsoluteMaxBranchesPerSurface = 3; // the absolute maximum number of branches per surface
                                                                    // the actual value is configurable up to this number


   // Wrapper class which provides the actual measurement selector and
   // allows to connect it to the delegate used by the track finder
   template <typename track_container_t>
   class AtlasActsMeasurmentSelector : public ActsTrk::IMeasurementSelector {
   public:
      using TheAtlasMeasurementSelector
               = AtlasMeasurementSelector<
                     gAbsoluteMaxBranchesPerSurface,
                     typename track_container_t::TrackStateContainerBackend,
                     ActsTrk::detail::AtlasMeasurementContainerList::measurement_container_variant_t
                     // where measurement_container_variant_t is e.g.
                     //   variant<  ContainerRefWithDim<xAOD::PixelClusterContainer,2>, ... >
                    >;

      using BoundState = std::tuple<Acts::BoundTrackParameters, Acts::BoundMatrix, double>;
      // the delegate used by the track finder to which the measurement selector needs to be connected to

      AtlasActsMeasurmentSelector(ActsTrk::MeasurementCalibrator2 &&calibrator,
                                  const ActsTrk::detail::MeasurementRangeList &measurementRanges,
                                  TheAtlasMeasurementSelector::Config &&config)
         : m_calibrator( std::move(calibrator)),
           m_measurementSelector(std::move(config),
                                 measurementRanges)
      {
         // have to register one calibrator per measurement container type and associated dimension.
         // @TODO unfortunately automatic type deduction does not work, so have to provide the type
         //       additionally
         if constexpr( s_fullPreCalibration) {
            m_measurementSelector.template setPreCalibrator<2,xAOD::PixelCluster>(m_calibrator.pixelPreCalibrator());
            m_measurementSelector.template setPreCalibrator<1,xAOD::StripCluster>(m_calibrator.stripPreCalibrator());
            m_measurementSelector.template setPreCalibrator<3,xAOD::HGTDCluster>(m_calibrator.hgtdPreCalibrator());
            m_measurementSelector.template setCalibrator<2,xAOD::PixelCluster>(m_calibrator.pixelPostCalibrator());
            m_measurementSelector.template setCalibrator<1,xAOD::StripCluster>(m_calibrator.stripPostCalibrator());
            m_measurementSelector.template setCalibrator<3,xAOD::HGTDCluster>(m_calibrator.hgtdPostCalibrator());
         }
         else {
            m_measurementSelector.template setCalibrator<2,xAOD::PixelCluster>(m_calibrator.pixelPostCalibrator());
            m_measurementSelector.template setCalibrator<1,xAOD::StripCluster>(m_calibrator.stripPostCalibrator());
            m_measurementSelector.template setCalibrator<3,xAOD::HGTDCluster>(m_calibrator.hgtdPostCalibrator());
         }
      }

      // called by the track finder to connect this measurement selector to the ckf.
      void connect(std::any delegate_ptr) const override {
         using TrackStateCreator = Acts::CombinatorialKalmanFilterExtensions<RecoTrackContainer>::TrackStateCreator;

         auto delegate = std::any_cast< TrackStateCreator *>(delegate_ptr);
         delegate->template connect< & TheAtlasMeasurementSelector::createTrackStates >(&m_measurementSelector);
      }

      // provides the calibrators
      ActsTrk::MeasurementCalibrator2         m_calibrator;

      // the actual measurement selector
      TheAtlasMeasurementSelector             m_measurementSelector;
   };
}

namespace ActsTrk::detail {
// return a configured, wrapper for the measurement selector
std::unique_ptr<ActsTrk::IMeasurementSelector>  getMeasurementSelector(const ActsTrk::IOnBoundStateCalibratorTool *onTrackCalibratorTool,
                                                                       const ActsTrk::detail::MeasurementRangeList &measurementRanges,
                                                                       const std::vector<float> &etaBinsf,
                                                                       const std::vector<std::pair<float, float> > &chi2CutOffOutlier,
                                                                       const std::vector<size_t> &numMeasurementsCutOff) {

    // set calibrators per measurement container type (order does not matter);
    ActsTrk::MeasurementCalibrator2 atl_measurement_calibrator(onTrackCalibratorTool);
    using AtlMeasurementSelectorCuts = AtlasMeasurementSelectorCuts;

    using AtlMeasurementSelector = AtlasActsMeasurmentSelector<RecoTrackContainer>;
    using AtlMeasurementSelectorConfig = AtlMeasurementSelector::TheAtlasMeasurementSelector::Config;

    std::unique_ptr<ActsTrk::IMeasurementSelector>
       selector(new AtlMeasurementSelector(
                           std::move(atl_measurement_calibrator),
                           measurementRanges,
                           AtlMeasurementSelectorConfig{ {Acts::GeometryIdentifier(),
                                                          AtlMeasurementSelectorCuts{ etaBinsf,
                                                                                      chi2CutOffOutlier,
                                                                                      numMeasurementsCutOff} }}));
    return selector;

}
}
