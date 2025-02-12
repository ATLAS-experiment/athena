/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/SurfaceEncoding.h"
#include "xAODTracking/TrackMeasurementAuxContainer.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODTracking/TrackState.h"
#include "xAODTracking/TrackParameters.h"
#include "xAODTracking/TrackJacobian.h"

constexpr uint64_t InvalidGeoID = std::numeric_limits<uint64_t>::max();

const std::set<std::string> ActsTrk::MutableMultiTrajectory::s_staticVariables = {
  "chi2", "pathLength", "typeFlags", "previous", "next", "predicted", "filtered", "smoothed", "jacobian", "calibrated", "measDim",
  "uncalibratedMeasurement", "uncalibratedMeasurementLink" /*created by converter*/, "geometryId", "surfaceIndex"
 };

namespace {
   // utility functions facilitate access of dynamic (or static) aux variables
   template <typename T>
   using const_span = std::span<T const>;

   // get an aux variable for elements (read/write)
   template <typename T>
   std::span<T> getElementVector( xAOD::AuxContainerBase &aux_container, const SG::ConstAccessor<T> &accessor) {
      const T *data = static_cast<const T *>(aux_container.getData (accessor.auxid()));
      T *non_const_data = const_cast< T *>(data); // aux_container is non-const so can be modified.
      std::span<T> ret(non_const_data, aux_container.size() );
      return ret;
   }
   // get an aux variable for elements (read only)
   template <typename T>
   const_span<T> getElementVector(const xAOD::AuxContainerBase &aux_container, const SG::ConstAccessor<T> &accessor) {
      const T *data = static_cast<const T *>(aux_container.getData (accessor.auxid()));
      const_span<T> ret( data, aux_container.size() );
      return ret;
   }

   // create a new decoration for all elements using the default value
   template <typename T>
   std::span<T> createDecoration(xAOD::AuxContainerBase &aux_container, const SG::Decorator<T> &decor) {
      std::size_t sz=aux_container.size();
      T *data = static_cast<T *>(aux_container.getDecoration(decor.auxid(), sz, sz));
      return std::span<T>( data, sz );
   }

   void throwRangeError(const xAOD::AuxContainerBase &aux_container, std::size_t index) {
      std::stringstream msg;
      msg << "range error: " << index << " >= " << aux_container.size()
          << " when accessing container of type " << typeid(xAOD::AuxContainerBase).name();
      throw std::range_error(msg.str());
   }

   // get one an aux variable (dynamic or static) for one element
   // @throw will throw if the index exceeds the container size
   template <typename T>
   const T &getElement(const xAOD::AuxContainerBase &aux_container, const SG::ConstAccessor<T> &accessor, std::size_t index) {
      if (index>= aux_container.size()) {
         throwRangeError(aux_container, index);
      }
      const_span<const xAOD::UncalibratedMeasurement*> vec = getElementVector(aux_container, accessor);
      return vec[index];
   }

constexpr std::optional<bool> has_impl(
    const xAOD::TrackStateAuxContainer* trackStates, Acts::HashedString key,
    ActsTrk::IndexType istate) {
  using namespace Acts::HashedStringLiteral;
  using Acts::MultiTrajectoryTraits::kInvalid;
  INSPECTCALL(key << " " << istate);

  switch (key) {
    case "previous"_hash:
      return trackStates->previous[istate] < kInvalid;
    case "chi2"_hash:{
      INSPECTCALL(key << " " << istate << " chi2");
      return true;
    }
    case "pathLength"_hash:{
      INSPECTCALL(key << " " << istate << " pathLength");
      return true;
    }
    case "typeFlags"_hash:{
      INSPECTCALL(key << " " << istate << " type flags");
      return true;
    }
    case "predicted"_hash:{
      INSPECTCALL(key << " " << istate << " predicted");
      return trackStates->predicted[istate] < kInvalid;
    }
    case "filtered"_hash:{
      INSPECTCALL(key << " " << istate << " filtered");
      return trackStates->filtered[istate] < kInvalid;
    }
    case "smoothed"_hash:{
      INSPECTCALL(key << " " << istate << " smoothed");
      return trackStates->smoothed[istate] < kInvalid;
    }
    case "jacobian"_hash:{
      INSPECTCALL(key << " " << istate << " jacobian");
      return trackStates->jacobian[istate] < kInvalid;
    }
    case "projector"_hash:{
      INSPECTCALL(key << " " << istate << " projector");
      return trackStates->calibrated[istate] < kInvalid;
    }
    case "calibrated"_hash:{
      INSPECTCALL(key << " " << istate << " calibrated");
      return trackStates->measDim[istate] < kInvalid;
    }
    case "calibratedCov"_hash: {
      INSPECTCALL(key << " " << istate << " calibratedCov");
      return trackStates->measDim[istate] < kInvalid;
    }
    case "measdim"_hash: {
      INSPECTCALL(key << " " << istate << " measdim");
      return true;
    }
    case "referenceSurface"_hash: {
      INSPECTCALL(key << " " << istate << " referenceSurfaceEnco");
      return true;
    }

      // TODO restore once only the EL Source Links are in use 
      // return !trackStates[istate]->uncalibratedMeasurementLink().isDefault();
  }
  INSPECTCALL(key << " " << istate << " not a predefined component");
  return std::optional<bool>();
}
}


template<typename T>
const T* to_const_ptr(const std::unique_ptr<T>& ptr) {
  return ptr.get();
}

ActsTrk::MutableMultiTrajectory::MutableMultiTrajectory()
  : m_trackStatesAux (std::make_unique<xAOD::TrackStateAuxContainer>()),
    m_trackParametersAux (std::make_unique<xAOD::TrackParametersAuxContainer>()),
    m_trackJacobiansAux (std::make_unique<xAOD::TrackJacobianAuxContainer>()),
    m_trackMeasurementsAux (std::make_unique<xAOD::TrackMeasurementAuxContainer>()),
    m_surfacesBackend (std::make_unique<xAOD::TrackSurfaceContainer>()),
    m_surfacesBackendAux (std::make_unique<xAOD::TrackSurfaceAuxContainer>())
{
  INSPECTCALL("c-tor " << this)

  m_surfacesBackend->setStore(m_surfacesBackendAux.get());

}

ActsTrk::MutableMultiTrajectory::MutableMultiTrajectory(const ActsTrk::MutableMultiTrajectory& other) 
  : MutableMultiTrajectory()
  {
  INSPECTCALL("copy c-tor " << this <<  " src " << &other << " " << other.size())

  *m_trackStatesAux.get() = *other.m_trackStatesAux.get();
  *m_trackParametersAux.get() = *other.m_trackParametersAux.get();
  *m_trackJacobiansAux.get() = *other.m_trackJacobiansAux.get();
  *m_trackMeasurementsAux.get() = *other.m_trackMeasurementsAux.get();
  m_decorations = other.m_decorations;
  m_calibratedSourceLinks = other.m_calibratedSourceLinks;
  m_uncalibratedSourceLinks = other.m_uncalibratedSourceLinks;

  m_surfaces = other.m_surfaces;
  m_geoContext = other.m_geoContext;
  INSPECTCALL("copy c-tor  done")
}


bool ActsTrk::MutableMultiTrajectory::has_backends() const {
  return m_trackStatesAux != nullptr and m_trackParametersAux != nullptr and
         m_trackJacobiansAux != nullptr and m_trackMeasurementsAux != nullptr
         and m_surfacesBackend != nullptr;
}

namespace{
  template<typename CONT>
  void stepResize( CONT* auxPtr, const size_t realSize, const size_t sizeStep = 20) {
    if( realSize >= auxPtr->size() )  auxPtr->resize(auxPtr->size()+sizeStep);
  }
}

ActsTrk::IndexType ActsTrk::MutableMultiTrajectory::addTrackState_impl(
    Acts::TrackStatePropMask mask,
    ActsTrk::IndexType previous) {
  using namespace Acts::HashedStringLiteral;
  INSPECTCALL( this << " " <<  mask << " " << m_trackStatesAux->size() << " " << previous);
  assert(m_trackStatesAux && "Missing Track States backend");
  stepResize(m_trackStatesAux.get(), m_trackStatesSize);
  m_surfaces.push_back(nullptr);

  // set kInvalid
  using Acts::MultiTrajectoryTraits::kInvalid;

  if (previous >= kInvalid - 1)
    previous = kInvalid;  // fix needed in Acts::MTJ
  m_trackStatesAux->previous[m_trackStatesSize] = previous;
  using namespace Acts;

  auto addParam = [this]() -> ActsTrk::IndexType {
    stepResize(m_trackParametersAux.get(), m_trackParametersSize, 60);
    // TODO ask AK if this resize could be method of aux container
    m_trackParametersAux->params[m_trackParametersSize].resize(Acts::eBoundSize);
    m_trackParametersAux->covMatrix[m_trackParametersSize].resize(Acts::eBoundSize*Acts::eBoundSize);
    m_trackParametersSize++;
    return m_trackParametersSize-1;
  };

  auto addJacobian = [this]() -> ActsTrk::IndexType {
    stepResize(m_trackJacobiansAux.get(), m_trackJacobiansSize);
    m_trackJacobiansAux->jac[m_trackJacobiansSize].resize(Acts::eBoundSize*Acts::eBoundSize);
    m_trackJacobiansSize++;
    return m_trackJacobiansSize-1;
  };

  auto addMeasurement = [this]() -> ActsTrk::IndexType {
    stepResize(m_trackMeasurementsAux.get(), m_trackMeasurementsSize );
    m_trackMeasurementsAux->meas[m_trackMeasurementsSize].resize(Acts::eBoundSize);
    m_trackMeasurementsAux->covMatrix[m_trackMeasurementsSize].resize(Acts::eBoundSize*Acts::eBoundSize);
    m_trackMeasurementsSize++;
    return m_trackMeasurementsSize-1;
  };

  m_trackStatesAux->predicted[m_trackStatesSize] = kInvalid;
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Predicted)) {
    m_trackStatesAux->predicted[m_trackStatesSize] = addParam();
  }

  m_trackStatesAux->filtered[m_trackStatesSize] = kInvalid;
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Filtered)) {
    m_trackStatesAux->filtered[m_trackStatesSize] = addParam();
  }

  m_trackStatesAux->smoothed[m_trackStatesSize] = kInvalid;
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Smoothed)) {
    m_trackStatesAux->smoothed[m_trackStatesSize] = addParam();
  }

  m_trackStatesAux->jacobian[m_trackStatesSize] = kInvalid;
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Jacobian)) {
    m_trackStatesAux->jacobian[m_trackStatesSize] = addJacobian();
  }

  m_uncalibratedSourceLinks.emplace_back(std::nullopt);
  m_trackStatesAux->calibrated[m_trackStatesSize] = kInvalid;
  m_trackStatesAux->measDim[m_trackStatesSize] = kInvalid;
  // @TODO uncalibrated ?
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Calibrated)) {
    m_trackStatesAux->calibrated.at(m_trackStatesSize)  = addMeasurement();
    m_calibratedSourceLinks.emplace_back(std::nullopt);
  }

  m_trackStatesAux->geometryId[m_trackStatesSize] = InvalidGeoID; // surface is invalid until set
  m_trackStatesSize++;
  return m_trackStatesSize-1;
}


void ActsTrk::MutableMultiTrajectory::addTrackStateComponents_impl(
    ActsTrk::IndexType istate,
    Acts::TrackStatePropMask mask) {
  using namespace Acts::HashedStringLiteral;
  INSPECTCALL( this << " " <<  mask << " " << m_trackStatesAux->size() << " " << previous);

  assert(m_trackStatesAux && "Missing Track States backend");

  // set kInvalid
  using Acts::MultiTrajectoryTraits::kInvalid;

  using namespace Acts;

  auto addParam = [this]() -> ActsTrk::IndexType {
    stepResize(m_trackParametersAux.get(), m_trackParametersSize, 60);
    // TODO ask AK if this resize could be method of aux container
    m_trackParametersAux->params[m_trackParametersSize].resize(Acts::eBoundSize);
    m_trackParametersAux->covMatrix[m_trackParametersSize].resize(Acts::eBoundSize*Acts::eBoundSize);
    m_trackParametersSize++;
    return m_trackParametersSize-1;
  };

  auto addJacobian = [this]() -> ActsTrk::IndexType {
    stepResize(m_trackJacobiansAux.get(), m_trackJacobiansSize);
    m_trackJacobiansAux->jac[m_trackJacobiansSize].resize(Acts::eBoundSize*Acts::eBoundSize);
    m_trackJacobiansSize++;
    return m_trackJacobiansSize-1;
  };

  auto addMeasurement = [this]() -> ActsTrk::IndexType {
    stepResize(m_trackMeasurementsAux.get(), m_trackMeasurementsSize );
    m_trackMeasurementsAux->meas[m_trackMeasurementsSize].resize(Acts::eBoundSize);
    m_trackMeasurementsAux->covMatrix[m_trackMeasurementsSize].resize(Acts::eBoundSize*Acts::eBoundSize);
    m_trackMeasurementsSize++;
    return m_trackMeasurementsSize-1;
  };

  if ((m_trackStatesAux->predicted[istate] == kInvalid) &&
      ACTS_CHECK_BIT(mask, TrackStatePropMask::Predicted)) {
    m_trackStatesAux->predicted[istate] = addParam();
  }

  if ((m_trackStatesAux->filtered[istate] == kInvalid) &&
      ACTS_CHECK_BIT(mask, TrackStatePropMask::Filtered)) {
    m_trackStatesAux->filtered[istate] = addParam();
  }

  if ((m_trackStatesAux->smoothed[istate] == kInvalid) &&
      ACTS_CHECK_BIT(mask, TrackStatePropMask::Smoothed)) {
    m_trackStatesAux->smoothed[istate] = addParam();
  }

  if ((m_trackStatesAux->jacobian[istate] == kInvalid) &&
      ACTS_CHECK_BIT(mask, TrackStatePropMask::Jacobian)) {
    m_trackStatesAux->jacobian[istate] = addJacobian();
  }

  if ((m_trackStatesAux->calibrated[istate] == kInvalid) &&
      ACTS_CHECK_BIT(mask, TrackStatePropMask::Calibrated)) {
    m_trackStatesAux->calibrated[istate]  = addMeasurement();
    m_calibratedSourceLinks.emplace_back(std::nullopt);
  }
}


void ActsTrk::MutableMultiTrajectory::shareFrom_impl(
    ActsTrk::IndexType iself,
    ActsTrk::IndexType iother,
    Acts::TrackStatePropMask shareSource,
    Acts::TrackStatePropMask shareTarget) {

  assert(ACTS_CHECK_BIT(this->getTrackState(iother).getMask(), shareSource) &&
         "Source has incompatible allocation");

  using PM = Acts::TrackStatePropMask;
  // set kInvalid
  using Acts::MultiTrajectoryTraits::kInvalid;

  ActsTrk::IndexType sourceIndex{kInvalid};
  switch (shareSource) {
    case PM::Predicted:
      sourceIndex = m_trackStatesAux->predicted[iother];
      break;
    case PM::Filtered:
      sourceIndex = m_trackStatesAux->filtered[iother];
      break;
    case PM::Smoothed:
      sourceIndex = m_trackStatesAux->smoothed[iother];
      break;
    case PM::Jacobian:
      sourceIndex = m_trackStatesAux->jacobian[iother];
      break;
    default:
      throw std::domain_error{"MutableMultiTrajectory Unable to share this component"};
  }

  assert(sourceIndex != kInvalid);

  switch (shareTarget) {
    case PM::Predicted:
      assert(shareSource != PM::Jacobian);
      m_trackStatesAux->predicted[iself] = sourceIndex;
      break;
    case PM::Filtered:
      assert(shareSource != PM::Jacobian);
      m_trackStatesAux->filtered[iself] = sourceIndex;
      break;
    case PM::Smoothed:
      assert(shareSource != PM::Jacobian);
      m_trackStatesAux->smoothed[iself] = sourceIndex;
      break;
    case PM::Jacobian:
      assert(shareSource == PM::Jacobian);
      m_trackStatesAux->jacobian[iself] = sourceIndex;
      break;
    default:
      throw std::domain_error{"MutableMultiTrajectory Unable to share this component"};
  }
}


void ActsTrk::MutableMultiTrajectory::unset_impl(
    Acts::TrackStatePropMask target,
    ActsTrk::IndexType istate) {

  using PM = Acts::TrackStatePropMask;
  // set kInvalid
  using Acts::MultiTrajectoryTraits::kInvalid;

  switch (target) {
    case PM::Predicted:
      m_trackStatesAux->predicted[istate] = kInvalid;
      break;
    case PM::Filtered:
      m_trackStatesAux->filtered[istate] = kInvalid;

      break;
    case PM::Smoothed:
      m_trackStatesAux->smoothed[istate] = kInvalid;
      break;
    case PM::Jacobian:
      m_trackStatesAux->jacobian[istate] = kInvalid;

      break;
    case PM::Calibrated:
      m_trackStatesAux->measDim[istate] = kInvalid;
      // TODO here m_measOffset[istate] and m_measCovOffset[istate] should be
      // set to kInvalid

      break;
    default:
      throw std::domain_error{"MutableMultiTrajectory Unable to unset this component"};
  }
}

std::any ActsTrk::MutableMultiTrajectory::component_impl(
    Acts::HashedString key, ActsTrk::IndexType istate) {
  using namespace Acts::HashedStringLiteral;
  assert(istate < m_trackStatesAux->size() &&
         "Attempt to reach beyond the Track States container size");
  INSPECTCALL(key << " " << istate << " non-const component_impl")

  switch (key) {
    case "previous"_hash:
      return &(m_trackStatesAux->previous[istate]);
    case "next"_hash:
      return &(m_trackStatesAux->next[istate]);
    case "chi2"_hash:
      return &(m_trackStatesAux->chi2[istate]);
    case "pathLength"_hash:
      return &(m_trackStatesAux->pathLength[istate]);
    case "predicted"_hash:
      return &(m_trackStatesAux->predicted[istate]);
    case "filtered"_hash:
      return &(m_trackStatesAux->filtered[istate]);
    case "smoothed"_hash:
      return &(m_trackStatesAux->smoothed[istate]);
    case "projector"_hash: {
      auto idx = m_trackStatesAux->calibrated[istate];
      return &(m_trackMeasurementsAux->projector[idx]);
    }    
    case "measdim"_hash:
      return &(m_trackStatesAux->measDim[istate]);      
    case "typeFlags"_hash:
      return &(m_trackStatesAux->typeFlags[istate]);

    default: {
      for (auto& d : m_decorations) {
        if (d.hash == key) {
          return d.setter(m_trackStatesAux.get(), istate, d.auxid);
        }
      }
      throw std::runtime_error("MutableMultiTrajectory::component_impl no such component " + std::to_string(key));
    }
  }
}


std::any ActsTrk::MutableMultiTrajectory::component_impl(
    Acts::HashedString key,
    ActsTrk::IndexType istate) const {
  using namespace Acts::HashedStringLiteral;
  assert(istate < m_trackStatesAux->size() &&
         "Attempt to reach beyond the Track States container size");
  INSPECTCALL(key << " " << istate << " const component_impl")
  switch (key) {
    case "previous"_hash:
      return &(to_const_ptr(m_trackStatesAux)->previous[istate]);
    case "next"_hash:
      return &(to_const_ptr(m_trackStatesAux)->next[istate]);
    case "chi2"_hash:
      return &(to_const_ptr(m_trackStatesAux)->chi2[istate]);
    case "pathLength"_hash:
      return &(to_const_ptr(m_trackStatesAux)->pathLength[istate]);
    case "predicted"_hash:
      return &(to_const_ptr(m_trackStatesAux)->predicted[istate]);
    case "filtered"_hash:
      return &(to_const_ptr(m_trackStatesAux)->filtered[istate]);
    case "smoothed"_hash:
      return &(to_const_ptr(m_trackStatesAux)->smoothed[istate]);
    case "jacobian"_hash:
      return &(to_const_ptr(m_trackStatesAux)->jacobian[istate]);
    case "projector"_hash:{
      auto idx = to_const_ptr(m_trackStatesAux)->calibrated[istate];
      return &(to_const_ptr(m_trackMeasurementsAux)->projector[idx]);
    }
    case "calibrated"_hash: {
      return &(to_const_ptr(m_trackStatesAux)->calibrated[istate]);
    }
    case "calibratedCov"_hash: {
      return &(to_const_ptr(m_trackStatesAux)->calibrated[istate]);
    }
    case "measdim"_hash:
      return &(to_const_ptr(m_trackStatesAux)->measDim[istate]);
    case "typeFlags"_hash:
      return &(to_const_ptr(m_trackStatesAux)->typeFlags[istate]);
    default: {
      for (auto& d : m_decorations) {
        if (d.hash == key) {
          INSPECTCALL("getting dynamic variable " << d.name << " " << istate);
          return d.getter(m_trackStatesAux.get(), istate, d.auxid);
        }
      }
      throw std::runtime_error("MutableMultiTrajectory::component_impl const no such component " + std::to_string(key));
    }
  }
}

bool ActsTrk::MutableMultiTrajectory::has_impl(
    Acts::HashedString key, ActsTrk::IndexType istate) const {
  std::optional<bool> inTrackState = ::has_impl(m_trackStatesAux.get(), key, istate);
  if (inTrackState.has_value())
    return inTrackState.value();

  // TODO remove once EL based source links are in use only
  using namespace Acts::HashedStringLiteral;
  if (key == "uncalibratedSourceLink"_hash){
    INSPECTCALL(key << " " << istate << " uncalibratedSourceLink")
    static const SG::Accessor<const xAOD::UncalibratedMeasurement*> acc{"uncalibratedMeasurement"};
    bool has_auxid= m_trackStatesAux->getAuxIDs().test(acc.auxid());
    if (has_auxid) {
      if(getElement(*m_trackStatesAux, acc, istate) != nullptr) {
        return true;
      }
    }

    return (istate < m_uncalibratedSourceLinks.size() &&  m_uncalibratedSourceLinks[istate].has_value());
  }

  for (auto& d : m_decorations) {
    if (d.hash == key) {
      INSPECTCALL(key << " " << istate << " d.name")
      return true;
    }
  }

  return false;
}

void ActsTrk::MutableMultiTrajectory::clear_impl() {
  INSPECTCALL(this);
  m_trackStatesAux->resize(0);
  m_trackStatesSize = 0;

  m_trackParametersAux->resize(0);
  m_trackParametersSize = 0;

  m_trackJacobiansAux->resize(0);
  m_trackJacobiansSize = 0;

  m_trackMeasurementsAux->resize(0);
  m_trackMeasurementsSize = 0;

  m_surfacesBackend->clear();
  m_surfaces.clear();
  m_calibratedSourceLinks.clear();
  m_uncalibratedSourceLinks.clear();
}


ActsTrk::IndexType ActsTrk::MutableMultiTrajectory::calibratedSize_impl(ActsTrk::IndexType istate) const {
  // Retrieve the calibrated measurement size
  // INSPECTCALL(istate << " " << trackMeasurements().size());
  return m_trackStatesAux->measDim[istate];
}


void ActsTrk::MutableMultiTrajectory::setUncalibratedSourceLink_impl(ActsTrk::IndexType istate,
                                    const Acts::SourceLink& sourceLink) {
  INSPECTCALL( istate );

  static const SG::Decorator<const xAOD::UncalibratedMeasurement*> decor{"uncalibratedMeasurement"};
  if (istate>= m_trackStatesAux->size()) {
     throw std::range_error("istate out of range on TrackStates when attempting to access uncalibrated measurements");
  }
  std::span<const xAOD::UncalibratedMeasurement*> uncalibratedMeasurements = createDecoration(*m_trackStatesAux, decor);

  // @TODO normally the source links should contain an xAOD::UncalibratedMeasurement pointer
  //       but in some cases some other payload is used e.g. when converting Trk::Tracks.
  //       Only UncalibratedMeasurement pointer can be stored in the xAOD backend. Everything
  //       else has to go into the cache.
  //       Currently there is no possibility to check whether the source link contains a certain payload
  //       so can only catch the bad any cast
  try {
     uncalibratedMeasurements[istate] = sourceLink.get<ATLASUncalibSourceLink>();
  }
  catch  (std::bad_any_cast &err) {
     assert( istate < m_uncalibratedSourceLinks.size());
     m_uncalibratedSourceLinks[istate] = sourceLink;
  }
}

void ActsTrk::MutableMultiTrajectory::setReferenceSurface_impl(IndexType istate,
                                std::shared_ptr<const Acts::Surface> surface) {
  if ( istate >= m_surfaces.size() )
    m_surfaces.resize(istate+1, nullptr);

  m_trackStatesAux->geometryId[istate] = surface->geometryId().value();
  if (surface->geometryId().value() == 0) { // free surface, needs recording of properties
    m_surfacesBackend->push_back(new xAOD::TrackSurface());
    encodeSurface(m_surfacesBackendAux.get(), m_surfacesBackendAux->size()-1, surface.get(), m_geoContext.context());
    m_trackStatesAux->surfaceIndex[istate] =  m_surfacesBackend->size()-1;
    m_surfaces[istate] = std::move(surface); // and memory management

  } else {
    m_surfaces[istate] = surface.get(); // no memory management, bare pointer
  }
  // store surface representation in
}

namespace {
  const Acts::Surface* toSurfacePtr(const ActsTrk::StoredSurface& surfaceVariant) {
    if ( std::holds_alternative<const Acts::Surface*>(surfaceVariant)) 
      return std::get<const Acts::Surface*>(surfaceVariant);
    return std::get<std::shared_ptr<const Acts::Surface>>(surfaceVariant).get();
  }
}

const Acts::Surface* ActsTrk::MutableMultiTrajectory::referenceSurface_impl(IndexType istate) const {
  if ( istate >= m_surfaces.size() ) throw std::out_of_range("MutableMultiTrajectory index " + std::to_string(istate) + " out of range " + std::to_string(m_surfaces.size()) + " when accessing reference surface");
  return toSurfacePtr(m_surfaces[istate]);
}

void ActsTrk::MutableMultiTrajectory::copyDynamicFrom_impl (ActsTrk::IndexType istate,
                            Acts::HashedString key,
                            const std::any& src_ptr) {
  for (const ActsTrk::detail::Decoration& d : m_decorations) {
    if (d.hash == key) {
      d.copier(m_trackStatesAux.get(), istate, d.auxid, src_ptr);
      return;
    }
  }
  throw std::runtime_error("MultiTrajectory::copyDynamicFrom_impl no such decoration in destination MTJ " + std::to_string(key));
}

std::vector<Acts::HashedString> ActsTrk::MutableMultiTrajectory::dynamicKeys_impl() const {
  std::vector<Acts::HashedString> keys;
  for ( const ActsTrk::detail::Decoration& d: m_decorations) {
    keys.push_back(d.hash);
  }
  return keys;
}


void ActsTrk::MutableMultiTrajectory::trim() {
  m_trackStatesAux->resize(m_trackStatesSize);
  static const SG::Decorator<const xAOD::UncalibratedMeasurement*> decor{"uncalibratedMeasurement"};
  std::span<const xAOD::UncalibratedMeasurement*> uncalibratedMeasurements = createDecoration(*m_trackStatesAux, decor);
  (void) uncalibratedMeasurements;
  m_trackMeasurementsAux->resize(m_trackMeasurementsSize);
  m_trackJacobiansAux->resize(m_trackJacobiansSize);
  m_trackParametersAux->resize(m_trackParametersSize);
}

/////////////////////////////////////////////////////////
// ReadOnly MTJ
ActsTrk::MultiTrajectory::MultiTrajectory(
    DataLink<xAOD::TrackStateAuxContainer> trackStates,
    DataLink<xAOD::TrackParametersAuxContainer> trackParameters,
    DataLink<xAOD::TrackJacobianAuxContainer> trackJacobians,
    DataLink<xAOD::TrackMeasurementAuxContainer> trackMeasurements, 
    DataLink<xAOD::TrackSurfaceAuxContainer> trackSurfaces)
    : m_trackStatesAux(trackStates),
      m_trackParametersAux(trackParameters),
      m_trackJacobiansAux(trackJacobians),
      m_trackMeasurementsAux(trackMeasurements), 
      m_trackSurfacesAux(trackSurfaces) {
      INSPECTCALL("ctor " << this << " " << m_trackStatesAux->size());
      m_decorations = ActsTrk::detail::restoreDecorations(m_trackStatesAux, ActsTrk::MutableMultiTrajectory::s_staticVariables);
}

ActsTrk::MultiTrajectory::MultiTrajectory(const ActsTrk::MutableMultiTrajectory& other)
  : m_trackStatesAux(other.m_trackStatesAux.get()),
  m_trackParametersAux(other.m_trackParametersAux.get()),
  m_trackJacobiansAux(other.m_trackJacobiansAux.get()),
  m_trackMeasurementsAux(other.m_trackMeasurementsAux.get()), 
  m_trackSurfacesAux(other.m_surfacesBackendAux.get()) {
  INSPECTCALL("ctor " << this << " " << m_trackStatesAux->size());
  m_decorations = ActsTrk::detail::restoreDecorations(m_trackStatesAux, ActsTrk::MutableMultiTrajectory::s_staticVariables);
}


bool ActsTrk::MultiTrajectory::has_impl(Acts::HashedString key,
                                             ActsTrk::IndexType istate) const {
  // const auto& trackStates = *m_trackStates;
  std::optional<bool> inTrackState = ::has_impl(m_trackStatesAux, key, istate);
  if (inTrackState.has_value())
    return inTrackState.value();
  // TODO remove once EL based source links are in use only
  using namespace Acts::HashedStringLiteral;
  if (key == "uncalibratedSourceLink"_hash) {
      static const SG::Accessor<const xAOD::UncalibratedMeasurement*> acc{"uncalibratedMeasurement"};
      bool has_auxid= m_trackStatesAux->getAuxIDs().test(acc.auxid());
      if (has_auxid) {
         return getElement( *m_trackStatesAux, acc, istate) != nullptr;
      }
      else {
         return (istate < m_uncalibratedSourceLinks.size() &&  m_uncalibratedSourceLinks[istate].has_value());
      }
  }
  return false;
}

std::any ActsTrk::MultiTrajectory::component_impl(
    Acts::HashedString key, ActsTrk::IndexType istate) const {
  using namespace Acts::HashedStringLiteral;
  switch (key) {
    case "previous"_hash:
      return &(m_trackStatesAux->previous[istate]);
    case "next"_hash:
      return &(m_trackStatesAux->next[istate]);
    case "chi2"_hash:
      return &(m_trackStatesAux->chi2[istate]);
    case "pathLength"_hash:
      return &(m_trackStatesAux->pathLength[istate]);
    case "predicted"_hash:
      return &(m_trackStatesAux->predicted[istate]);
    case "filtered"_hash:
      return &(m_trackStatesAux->filtered[istate]);
    case "smoothed"_hash:
      return &(m_trackStatesAux->smoothed[istate]);
    case "jacobian"_hash:
      return &(m_trackStatesAux->jacobian[istate]);
    case "projector"_hash: {
      return &(m_trackMeasurementsAux->projector.at(m_trackStatesAux->calibrated[istate]));
    }
    case "calibrated"_hash: {
      return &(m_trackStatesAux->calibrated[istate]);
    }
    case "calibratedCov"_hash: {
      return &(m_trackStatesAux->calibrated[istate]);
    }
    case "measdim"_hash:
      return &(m_trackStatesAux->measDim[istate]);
    case "typeFlags"_hash:
      return &(m_trackStatesAux->typeFlags[istate]);
    default: {
      for (auto& d : m_decorations) {
        if (d.hash == key) {
          // TODO the dynamic_cast will disappear
          return  d.getter(m_trackStatesAux.cptr(), istate, d.auxid);
        }
      }
      throw std::runtime_error("MultiTrajectory::component_impl no such component " + std::to_string(key));
    }
  }
}

bool ActsTrk::MultiTrajectory::hasColumn_impl(
    Acts::HashedString key) const {
  using namespace Acts::HashedStringLiteral;
  // TODO try using staticVariables set
  switch (key) {
    case "previous"_hash:
    case "next"_hash:
    case "chi2"_hash:
    case "pathLength"_hash:
    case "predicted"_hash:
    case "filtered"_hash:
    case "smoothed"_hash:
    case "jacobian"_hash:
    case "projector"_hash:
    case "uncalibratedSourceLink"_hash:
    case "calibrated"_hash:
    case "calibratedCov"_hash:
    case "measdim"_hash:
    case "typeFlags"_hash:
      return true;
  }
  for (auto& d : m_decorations) {
      if (d.hash == key) {
        return true;
      }
    }
  return false;
}

ActsTrk::IndexType
ActsTrk::MultiTrajectory::calibratedSize_impl(ActsTrk::IndexType istate) const {
   return m_trackStatesAux->measDim[istate];
}

void ActsTrk::MultiTrajectory::moveSurfaces(const ActsTrk::MutableMultiTrajectory* mtj) {
  m_surfaces = std::move(mtj->m_surfaces);
}

// TODO remove this implementation once tracking uses only sourceLinks with EL
void ActsTrk::MultiTrajectory::moveLinks(const ActsTrk::MutableMultiTrajectory* mtj) {
  m_calibratedSourceLinks = std::move(mtj->m_calibratedSourceLinks);
  m_uncalibratedSourceLinks = std::move(mtj->m_uncalibratedSourceLinks);
}

void ActsTrk::MultiTrajectory::fillSurfaces(const Acts::TrackingGeometry* geo, const Acts::GeometryContext& geoContext ) {
  if ( not m_surfaces.empty() )
    return;
  m_surfaces.resize(m_trackStatesAux->size(), nullptr);
  for ( IndexType i = 0; i < m_trackStatesAux->size(); i++ ) {
      auto geoID = m_trackStatesAux->geometryId[i];
      if ( geoID ==  InvalidGeoID ) {
        m_surfaces[i] = nullptr;
        continue;
      }
      if ( geoID != 0 ) {
        m_surfaces[i] = geo->findSurface(geoID);
      } else {
        unsigned int backendIndex = m_trackStatesAux->surfaceIndex[i];
        std::shared_ptr<const Acts::Surface> surface = decodeSurface( m_trackSurfacesAux, backendIndex, geoContext);
        m_surfaces[i] = surface; // TODO

      }
  }
}


const Acts::Surface* ActsTrk::MultiTrajectory::referenceSurface_impl(IndexType istate) const {
  INSPECTCALL( this <<  " " << istate << " " << m_trackStatesAux->size() << " " << m_surfaces.size() << " surf ptr " << toSurfacePtr(m_surfaces[istate]));
  if ( istate >= m_surfaces.size() ) throw std::out_of_range("MultiTrajectory index " + std::to_string(istate) + " out of range " + std::to_string(m_surfaces.size()) + " when accessing reference surface");
  return toSurfacePtr(m_surfaces[istate]);
}

typename Acts::SourceLink
ActsTrk::MutableMultiTrajectory::getUncalibratedSourceLink_impl(
    ActsTrk::IndexType istate) const {
  assert(istate < m_trackStatesAux->size());
  // at the moment when converting Trk::Track to Acts tracks the measurements on track
  // are not converted to xAOD::UncalibratedMeasurements, and the Acts::SourceLinks
  // just contain a pointer to the Trk::Measurement. To keep this functionality
  // SourceLinks are either stored in the m_uncalibratedSourceLinks cache
  // or taken from the xAOD backend.
  static const SG::Accessor<const xAOD::UncalibratedMeasurement*> acc{"uncalibratedMeasurement"};
  if (m_trackStatesAux->getAuxIDs().test(acc.auxid())){
    if(auto* ptr = getElement(*m_trackStatesAux, acc, istate); ptr != nullptr) {
      return Acts::SourceLink( ptr );
    }
  }
  return m_uncalibratedSourceLinks[istate].value();
}

typename Acts::SourceLink
ActsTrk::MultiTrajectory::getUncalibratedSourceLink_impl(
    ActsTrk::IndexType istate) const {
  assert(istate < m_trackStatesAux->size());
  // at the moment when converting Trk::Track to Acts tracks the measurements on track
  // are not converted to xAOD::UncalibratedMeasurements, and the Acts::SourceLinks
  // just contain a pointer to the Trk::Measurement. To keep this functionality
  // SourceLinks are either stored in the m_uncalibratedSourceLinks cache
  // or taken from the xAOD backend.
  static const SG::Accessor<const xAOD::UncalibratedMeasurement*> acc{"uncalibratedMeasurement"};
  if (m_trackStatesAux->getAuxIDs().test(acc.auxid())){
    if(auto* ptr = getElement(*m_trackStatesAux, acc, istate); ptr != nullptr) {
      return Acts::SourceLink( ptr );
    }
  }
  return m_uncalibratedSourceLinks[istate].value();
}
