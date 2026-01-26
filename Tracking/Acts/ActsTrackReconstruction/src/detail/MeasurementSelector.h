/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#pragma once
// Alternative measurement selector
//
// This measurement selector is assuming the following
// - the number of selected measurements is small (~<10)
// - the number of measurement candidates is typically >> the
//   number of selected measuremebts
// - the total number of candidate measurements can be large
// - there is a simple one-to-one relation between bound state
//   parameters and measurement coordinates.

#include "Acts/Utilities/Result.hpp"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilterError.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Geometry/GeometryHierarchyMap.hpp"
#include "Acts/EventData/Types.hpp"
#include "Acts/EventData/TrackStatePropMask.hpp"
#include "boost/container/small_vector.hpp"

// for BaseTypes
#include "Acts/EventData/TrackStateProxy.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/EventData/TrackParameters.hpp"

// for MeasurementSizeMax
#include "Acts/EventData/MultiTrajectory.hpp"

#include <utility>
#include <type_traits>
#include <optional>

// Types to be used during measurement selection for the prediction and the
// measurement for calibrated measurements after selection if the actual calibration is
// executed after the calibration, and the trajectory, track state, bound parameter
// types.
template <typename derived_t>
struct MeasurementSelectorTraits
{
   // the measurement type after the selection e.g. a Matrix<N,1>
   template <std::size_t N>
   using CalibratedMeasurement = typename Acts::detail_lt::FixedSizeTypes<N>::Coefficients;

   // the  measurement covariance type after the selection e.g. a Matrix<N,N>
   template <std::size_t N>
   using CalibratedMeasurementCovariance = typename Acts::detail_lt::FixedSizeTypes<N>::Covariance;

   // the measurement type before the selection e.g. an Eigen::Map< Matrix<N,1> > if
   // the calibration is performed after the selection
   template <std::size_t N>
   using PreSelectionMeasurement = typename Acts::detail_lt::FixedSizeTypes<N>::Coefficients;

   // the measurement covariance type before the selection e.g. an Eigen::Map<Matrix<N,N> > if
   // the calibration is performed after the selection
   template <std::size_t N>
   using PreSelectionMeasurementCovariance = typename Acts::detail_lt::FixedSizeTypes<N>::Covariance;

   // e.g. the same as CalibratedMeasurement
   template <std::size_t N>
   using Predicted = typename Acts::detail_lt::FixedSizeTypes<N>::Coefficients;

   // e.g. the same as CalibratedMeasurementCovariance
   template <std::size_t N>
   using PredictedCovariance = typename Acts::detail_lt::FixedSizeTypes<N>::Covariance;

   // e.g. helper template to get the value_type from the container type
   // e.g. helper template to get the value_type from the measurement range iterator type

   template <typename T_MeasurementRangeIterator>
   struct MeasurementContainerTraits {
      using value_type = typename T_MeasurementRangeIterator::value_type;
   };

   // abstract measurement range
   // implements empty()
   // and can be converted by the derived_t into an iterable range
   // over measurements of concrete measurement container_type.
   using abstract_measurement_range_t = std::ranges::iota_view<unsigned int, unsigned int>;

   // the trajectory type to which states for selected measurements are to be added
   using trajectory_t = typename derived_t::traj_t;
   // the track state type for new track states
   using TrackStateProxy = trajectory_t::TrackStateProxy;

   // the value type usd for matrices
   using MatrixFloatType = double;
   using BoundTrackParameters = Acts::BoundTrackParameters;
   using BoundMatrix = Acts::BoundMatrix;

   using BoundState = std::tuple<BoundTrackParameters, BoundMatrix, double>;

   // maximum dimension of measurements
   static const std::size_t s_dimMax = 3;

   // must be the same as what is used for the CKF
   // @TODO how to get rid of this ?
   static constexpr std::size_t s_maxBranchesPerSurface = 10;
};

//
struct MeasurementSelectorMatrixTraits {
   /// matrix adapter for Eigen
   /// additionally need +,- and *
   template <class T_Matrix>
   static constexpr std::size_t matrixColumns() { return T_Matrix::ColsAtCompileTime;}
   template <class T_Matrix>
   static constexpr std::size_t matrixRows() { return T_Matrix::RowsAtCompileTime;}

   template <typename T_Float, class T_Matrix>
   static auto matrixTypeCast(const T_Matrix &matrix) { return matrix.template cast<T_Float>(); }

   template <class T_Matrix>
   static auto transpose(const T_Matrix &matrix) { return matrix.transpose(); }

   template <class T_Matrix>
   static auto invert(const T_Matrix &matrix) { return matrix.inverse(); }
};


// Map from the measurement to bound state domain
// it is assumed that there is a simple unambiguous association
// between coordinates of the measurement domain and the bound state domain
// e.g. measurement coordinate 1 maps to loc0 of the bound state.
// @TODO use FixedSizeSubspace ? currently does not provide methods to directly
//       create the a sub-space covariance matrix from a full covariance matrix,
//       and to create the projector bitset, which is stored in the TrackState.
struct ParameterMapping {

   template <std::size_t N>
   using type = std::array<unsigned char, N>;

   template <std::size_t N>
   static constexpr type<N> identity() {
      type<N> ret;
      for(int i=0; i<N; ++i) {
         ret[i]=i;
      }
      return ret;
   }
};

// utility to "project" a bound state parameter vector or covariance matrix onto the 1,2, ... N dimensional measurement domain
// @TODO allow to influence resulting matrix type ?
template <std::size_t N,class T_ResultType,class T_Matrix>
T_ResultType project(ParameterMapping::type<N> parameter_map, const T_Matrix &matrix)
{
   using MatrixIndexMapType = unsigned char; // "char" to reduce the size of the map, and if not wide enough this entire
                                             //        concept is likely inefficient.
   using MatrixIndexType = unsigned int;     // @TODO or std::size_t ? does not matter

   // ensure that index types are wide enough
   static_assert( MeasurementSelectorMatrixTraits::matrixRows<T_Matrix>() < std::numeric_limits<MatrixIndexMapType>::max());
   static_assert( N*MeasurementSelectorMatrixTraits::matrixRows<T_Matrix>() < std::numeric_limits<MatrixIndexType>::max());

   T_ResultType ret;
   if constexpr(MeasurementSelectorMatrixTraits::matrixColumns<T_Matrix>() == 1) {
      // handle projection of paramteter vector
      for (MatrixIndexType meas_i=0; meas_i<N; ++meas_i) {
         assert( meas_i < parameter_map.size() );
         ret(meas_i,0) = matrix( parameter_map[meas_i], 0);
      }
   }
   else {
      // handle projection of covariance matrix
      // "project" matrix
      for (MatrixIndexType meas_i=0; meas_i<N; ++meas_i) {
         assert( meas_i < parameter_map.size());
         MatrixIndexType param_i = parameter_map[meas_i];
         for (MatrixIndexType meas_j=0; meas_j<N; ++meas_j) {
            assert( meas_j < parameter_map.size());
            ret(meas_i,meas_j) = matrix(param_i, parameter_map[meas_j]);
         }
      }
   }
   return ret;
}




// helper method to compute a chi2 for the difference of two "measurement" and covariance pairs
template <typename measurement_vector_t, typename measurement_cov_matrix_t,
          typename predicted_vector_t, typename predicted_cov_matrix_t>
double computeChi2(const measurement_vector_t &a,
                   const measurement_cov_matrix_t &a_cov,
                   const predicted_vector_t &b,
                   const predicted_cov_matrix_t &b_cov) {

   // just sum sanity checks that a and b have the correct dimensions and that
   // the chi2 can actually be computed.
   static_assert( MeasurementSelectorMatrixTraits::matrixColumns<measurement_vector_t>() == 1); // a is vector
   static_assert( MeasurementSelectorMatrixTraits::matrixColumns<predicted_vector_t>() == 1); // b is vector
   static_assert( MeasurementSelectorMatrixTraits::matrixRows<measurement_cov_matrix_t>()
                  == MeasurementSelectorMatrixTraits::matrixColumns<measurement_cov_matrix_t>() ); // a is square matrix
   static_assert( MeasurementSelectorMatrixTraits::matrixRows<predicted_cov_matrix_t>()
                  == MeasurementSelectorMatrixTraits::matrixColumns<predicted_cov_matrix_t>() ); // b is square matrix
   static_assert( MeasurementSelectorMatrixTraits::matrixRows<measurement_cov_matrix_t>()
                  == MeasurementSelectorMatrixTraits::matrixRows<measurement_vector_t>() );        // a vector matches matrix
   static_assert( MeasurementSelectorMatrixTraits::matrixRows<predicted_cov_matrix_t>()
                  == MeasurementSelectorMatrixTraits::matrixRows<predicted_vector_t>() );        // b vector matches matrix
   static_assert( MeasurementSelectorMatrixTraits::matrixRows<measurement_cov_matrix_t>()
                  == MeasurementSelectorMatrixTraits::matrixRows<predicted_cov_matrix_t>() );    // a and b match

   // @TODO remove abstraction i.e. assume matrix has the interface of an eigen matrix
   auto inv_ab_cov( MeasurementSelectorMatrixTraits::invert(a_cov+b_cov) );
   auto  diff( a-b);
   return (MeasurementSelectorMatrixTraits::transpose(diff) * inv_ab_cov * diff)(0,0);
}

// Collection to hold the n-"best" candidates
// The objects of type PayloadType must support assignment operation, and must
// be default constructible. Moreover it must be possible to provide a
// "comparison" operator to order the payload objects.
template <std::size_t N, class PayloadType >
struct TopCollection {
   using IndexType = unsigned short; // @TODO or char ? If N>>10 this concept is likely
                                     //       inefficient
   //   using PayloadType = Payload<DIM>;
   TopCollection(std::size_t max_n) {
      init(max_n);
   }

   // @param max_n the maximum number of top-candidates is fixed by the template parameter
   //    N but can be reduced further to this number
   void init(std::size_t max_n) {
      assert( max_n < N);
      m_nextSlot=0;
      m_maxSlots=max_n;
      m_order[0]=0;
   }
   // @param get a slot to hold a new candidate which is not necessarily accepted in the list
   //     of the n-top candidates
   PayloadType &slot() {
      return m_slots[m_order[m_nextSlot] ];
   }
   // @param idx get the specified filled slot (read only) indicated by the index, where the index does not
   //    indicate the order in the top-candidate list
   const PayloadType &getSlot(IndexType idx) const {
      return m_slots[idx];
   }
   // @param idx get the specified filled slot indicated by the index, where the index does not
   //    indicate the order in the top-candidate list
   PayloadType &getSlot(IndexType idx) {
      return m_slots[idx];
   }
   // @param test whether the given index points to one of the accepted top candidates
   bool isValid(IndexType idx) const {
      return idx < m_nextSlot;
   }
   // Accept the element of the latest slot provided there is still a free slot or it is better than
   // the worst element.
   // @param comparison operator to compute e.g. a<b where "smaller" means "better"
   void acceptAndSort(std::function<bool(const PayloadType &a, const PayloadType &b)> comparison) {
      // bubble best element to top
      for (unsigned int slot_i = m_nextSlot;
           slot_i-- > 0
           && !comparison(m_slots[ m_order[slot_i] ],m_slots[ m_order[slot_i+1] ]);) {
         std::swap(m_order[slot_i],m_order[slot_i+1]);
      }
      // if there are still free slot increase the number of used slots
      if (m_nextSlot < m_maxSlots) {
         ++m_nextSlot;
         m_order[m_nextSlot]=m_nextSlot;
      }
   }
   // Accept the element of the latest slot provided there is still a free slot.
   bool acceptNoSort() {
      // if there are still free slot increase the number of used slots
      if (m_nextSlot < m_maxSlots) {
         ++m_nextSlot;
         m_order[m_nextSlot]=m_nextSlot;
      }
      return (m_nextSlot < m_maxSlots);
   }

   bool empty() const {
      return m_nextSlot==0;
   }

   IndexType size() const { return m_nextSlot; }

   // helper to iterate over the slot indices in sorting order from best to worst
   typename std::array<IndexType, N+1>::const_iterator begin() { return m_order.begin(); }
   typename std::array<IndexType, N+1>::const_iterator end()   { return m_order.begin()+m_nextSlot; }

   std::array<PayloadType, N+1> m_slots;         // storage for the slots
   std::array<IndexType,   N+1> m_order;         // order of the filled slots
   IndexType                    m_nextSlot = 0;  // the index of the next free slot
   IndexType                    m_maxSlots = 0;  // maximum number of top-slots
};



struct AtlasMeasurementSelectorCuts {
  /// bins in |eta| to specify variable selections
  std::vector<float> etaBins{};
  /// Maximum local chi2 contribution.
  std::vector<std::pair<float, float> > chi2CutOff{ {15,25} };
  /// Maximum number of associated measurements on a single surface.
  std::vector<std::size_t> numMeasurementsCutOff{1};
  /// Optional (expected negative) boundary tolerance to label edge holes as no measurement expected
  std::optional<Acts::BoundaryTolerance> edgeTolerance{};
};

// Measurement type specific measirement selector
// Assumptions:
//  - begin and end source_link_iterator point to a contiguous range of measurements in a single container
//  - all measurements in this range have the same dimensionality.
//  - the mapping of the bound parameters to the measurement domain for each measurement in this range
//    is identical
template <std::size_t NMeasMax,
          std::size_t DIMMAX,
          typename derived_t >
struct MeasurementSelectorBase {

   using Config = Acts::GeometryHierarchyMap<AtlasMeasurementSelectorCuts>;
   using traits = MeasurementSelectorTraits<derived_t>;

   Config m_config;

protected:
   using T_BoundState = typename MeasurementSelectorTraits<derived_t>::BoundState;
   using trajectory_t = typename MeasurementSelectorTraits<derived_t>::trajectory_t;
   using TrackStateProxy = typename MeasurementSelectorTraits<derived_t>::TrackStateProxy;
   static constexpr std::size_t s_maxBranchesPerSurface = MeasurementSelectorTraits<derived_t>::s_maxBranchesPerSurface;

   const derived_t &derived() const { return *static_cast<const derived_t *>(this); }

   // helper to create a projector bitset from a map from bound parameters to coordinates
   struct ProjectorBitSetMaker {
      template <std::size_t N>
      static
      Acts::ProjectorBitset create(const ParameterMapping::type<N> &parameter_map) {
         constexpr std::size_t nrows = Acts::MultiTrajectoryTraits::MeasurementSizeMax;
         constexpr std::size_t ncols = Acts::eBoundSize;

         std::bitset<nrows * ncols> proj_bitset {};

         for (unsigned int col_i=0; col_i<N; ++col_i) {
            unsigned int row_i = parameter_map[col_i];
            unsigned int idx = col_i *nrows + row_i;      // @TODO handle row major and column major correctly
            proj_bitset[ (nrows * ncols - 1) - idx ] = 1;
         }
         return proj_bitset.to_ullong();
      }
   };

   // helper to create states without the calibrated measurement, covariance, and sourcelink
   // if more than one state is created the states after the first will share
   // the jacobi and the prediction, if outlier_states is true no storage is created
   // for filtered.
   // @TODO should there be the possibility to have multiple states which mix outlier and non-outlier states ?
   static void createStates(std::size_t n_new_states,
                            const T_BoundState& boundState,
                            std::size_t prevTip,
                            trajectory_t& trajectory,
                            const Acts::BoundSubspaceIndices& subspaceIndices,
                            boost::container::small_vector< typename TrackStateProxy::IndexType, s_maxBranchesPerSurface> &track_states,
                            const Acts::Logger& logger,
                            bool outlier_states) {
      // create track states for all compatible measurement candidates
      const auto &boundParams = derived_t::boundParams(boundState);
      const auto &pathLength = derived_t::pathLength(boundState);

      using PM = Acts::TrackStatePropMask;

      track_states.reserve( n_new_states );
      std::optional<TrackStateProxy> firstTrackState{
         std::nullopt};
      for (unsigned int state_i=0; state_i<n_new_states; ++state_i) {
         PM mask = PM::Predicted | PM::Filtered | PM::Jacobian | PM::Calibrated;

         if (firstTrackState.has_value()) {
            // subsequent track states don't need storage for these as they will
            // be shared
            mask &= ~PM::Predicted & ~PM::Jacobian;
         }

         if (outlier_states) {
            // outlier won't have separate filtered parameters
            mask &= ~PM::Filtered;
         }

         TrackStateProxy trackState = trajectory.makeTrackState(mask, prevTip);
         ACTS_VERBOSE("Create SourceLink output track state #"
                      << trackState.index() << " with mask: " << mask);

         if (firstTrackState.has_value()) {
            trackState.shareFrom(*firstTrackState, PM::Predicted);
            trackState.shareFrom(*firstTrackState, PM::Jacobian);
         }
         else {
            // only set these for first
            trackState.predicted() = boundParams.parameters();
            if (boundParams.covariance()) {
               trackState.predictedCovariance() = *boundParams.covariance();
            }
            trackState.jacobian() = derived_t::boundJacobiMatrix(boundState);
            firstTrackState = trackState;
         }
         trackState.pathLength() = pathLength;

         trackState.setReferenceSurface(boundParams.referenceSurface().getSharedPtr());

         trackState.setProjectorSubspaceIndices(subspaceIndices);

         Acts::TrackStateType typeFlags = trackState.typeFlags();
         if (trackState.referenceSurface().surfaceMaterial() != nullptr) {
            typeFlags.set(Acts::TrackStateFlag::MaterialFlag);
         }
         typeFlags.set(Acts::TrackStateFlag::ParameterFlag);

         // @TODO these track states still need some additional processing. Should there be a special
         //       flag for this ?
         track_states.push_back( trackState.index());
      }
   }


   // utility struct to temporarily store data about the best measurements
   template <std::size_t DIM, typename T_SourceLink>
   struct MatchingMeasurement {
      using MeasCovPair = std::pair < typename MeasurementSelectorTraits<derived_t>::template PreSelectionMeasurement<DIM>,
                                      typename MeasurementSelectorTraits<derived_t>::template PreSelectionMeasurementCovariance<DIM> >;
      MeasCovPair      m_measurement;
      std::optional<T_SourceLink> m_sourceLink;
      float            m_chi2;
      bool             m_isOutLier;
   };

   // type and dimension specific function to select measurements from the range defined by the source link iterators.
   // will iterate over the contiguous measurement range defined by the source link iterators where the measurements
   // are contained in the given container. The selection lopp will get the measurement and covariance with the
   // help of a preCalibrator. select measurements based on smallest chi2 wrt. the prediction, then optionally
   // apply a full calibrator after the selection and finally create track states.
   template <std::size_t DIM, typename T_MeasurementRange>
   Acts::Result<boost::container::small_vector< typename TrackStateProxy::IndexType, s_maxBranchesPerSurface> >
   selectMeasurementsCreateTrackStates(const Acts::GeometryContext& geometryContext,
                                       const Acts::CalibrationContext& calibrationContext,
                                       const Acts::Surface& surface,
                                       const T_BoundState& boundState,
                                       T_MeasurementRange &&measurement_range,
                                       std::size_t prevTip,
                                       trajectory_t& trajectory,
                                       const Acts::Logger& logger,
                                       const std::size_t numMeasurementsCut,
                                       const std::pair<float,float>& maxChi2Cut,
                                       bool forced) const {
      Acts::Result<boost::container::small_vector< typename TrackStateProxy::IndexType, s_maxBranchesPerSurface> >
         result = boost::container::small_vector< typename TrackStateProxy::IndexType, s_maxBranchesPerSurface>{};

      using iterator_t = decltype(measurement_range.begin());
      using container_value_t  = typename MeasurementSelectorTraits<derived_t>::template MeasurementContainerTraits<iterator_t>::value_type;
      using BaseElementType = std::remove_cv_t<std::remove_pointer_t< container_value_t > >;
      // get calibrator
      using TheMatchingMeasurement = MatchingMeasurement<DIM, container_value_t >;
      using MeasCovPair = TheMatchingMeasurement::MeasCovPair;

      using Predicted = typename MeasurementSelectorTraits<derived_t>::template Predicted<DIM>;
      using PredictedCovariance = typename MeasurementSelectorTraits<derived_t>::template PredictedCovariance<DIM>;



      // get prediction in the measurement domain
      Acts::SubspaceIndices<DIM> parameter_map = derived().template parameterMap<DIM>(geometryContext,
                                                                                       calibrationContext,
                                                                                       surface);
      auto predicted
         = std::make_pair( project<DIM, Predicted>(parameter_map,
                                                   derived().boundParams(boundState).parameters()),
                           project<DIM, PredictedCovariance>(parameter_map,
                                                             derived().boundParams(boundState).covariance().value()));

      // select n measurents with the smallest chi2.
      auto preCalibrator = derived().template preCalibrator<DIM, BaseElementType>();
      auto postCalibrator = derived().template postCalibrator<DIM, BaseElementType>();
      TopCollection<NMeasMax, TheMatchingMeasurement > selected_measurements(numMeasurementsCut);
      for ( const auto &measurement : measurement_range ) {
         TheMatchingMeasurement &matching_measurement=selected_measurements.slot();
         if (forced && postCalibrator) {
            // skip preCalibrator and computeChi2, by doing everything else. We start with what's done if no preCalibrator.
            const auto &m = derived().forwardToCalibrator(measurement);
            matching_measurement.m_measurement = std::make_pair( m.template localPosition<DIM>(), m.template localCovariance<DIM>() );
            matching_measurement.m_chi2 = 0.0f;
            matching_measurement.m_sourceLink=measurement;
            if (!selected_measurements.acceptNoSort()) break;
         } else {
            matching_measurement.m_measurement = preCalibrator(geometryContext,
                                                               calibrationContext,
                                                               derived().forwardToCalibrator(measurement),
                                                               derived().boundParams(boundState));
            matching_measurement.m_chi2 = computeChi2(matching_measurement.m_measurement.first,
                                                      matching_measurement.m_measurement.second,
                                                      predicted.first,
                                                      predicted.second);
            // only consider measurements which pass the outlier chi2 cut
            if (matching_measurement.m_chi2<maxChi2Cut.second) {
                matching_measurement.m_sourceLink=measurement;
                selected_measurements.acceptAndSort([](const TheMatchingMeasurement &a,
                                                       const TheMatchingMeasurement &b) {
                   return a.m_chi2 < b.m_chi2;
                });
            }
         }
      }

      // apply final calibration to n-best measurements
      using post_calib_meas_cov_pair_t
         = std::pair<typename MeasurementSelectorTraits<derived_t>::template CalibratedMeasurement<DIM>,
                     typename MeasurementSelectorTraits<derived_t>::template CalibratedMeasurementCovariance<DIM> >;

      // need extra temporary buffer if the types to store "measurements" during measurement selection
      // and calibrated measurements after the measurement selection are not identical
      static constexpr bool pre_and_post_calib_types_agree
         = std::is_same< typename MeasurementSelectorTraits<derived_t>::template CalibratedMeasurement<DIM>,
                         typename MeasurementSelectorTraits<derived_t>::template PreSelectionMeasurement<DIM> >::value
          && std::is_same< typename MeasurementSelectorTraits<derived_t>::template CalibratedMeasurementCovariance<DIM>,
                           typename MeasurementSelectorTraits<derived_t>::template PreSelectionMeasurementCovariance<DIM> >::value;

      using Empty = struct {};
      typename std::conditional< !pre_and_post_calib_types_agree,
                                 std::array< post_calib_meas_cov_pair_t, NMeasMax>, // @TODO could use boost static_vector instead
                                 Empty>::type calibrated;
      if (postCalibrator) {
         typename std::conditional<!pre_and_post_calib_types_agree, unsigned int, Empty>::type calibrated_meas_cov_i{};

         // apply final calibration, recompute chi2
         for (typename TopCollection<NMeasMax, MeasCovPair >::IndexType
                 idx: selected_measurements) {
            TheMatchingMeasurement &a_selected_measurement = selected_measurements.getSlot(idx);

            // helper to select the destination storage which is the extra temporary buffer
            // if the measurement storage types during and after selection are different.
            post_calib_meas_cov_pair_t &calibrated_measurement
               = [&calibrated, &a_selected_measurement, calibrated_meas_cov_i]() -> post_calib_meas_cov_pair_t & {
                  if constexpr(pre_and_post_calib_types_agree) {
                     (void) calibrated;
                     (void) calibrated_meas_cov_i;
                     return a_selected_measurement.m_measurement;
                  }
                  else {
                     (void) a_selected_measurement;
                     assert(calibrated_meas_cov_i < calibrated.size());
                     return calibrated[calibrated_meas_cov_i];
                  }
               }();

            // apply the calibration
            calibrated_measurement = postCalibrator(geometryContext,
                                                    calibrationContext,
                                                    derived().forwardToCalibrator(a_selected_measurement.m_sourceLink.value()),
                                                    derived().boundParams(boundState));
            // update chi2 using calibrated measurement
            a_selected_measurement.m_chi2 = computeChi2(calibrated_measurement.first,
                                                         calibrated_measurement.second,
                                                         predicted.first,
                                                         predicted.second);
            // ... and set outlier flag
            a_selected_measurement.m_isOutLier =  (!forced && a_selected_measurement.m_chi2 >= maxChi2Cut.first);
            if constexpr(!pre_and_post_calib_types_agree) {
               ++calibrated_meas_cov_i;
            }
         }
      }
      else {
         // if no final calibration is performed only the outlier flag still needs to be set
         for (typename TopCollection<NMeasMax, MeasCovPair >::IndexType
                 idx: selected_measurements) {
            TheMatchingMeasurement &a_selected_measurement = selected_measurements.getSlot(idx);
            a_selected_measurement.m_isOutLier =  (!forced && a_selected_measurement.m_chi2 >= maxChi2Cut.first);
         }
      }

      // First Create states without setting information about the calibrated measurement for the selected measurements
      // @TODO first create state then copy measurements, or crete state by state and set measurements ?
      //       the lastter has the "advantage" that the outlier flag can be set individually
      //       the former has the advantage that part of the state creation code is independent of the
      //       the measurement.
      
      Acts::BoundSubspaceIndices boundSubspaceIndices;
      std::copy(parameter_map.begin(), parameter_map.end(), boundSubspaceIndices.begin());
      createStates( selected_measurements.size(),
                    boundState,
                    prevTip,
                    trajectory,
                    boundSubspaceIndices,
                    *result,
                    logger,
                    (!selected_measurements.empty()
                     ? selected_measurements.getSlot( *(selected_measurements.begin())).m_isOutLier
                     : false) );
      assert( result->size() == selected_measurements.size() );

      // helper to determine whether calibrated storeage is to be used
      auto use_calibrated_storage = [&postCalibrator]() -> bool {
         if constexpr(pre_and_post_calib_types_agree) {
            (void) postCalibrator;
            return false;
         }
         else {
            // this is only known during runtime
            // but it should not be tested if the types agree.
            return postCalibrator;
         }
      };

      if (selected_measurements.empty()) {
         auto config_for_surface = m_config.find(surface.geometryId());
         if (config_for_surface != m_config.end()) {
            if (config_for_surface->edgeTolerance.has_value()) {
               // if the  prediction failes the bound test, where the edgeTolerance is expected to be negative
               // then the "hole" is considered to be an edge hole which is not treated as a hole.
               auto local_coords  = derived().boundParams(boundState).parameters().template block<2,1>(0,0);
               if (!surface.insideBounds(local_coords,config_for_surface->edgeTolerance.value())) {
                  result = result.failure(Acts::CombinatorialKalmanFilterError::NoMeasurementExpected);
               }
            }
         }
      }
      else {
      // copy selected measurements to pre-created states
      unsigned int state_i=0;
      for (typename TopCollection<NMeasMax, MeasCovPair >::IndexType
              idx: selected_measurements) {
         assert( state_i < result->size());
         TrackStateProxy trackState( trajectory.getTrackState( (*result)[state_i] ) );
         TheMatchingMeasurement &a_selected_measurement = selected_measurements.getSlot(idx);
         trackState.setUncalibratedSourceLink(derived().makeSourceLink(std::move(a_selected_measurement.m_sourceLink.value())));
         // flag outliers accordingly, so that they are handled correctly by the post processing
         trackState.typeFlags().set( a_selected_measurement.m_isOutLier
                                     ? Acts::TrackStateFlag::OutlierFlag
                                     : Acts::TrackStateFlag::MeasurementFlag );
         trackState.allocateCalibrated(DIM);
         if (use_calibrated_storage()) {
            // if the final clibration is performed after the selection then
            // copy these measurements and covariances to the track states
            assert( use_calibrated_storage() == !pre_and_post_calib_types_agree);
            if constexpr(!pre_and_post_calib_types_agree) {
               assert( state_i < calibrated.size());
               trackState.template calibrated<DIM>()
                  = MeasurementSelectorMatrixTraits::matrixTypeCast<typename MeasurementSelectorTraits<derived_t>::MatrixFloatType>(calibrated[state_i].first);
               trackState.template calibratedCovariance<DIM>()
                  = MeasurementSelectorMatrixTraits::matrixTypeCast<typename MeasurementSelectorTraits<derived_t>::MatrixFloatType>(calibrated[state_i].second);
               trackState.chi2() = a_selected_measurement.m_chi2;
            }
         }
         else {
            trackState.template calibrated<DIM>()
               = MeasurementSelectorMatrixTraits::matrixTypeCast<typename MeasurementSelectorTraits<derived_t>::MatrixFloatType>(a_selected_measurement.m_measurement.first);
            trackState.template calibratedCovariance<DIM>()
               = MeasurementSelectorMatrixTraits::matrixTypeCast<typename MeasurementSelectorTraits<derived_t>::MatrixFloatType>(a_selected_measurement.m_measurement.second);
            trackState.chi2() = a_selected_measurement.m_chi2;
         }
         ++state_i;
      }
      }
      return result;
   }

   template <typename parameters_t>
   static std::size_t getEtaBin(const parameters_t& boundParameters,
                                const std::vector<float> &etaBins) {
      if (etaBins.empty()) {
         return 0u;  // shortcut if no etaBins
      }
      const float eta = std::abs(std::atanh(std::cos(boundParameters.parameters()[Acts::eBoundTheta])));
      std::size_t bin = 0;
      for (auto etaBin : etaBins) {
         if (etaBin >= eta) {
            break;
         }
         bin++;
      }
      return bin;
   }

public:
   // get numMeasurement and maxChi2 cuts for the given surface
   // @TODO should the cuts just depend on the surface or really on the bound parameters (i.e. bound eta)
   std::tuple< std::size_t, std::pair<float,float> > getCuts(const Acts::Surface& surface,
                                                             const T_BoundState& boundState,
                                                             const Acts::Logger& logger) const {
      std::tuple< std::size_t, std::pair<float,float> > result;
      std::size_t &numMeasurementsCut = std::get<0>(result);
      std::pair<float,float> &maxChi2Cut = std::get<1>(result);
      // Get geoID of this surface
      auto geoID = surface.geometryId();
      // Find the appropriate cuts
      auto cuts = m_config.find(geoID);
      if (cuts == m_config.end()) {
         // indicates failure
         numMeasurementsCut = 0;
      }
      else {
         // num measurement Cut
         // getchi2 cut
         std::size_t eta_bin = getEtaBin(derived().boundParams(boundState), cuts->etaBins);
         numMeasurementsCut = (!cuts->numMeasurementsCutOff.empty()
                               ? cuts->numMeasurementsCutOff[ std::min(cuts->numMeasurementsCutOff.size()-1, eta_bin) ]
                               : NMeasMax);
         maxChi2Cut = ! cuts->chi2CutOff.empty()
            ? cuts->chi2CutOff[ std::min(cuts->chi2CutOff.size()-1, eta_bin) ]
            : std::make_pair<float,float>(std::numeric_limits<float>::max(),
                                          std::numeric_limits<float>::max());
         ACTS_VERBOSE("Get cut for eta-bin="
                      << (eta_bin < cuts->etaBins.size() ? cuts->etaBins[eta_bin] : std::numeric_limits<float>::max())
                      << ": chi2 (max,max-outlier) " << maxChi2Cut.first << ", " << maxChi2Cut.second
                      << " max.meas." << numMeasurementsCut);
      }
      return result;
   }
};

// Measurement selector which calls a type specific selection method
// the measurement selector expects a list of measurement containers to be provided
// by the source link iterators as well as the index of the container to be used for the
// given measurement range, which is defined by the source link iterators.
// the measurement range must refer to a contiguous range of measurements contained
// within a single measurement container.
// The "container" itself is a variant of particular measurement containers with associated
// dimension i.e. number of coordinates per measurement. A member funcion specific to one of
// the alternatives of the variant will be called to perform the selection and track state
// creation.
template <std::size_t NMeasMax,
          typename derived_t,
          typename measurement_container_variant_t >
struct MeasurementSelectorWithDispatch : public MeasurementSelectorBase< NMeasMax, MeasurementSelectorTraits<derived_t>::s_dimMax, derived_t> {

   using Base = MeasurementSelectorBase< NMeasMax, MeasurementSelectorTraits<derived_t>::s_dimMax, derived_t>;
   using T_BoundState = typename Base::T_BoundState;
   using trajectory_t = typename Base::trajectory_t;
   using TrackStateProxy = typename Base::TrackStateProxy;
   static constexpr std::size_t s_maxBranchesPerSurface = Base::s_maxBranchesPerSurface;

   // helper to get the maximum number of measurement diemsions.
   static constexpr std::size_t dimMax() {
      return MeasurementSelectorTraits<derived_t>::s_dimMax;
   }

  Acts::Result<boost::container::small_vector< typename TrackStateProxy::IndexType, s_maxBranchesPerSurface> >
  createTrackStates(const Acts::GeometryContext& geometryContext,
                    const Acts::CalibrationContext& calibrationContext,
                    const Acts::Surface& surface,
                    const T_BoundState& boundState,
                    typename TrackStateProxy::IndexType prevTip,
                    [[maybe_unused]] std::vector<TrackStateProxy>& trackStateCandidates,
                    trajectory_t& trajectory,
                    const Acts::Logger& logger) const {

      Acts::Result<boost::container::small_vector< typename TrackStateProxy::IndexType, s_maxBranchesPerSurface> >
         result = Acts::CombinatorialKalmanFilterError::MeasurementSelectionFailed;
      // get associated measurement container and the relevant measurement range for the given surface.
      auto [a_measurement_container_variant_ptr, range, forced] = this->derived().containerAndRange(surface);
      if (!forced && !this->derived().expectMeasurements( surface, a_measurement_container_variant_ptr, range)) {
         result = result.failure(Acts::CombinatorialKalmanFilterError::NoMeasurementExpected);
         return result;
      }
      if (!range.empty()) {
         auto [numMeasurementsCut, maxChi2Cut] = this->getCuts(surface,boundState, logger);
         // numMeasurementsCut is == 0 in case getCuts failed
         // anyway cannot select anything if numMeasurementsCut==0;
         if (numMeasurementsCut>0) {

            result = std::visit( [this,
                                  &geometryContext,
                                  &calibrationContext,
                                  &surface,
                                  &boundState,
                                  &range,
                                  prevTip,
                                  &trajectory,
                                  &logger,
                                  numMeasurementsCut,
                                  &maxChi2Cut,
                                  forced] (const auto &measurement_container_with_dimension) {
               using ArgType = std::remove_cv_t<std::remove_reference_t< decltype(measurement_container_with_dimension) > >;
               constexpr std::size_t DIM = ArgType::dimension();
               auto measurement_range = this->derived().rangeForContainer(measurement_container_with_dimension,range);
               return
                  this->template
                  selectMeasurementsCreateTrackStates<DIM, decltype(measurement_range)>(geometryContext,
                                                           calibrationContext,
                                                           surface,
                                                           boundState,
                                                           std::move(measurement_range),
                                                           prevTip,
                                                           trajectory,
                                                           logger,
                                                           numMeasurementsCut,
                                                           maxChi2Cut,
                                                           forced);
            },
            *a_measurement_container_variant_ptr);
         }
      }
      else {
         result = Acts::Result<boost::container::small_vector< typename TrackStateProxy::IndexType, s_maxBranchesPerSurface> >::success({});
      }
      return result;
   }
};

template <std::size_t NMeasMax,
          typename derived_t,
          typename measurement_container_variant_t>
struct MeasurementSelectorBaseImpl : public MeasurementSelectorWithDispatch<NMeasMax, derived_t, measurement_container_variant_t> {
   using T_BoundState = MeasurementSelectorTraits<derived_t>::BoundState;
   using abstract_measurement_range_t = MeasurementSelectorTraits<derived_t>::abstract_measurement_range_t;

   // get the bound parameters and covariance of the bound state
   static const MeasurementSelectorTraits<derived_t>::BoundTrackParameters &boundParams(const T_BoundState &boundState)  {
      return std::get<0>(boundState);
   }

   // get the jacobi matrix of the bound state
   static const MeasurementSelectorTraits<derived_t>::BoundMatrix &boundJacobiMatrix(const T_BoundState &boundState)  {
      return std::get<1>(boundState);
   }
   // get the accumulated path length of the bound state
   static double pathLength(const T_BoundState &boundState)  {
      return std::get<2>(boundState);
   }

   // create a source link from the measurement
   template <typename T_Value>
   static Acts::SourceLink makeSourceLink(T_Value &&value) {
      return Acts::SourceLink{value};
   }

   // perform simple transformation to cerate the type
   // that is passed to the calibrator.
   // by default pass the measurement by reference not pointer
   template <typename T>
   static const auto &forwardToCalibrator(const T &a) {
      if constexpr( std::is_same<T, std::remove_pointer_t<T> >::value ) {
         return a;
      }
      else {
         return *a;
      }
   }


   // get mapping between bound state parameters and coordinates in the measurement domain
   // in most cases this is just the identity operation e.g. loc0 -> coord0 and loc1 -> coord1
   // i.e. map[0]=0; map[1]=1;
   // @TODO should this be measurement type specific, or is dimension and the surface good enough ?
   template <std::size_t DIM>
   ParameterMapping::type<DIM>
   parameterMap(const Acts::GeometryContext&,
                const Acts::CalibrationContext&,
                const Acts::Surface&) {
      return ParameterMapping::identity<DIM>();
   }

   // By default the methods postCalibrator and preCalibrator return delegates:
   template <std::size_t DIM, typename measurement_t>
   using PreCalibrator = Acts::Delegate<
      std::pair < typename MeasurementSelectorTraits<derived_t>::template PreSelectionMeasurement<DIM>,
                  typename MeasurementSelectorTraits<derived_t>::template PreSelectionMeasurementCovariance<DIM> >
                (const Acts::GeometryContext&,
                 const Acts::CalibrationContext&,
                 const measurement_t &,
                 const typename MeasurementSelectorTraits<derived_t>::BoundTrackParameters &)>;

   // Since the measurement types used during measurement selection and after measurement selection
   // might be different so are the types of the calibrator delegates
   template <std::size_t DIM, typename measurement_t>
   using PostCalibrator = Acts::Delegate<
      std::pair < typename MeasurementSelectorTraits<derived_t>::template CalibratedMeasurement<DIM>,
                  typename MeasurementSelectorTraits<derived_t>::template CalibratedMeasurementCovariance<DIM> >
                (const Acts::GeometryContext&,
                 const Acts::CalibrationContext&,
                 const measurement_t &,
                 const typename MeasurementSelectorTraits<derived_t>::BoundTrackParameters &)>;

   /// the calibrator used after the measurement selection which does not have to be "connected"
   /// @note The return value does not have to be a delegate, it can be an arbitrary functor/lambda,
   /// which however must be castable to bool to test whether there is a postCalibrator
   template <std::size_t DIM, typename measurement_t>
   const PostCalibrator<DIM, measurement_t> &
   postCalibrator() const; // not implemented

   // the "calibrator" which is used during the measuremnt selection
   /// @note The return value does not have to be a delegate, it can be an arbitrary functor/lambda,
   //  this delegate must be connected or be a valid functor. If not this likely will lead to an
   //  exception or seg fault.
   template <std::size_t DIM, typename measurement_t>
   const PreCalibrator<DIM, measurement_t> &
   preCalibrator() const; // not implemented


   /// Get container and index range for measurements on the given surface
   /// @return tuple of container, begin index and end index
   /// @note return a tuple containing a measurement container variant for the given surface and the index range for measurements
   ///       on that surface. The index range may be empty.
   std::tuple<const measurement_container_variant_t &, abstract_measurement_range_t>
   containerAndRange(const Acts::Surface &surface) const; // not implemented

   /// @param surface a surface
   /// @param container_variant_ptr abstract measurement container as returned by @ref containerAndRange
   /// @param abstract_range the range as returned by @ref containerAndRange for the given surface
   /// @return true if measurements are expected, false if the surface is not supposed to have valid measurements.
   bool expectMeasurements([[maybe_unused]] const Acts::Surface &surface,
                           [[maybe_unused]] const measurement_container_variant_t *container_variant_ptr,
                           const abstract_measurement_range_t &abstract_range) const;

   /// Create a range over elements of the given container from an abstract range
   /// @tparam measurement_container_t a concrete container type which is one of one of the possible types of measurement_container_variant_t
   /// @param abstract_range the range as returned by @ref containerAndRange
   /// @return an itrerable range over elements of the given specified by the abstract range.
   template <typename measurement_container_t>
   static auto
   rangeForContainer(const measurement_container_t &concrete_container, const abstract_measurement_range_t &abstract_range);
};
