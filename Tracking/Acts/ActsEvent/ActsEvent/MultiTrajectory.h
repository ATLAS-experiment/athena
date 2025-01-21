/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_MultiTrajectory_h
#define ActsEvent_MultiTrajectory_h
#include <memory>
#include <type_traits>
#include <variant>
#include <string_view>

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/TrackStatePropMask.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/Utilities/HashedString.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "Acts/Surfaces/Surface.hpp"
#include "CxxUtils/concepts.h"
#include "xAODTracking/TrackJacobianAuxContainer.h"
#include "xAODTracking/TrackMeasurementAuxContainer.h"
#include "xAODTracking/TrackParametersAuxContainer.h"
#include "xAODTracking/TrackStateContainer.h" // TODO remove once decorations are fixed
#include "xAODTracking/TrackStateAuxContainer.h"
#include "xAODTracking/TrackSurfaceAuxContainer.h"
#include "xAODTracking/TrackSurfaceContainer.h"
#include "ActsGeometry/ATLASSourceLink.h"
// #define DEBUG_MTJ
#ifdef DEBUG_MTJ
inline std::string_view name_only(const char* s) {
  return std::string_view(s+std::string_view(s).rfind('/'));
}
#define INSPECTCALL(_INFO) {std::cout << name_only(__FILE__) <<":"<<__LINE__<<" "<<__PRETTY_FUNCTION__<<" "<<_INFO<<std::endl; }
#else
#define INSPECTCALL(_INFO)
#endif
#include "ActsEvent/Decoration.h"

namespace ActsTrk {
class MutableMultiTrajectory;
class MultiTrajectory;
class MutableTrackContainerHandlesHelper;
}  // namespace ActsTrk

namespace Acts {
class Surface;
template <typename T>
struct IsReadOnlyMultiTrajectory {};

template <typename T>
struct IsReadOnlyMultiTrajectory<T&> : IsReadOnlyMultiTrajectory<T> {};

template <typename T>
struct IsReadOnlyMultiTrajectory<T&&> : IsReadOnlyMultiTrajectory<T> {};

template <>
struct IsReadOnlyMultiTrajectory<ActsTrk::MultiTrajectory>
    : std::true_type {};

template <>
struct IsReadOnlyMultiTrajectory<ActsTrk::MutableMultiTrajectory>
    : std::false_type {};
}  // namespace Acts

namespace ActsTrk {

using IndexType = std::uint32_t;

using StoredSurface = std::variant<const Acts::Surface*, std::shared_ptr<const Acts::Surface>>;


/**
 * @brief Athena implementation of ACTS::MultiTrajectory (ReadWrite version)
 * The data is stored in 4 external backends. 
 * Backends lifetime are not maintained by this class.
 * except when objects are default constructed (this functionality will be removed). 
 * This class is meant to be used in track finding algorithms (e.g. CKF) and then converted
 * MultiTrajectory variant. These conversion is meant to be costless. 
 */
class MutableMultiTrajectory final
    : public Acts::MultiTrajectory<ActsTrk::MutableMultiTrajectory> {
 public:
  friend ActsTrk::MultiTrajectory;
  using TrackStateProxy = typename Acts::MultiTrajectory<
      ActsTrk::MutableMultiTrajectory>::TrackStateProxy;
  using ConstTrackStateProxy = typename Acts::MultiTrajectory<
      ActsTrk::MutableMultiTrajectory>::ConstTrackStateProxy;

  /**
   * @brief Construct a new Multi Trajectory object owning backends
   */
  MutableMultiTrajectory();
  MutableMultiTrajectory(const ActsTrk::MutableMultiTrajectory& other);
  MutableMultiTrajectory(ActsTrk::MutableMultiTrajectory&& other) = default;
  MutableMultiTrajectory& operator=(const ActsTrk::MutableMultiTrajectory& other) = delete;

  /**
   * @brief Add state with stograge for data that depends on the mask
   *
   * @param mask - bitmask deciding which backends are extended
   * @param istate - previous state
   * @return index of just added state
   */
  ActsTrk::IndexType addTrackState_impl(Acts::TrackStatePropMask mask,
                                        ActsTrk::IndexType iprevious);

  /**
   * @brief Add state components for the given mask
   *
   * @param istate - state index
   * @param mask - bitmask deciding which backends are extended
   */
  void addTrackStateComponents_impl(ActsTrk::IndexType istate,
                                    Acts::TrackStatePropMask mask);

  /**
   * @brief Access component by key
   *
   * @param key
   * @param istate
   * @return std::any - that needs to be cast to a const ptr (non const for the
   * nonconst variant)
   */

  std::any component_impl(Acts::HashedString key, ActsTrk::IndexType istate) const;
  std::any component_impl(Acts::HashedString key, ActsTrk::IndexType istate);

  /**
   * @brief checks if given state has requested component
   *
   * @param key - name (const char*)
   * @param istate - index in the
   * @return true
   * @return false
   */
  bool has_impl(Acts::HashedString key, ActsTrk::IndexType istate) const;

  /**
   * @brief checks if MTJ has requested column (irrespectively of the state)
   *
   * @param key - name (const char*)
   * @return true - if the column is present
   * @return false - if not
   */
  constexpr bool hasColumn_impl(Acts::HashedString key) const;

  /**
   * @brief enables particular decoration, type & name need to be specified
   *
   * @tparam T type of decoration (usually POD)
   * @param key name of the decoration
   */
  template <typename T>
  void addColumn_impl(std::string_view key);

  /**
   * @brief unsets a given state
   *
   * @param target - property
   * @param istate - index in the
   */

  void unset_impl(Acts::TrackStatePropMask target, ActsTrk::IndexType istate);

  /**
   * @brief shares from a given state
   *
   * @param shareSource, shareTarget - property
   * @param iself, iother  - indexes
   */

  void shareFrom_impl(ActsTrk::IndexType iself, ActsTrk::IndexType iother,
                      Acts::TrackStatePropMask shareSource,
                      Acts::TrackStatePropMask shareTarget);

  /**
   * @brief obtains proxy to the track state under given index
   *
   * @param index
   * @return TrackStateProxy::Parameters
   */

  typename ConstTrackStateProxy::Parameters parameters_impl(
      ActsTrk::IndexType index) const {
    return typename ConstTrackStateProxy::Parameters{m_trackParametersAux->params[index].data()};
  }

  typename TrackStateProxy::Parameters parameters_impl(ActsTrk::IndexType index) {
    return typename TrackStateProxy::Parameters{m_trackParametersAux->params[index].data()};
  }

  /**
   * @brief obtain covariances for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Covariance
   */
  typename ConstTrackStateProxy::Covariance covariance_impl(
      ActsTrk::IndexType index) const {
    return typename ConstTrackStateProxy::Covariance{m_trackParametersAux->covMatrix[index].data()};
  }
  typename TrackStateProxy::Covariance covariance_impl(ActsTrk::IndexType index) {
    return typename TrackStateProxy::Covariance{m_trackParametersAux->covMatrix[index].data()};    
  }

  /**
   * @brief obtain measurement covariances for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Covariance
   */
  typename ConstTrackStateProxy::Covariance trackMeasurementsCov(
      ActsTrk::IndexType index) const {
    return typename ConstTrackStateProxy::Covariance{m_trackMeasurementsAux->covMatrix[index].data()};

  }

  typename TrackStateProxy::Covariance trackMeasurementsCov(ActsTrk::IndexType index) {
    return typename TrackStateProxy::Covariance{m_trackMeasurementsAux->covMatrix[index].data()};

  }

  /**
   * @brief obtain jacobian for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Covariance
   */

  inline typename ConstTrackStateProxy::Covariance jacobian_impl(
      ActsTrk::IndexType istate) const {
    xAOD::TrackStateIndexType jacIdx = m_trackStatesAux->jacobian[istate];
    return typename ConstTrackStateProxy::Covariance{m_trackJacobiansAux->jac[jacIdx].data()};
  }

  typename TrackStateProxy::Covariance jacobian_impl(ActsTrk::IndexType istate) {
    xAOD::TrackStateIndexType jacIdx = m_trackStatesAux->jacobian[istate];
    return typename TrackStateProxy::Covariance{m_trackJacobiansAux->jac[jacIdx].data()};
  }

  /**
   * @brief obtain calibrated measurements for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Calibrated
   */

  template <std::size_t measdim>
  inline typename ConstTrackStateProxy::template Calibrated<measdim>
  calibrated_impl(ActsTrk::IndexType index) const {
    xAOD::TrackStateIndexType measIdx = m_trackStatesAux->calibrated[index];
    return typename ConstTrackStateProxy::template Calibrated<measdim>{m_trackMeasurementsAux->meas[measIdx].data()};
  }

  template <std::size_t measdim, bool Enable = true>
  std::enable_if_t<Enable,
                   typename TrackStateProxy::template Calibrated<measdim>>
  calibrated_impl(ActsTrk::IndexType index) {
    xAOD::TrackStateIndexType measIdx = m_trackStatesAux->calibrated[index];
    return typename TrackStateProxy::template Calibrated<measdim>{m_trackMeasurementsAux->meas[measIdx].data()};
  }

  /**
   * @brief obtain measurements covariance for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Covariance
   */

  template <std::size_t measdim>
  inline typename ConstTrackStateProxy::template CalibratedCovariance<measdim>
  calibratedCovariance_impl(ActsTrk::IndexType index) const {
    xAOD::TrackStateIndexType measIdx = m_trackStatesAux->calibrated[index];
    return ConstTrackStateProxy::template CalibratedCovariance<measdim>{m_trackMeasurementsAux->covMatrix[measIdx].data()};
  }
  template <std::size_t measdim, bool Enable = true>
  std::enable_if_t<
      Enable, typename TrackStateProxy::template CalibratedCovariance<measdim>>
  calibratedCovariance_impl(ActsTrk::IndexType index) {
    xAOD::TrackStateIndexType measIdx = m_trackStatesAux->calibrated[index];
    return TrackStateProxy::template CalibratedCovariance<measdim>{m_trackMeasurementsAux->covMatrix[measIdx].data()};
  }

  /**
   * @brief size of the MTJ
   *
   * @return size_t
   */

  inline Acts::TrackIndexType size_impl() const { 
    return m_trackStatesSize;
  }

  /**
   * @brief clears backends
   * decoration columns are still declared
   */
  void clear_impl();

  /**
   * @brief checks if the backends are connected (i.e. is safe to use, else any
   * other call will cause segfaults)
   */
  bool has_backends() const;

  /**
   * Implementation of allocation of calibrated measurements
   */
  template <typename val_t, typename cov_t>
  void allocateCalibrated_impl(IndexType istate,
                               const Eigen::DenseBase<val_t>& val,
                               const Eigen::DenseBase<cov_t>& cov)
    requires(Eigen::PlainObjectBase<val_t>::RowsAtCompileTime > 0 &&
             Eigen::PlainObjectBase<val_t>::RowsAtCompileTime <= Acts::eBoundSize &&
             Eigen::PlainObjectBase<val_t>::RowsAtCompileTime ==
                 Eigen::PlainObjectBase<cov_t>::RowsAtCompileTime &&
             Eigen::PlainObjectBase<cov_t>::RowsAtCompileTime ==
                 Eigen::PlainObjectBase<cov_t>::ColsAtCompileTime);

  /**
   * Implementation of calibrated size
   */
  ActsTrk::IndexType calibratedSize_impl(ActsTrk::IndexType istate) const;

  /**
   * Implementation of uncalibrated link insertion
   */
  void setUncalibratedSourceLink_impl(ActsTrk::IndexType istate,
                                      const Acts::SourceLink& sourceLink);

  /**
   * Implementation of uncalibrated link fetch
   */
  typename Acts::SourceLink getUncalibratedSourceLink_impl(
      ActsTrk::IndexType istate) const;

  void setReferenceSurface_impl(IndexType,
                                std::shared_ptr<const Acts::Surface>);
  const Acts::Surface* referenceSurface_impl(IndexType ) const;

  /**
   * @brief copy dynamic data from another MTJ
   * @param istate - index of the track to be filled
   * @param key - key of the dynamic data
   * @param src_ptr - pointer to the source data
   * @warning the type fetched from src and type of the destiantion decoration need to agree
   */
  void copyDynamicFrom_impl (ActsTrk::IndexType istate,
                             Acts::HashedString key,
                             const std::any& src_ptr);
  /**
   * @brief returns the keys of all the dynamic columns
   */
  std::vector<Acts::HashedString> dynamicKeys_impl() const;

  // access to some backends (for debugging purposes)
  // the receiver should not assume ownership or similar
  inline xAOD::TrackStateAuxContainer* trackStatesAux() { return m_trackStatesAux.get(); }

  inline const xAOD::TrackParametersAuxContainer* trackParametersAux() const {
    return m_trackParametersAux.get();
  }
  inline xAOD::TrackParametersAuxContainer* trackParametersAux() {
    return m_trackParametersAux.get();
  }

  inline xAOD::TrackJacobianAuxContainer* trackJacobiansAux() {
    return m_trackJacobiansAux.get();
  }

  inline xAOD::TrackMeasurementAuxContainer* trackMeasurementsAux() {
    return m_trackMeasurementsAux.get();
  }

  inline xAOD::TrackSurfaceAuxContainer* trackSurfacesAux() {
    return m_surfacesBackendAux.get();
  }

  static const std::set<std::string> s_staticVariables;

  friend ActsTrk::MutableTrackContainerHandlesHelper;

 private:

  std::unique_ptr<xAOD::TrackStateAuxContainer> m_trackStatesAux;
  size_t m_trackStatesSize = 0;

  std::unique_ptr<xAOD::TrackParametersAuxContainer> m_trackParametersAux;
  size_t m_trackParametersSize = 0;

  std::unique_ptr<xAOD::TrackJacobianAuxContainer> m_trackJacobiansAux;
  size_t m_trackJacobiansSize = 0;

  std::unique_ptr<xAOD::TrackMeasurementAuxContainer> m_trackMeasurementsAux;
  size_t m_trackMeasurementsSize = 0;

  std::unique_ptr<xAOD::TrackSurfaceContainer> m_surfacesBackend;
  std::unique_ptr<xAOD::TrackSurfaceAuxContainer> m_surfacesBackendAux;
  std::vector<ActsTrk::detail::Decoration> m_decorations;

  std::vector<std::optional<Acts::SourceLink>> m_calibratedSourceLinks;
  std::vector<std::optional<Acts::SourceLink>> m_uncalibratedSourceLinks;

  std::vector<StoredSurface> m_surfaces;
  ActsGeometryContext m_geoContext;

  // adjust preallocated size to actually used
  void trim();
};

/**
 * Read only version of MTJ
 * The implementation is separate as the details are significantly different 
 * and in addition only const methods are ever needed
 */
class MultiTrajectory
    : public Acts::MultiTrajectory<MultiTrajectory> {
 public:

  MultiTrajectory(
      DataLink<xAOD::TrackStateAuxContainer> trackStates,
      DataLink<xAOD::TrackParametersAuxContainer> trackParameters,
      DataLink<xAOD::TrackJacobianAuxContainer> trackJacobians,
      DataLink<xAOD::TrackMeasurementAuxContainer> trackMeasurements, 
      DataLink<xAOD::TrackSurfaceAuxContainer> trackSurfaces);

  bool has_impl(Acts::HashedString key, ActsTrk::IndexType istate) const;

  std::any component_impl(Acts::HashedString key, ActsTrk::IndexType istate) const;

  bool hasColumn_impl(Acts::HashedString key) const;

  typename ConstTrackStateProxy::Parameters parameters_impl(
      ActsTrk::IndexType index) const {
    return typename ConstTrackStateProxy::Parameters{m_trackParametersAux->params[index].data()};
  }

  typename ConstTrackStateProxy::Covariance covariance_impl(
      ActsTrk::IndexType index) const {
    return  typename ConstTrackStateProxy::Covariance{m_trackParametersAux->covMatrix[index].data()};
  }

  inline typename ConstTrackStateProxy::Covariance jacobian_impl(
      ActsTrk::IndexType istate) const {
    xAOD::TrackStateIndexType jacIdx = m_trackStatesAux->jacobian[istate];
    return typename ConstTrackStateProxy::Covariance{m_trackJacobiansAux->jac[jacIdx].data()};
  }

  template <std::size_t measdim>
  inline typename ConstTrackStateProxy::template Calibrated<measdim>
  calibrated_impl(IndexType istate) const {
    xAOD::TrackStateIndexType measIdx = m_trackStatesAux->calibrated[istate];
    return typename ConstTrackStateProxy::template Calibrated<measdim>{m_trackMeasurementsAux->meas[measIdx].data()};
  }

  template <std::size_t measdim>
  inline typename ConstTrackStateProxy::template CalibratedCovariance<measdim>
  calibratedCovariance_impl(IndexType index) const {
    xAOD::TrackStateIndexType measIdx = m_trackStatesAux->calibrated[index];
    return typename ConstTrackStateProxy::template CalibratedCovariance<measdim>{m_trackMeasurementsAux->covMatrix[measIdx].data()};
  }
  inline Acts::TrackIndexType size_impl() const { return m_trackStatesAux->size(); }

  ActsTrk::IndexType calibratedSize_impl(ActsTrk::IndexType istate) const;
  typename Acts::SourceLink getUncalibratedSourceLink_impl(ActsTrk::IndexType istate) const;

  const Acts::Surface* referenceSurface_impl(IndexType) const;

  /**
   * Fill surfaces either from persistency or from geometry
   * If the surfaces are already there it means that the container is trainsient and this is void operation
   */
  void fillSurfaces(const Acts::TrackingGeometry* geo, const Acts::GeometryContext& geoContext );
  /**
   * reuse surfaces from MutableMultiTrajectory
   */
  void moveSurfaces(const ActsTrk::MutableMultiTrajectory* mtj);

  void moveLinks(const ActsTrk::MutableMultiTrajectory* mtj);

  std::vector<Acts::HashedString> dynamicKeys_impl() const {
      // @TODO: This currently does not do anything useful
      return {};
  }

 private:
  const DataLink<xAOD::TrackStateAuxContainer> m_trackStatesAux;
  const DataLink<xAOD::TrackParametersAuxContainer> m_trackParametersAux;
  const DataLink<xAOD::TrackJacobianAuxContainer> m_trackJacobiansAux;
  const DataLink<xAOD::TrackMeasurementAuxContainer> m_trackMeasurementsAux;
  const DataLink<xAOD::TrackSurfaceAuxContainer> m_trackSurfacesAux;
  std::vector<ActsTrk::detail::Decoration> m_decorations;

  // // TODO remove once tracking code switches to sourceLinks with EL
  std::vector<std::optional<Acts::SourceLink>> m_calibratedSourceLinks;
  // still need this to store SourceLinks with other payloads than
  // pointer to xAOD::UncalibratedMeasurement e.g. pointer to Trk::Measurements
  // when converting Trk::Tracks to Acts tracks.
  std::vector<std::optional<Acts::SourceLink>> m_uncalibratedSourceLinks;

  std::vector<StoredSurface> m_surfaces;
};


/**
* @brief helper to construct interface container for already filled Aux container
* TODO maybe should be moved to xAOD area
*/
template<typename IFACE, typename AUX>
std::unique_ptr<IFACE> makeInterfaceContainer(const AUX* aux) {
  auto interface = std::make_unique<IFACE>();
  for ( size_t i = 0, sz = aux->size(); i < sz; ++i) {
    interface->emplace_back( new std::remove_pointer_t<typename IFACE::value_type>() );
  }
  interface->setStore(aux);
  return interface;
}


}  // namespace ActsTrk

#include "MultiTrajectory.icc"


#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::MultiTrajectory , 51219308 , 1 )

// These two lines shouldn't be here, but necessary until we have a proper
// solution
#include "Acts/EventData/VectorTrackContainer.hpp"
CLASS_DEF(Acts::ConstVectorTrackContainer, 1074811884, 1)

#endif
