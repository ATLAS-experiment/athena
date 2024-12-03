/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/TrackSummaryContainer.h"
#include <Acts/EventData/Types.hpp>
#include <stdexcept>
#include "xAODTracking/TrackSummary.h"
#include "ActsEvent/ParticleHypothesisEncoding.h"

// this is list of xAOD container variable names that are "hardcoded" in TrackSummary_v1
// their compatibility is maintain ed by the unit tests: AllStaticxAODVaraiblesAreKnown
const std::set<std::string> ActsTrk::TrackSummaryContainer::staticVariables = {
    "params", "covParams", "nMeasurements", "nHoles",   "chi2f",
    "ndf",    "nOutliers", "nSharedHits",   "tipIndex", "stemIndex",
    "particleHypothesis", "surfaceIndex"};

using namespace Acts::HashedStringLiteral;
const std::set<Acts::HashedString> ActsTrk::TrackSummaryContainer::staticVariableHashes = [](){
  std::set<Acts::HashedString> result;
  for (const auto& s: ActsTrk::TrackSummaryContainer::staticVariables) {
    result.insert(Acts::hashStringDynamic(s));
  }
  return result;
}();


ActsTrk::TrackSummaryContainer::TrackSummaryContainer(
    const DataLink<xAOD::TrackSummaryContainer>& link)
    : m_trackBackend(link)
    {}

const Acts::Surface* ActsTrk::TrackSummaryContainer::referenceSurface_impl(
    ActsTrk::IndexType itrack) const {
  if (itrack >= m_surfaces.size())
    throw std::out_of_range(
        "TrackSummaryContainer index out of range when accessing reference "
        "surface");
  return m_surfaces[itrack].get();
}

Acts::ParticleHypothesis ActsTrk::TrackSummaryContainer::particleHypothesis_impl(IndexType itrack) const{
  return ActsTrk::ParticleHypothesis::convert( static_cast<xAOD::ParticleHypothesis>(m_trackBackend->at(itrack)->particleHypothesis()));
}

std::size_t ActsTrk::TrackSummaryContainer::size_impl() const {
  return m_trackBackend->size();
}

namespace {
template <typename C>
std::any component_impl(C& container, Acts::HashedString key,
                        ActsTrk::IndexType itrack) {
  using namespace Acts::HashedStringLiteral;
  switch (key) {
    case "nMeasurements"_hash:
      return container.at(itrack)->nMeasurementsPtr();
    case "nHoles"_hash:
      return container.at(itrack)->nHolesPtr();
    case "chi2"_hash:
      return container.at(itrack)->chi2fPtr();
    case "ndf"_hash:
      return container.at(itrack)->ndfPtr();
    case "nOutliers"_hash:
      return container.at(itrack)->nOutliersPtr();
    case "nSharedHits"_hash:
      return container.at(itrack)->nSharedHitsPtr();
    case "tipIndex"_hash:
      return container.at(itrack)->tipIndexPtr();
    case "stemIndex"_hash:
      return container.at(itrack)->stemIndexPtr();

    default:
      return std::any();
  }
}
}  // namespace

std::any ActsTrk::TrackSummaryContainer::component_impl(
    Acts::HashedString key, ActsTrk::IndexType itrack) const {
  std::any result = ::component_impl(*m_trackBackend, key, itrack);
  if (result.has_value()) {
    return result;
  }
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      // TODO the dynamic case will be eliminated once we switch to use Aux containers directly
      return d.getter(m_trackBackend->getStore(), itrack, d.auxid);
    }
  }
  throw std::runtime_error("TrackSummaryContainer no such component " +
                           std::to_string(key));
}

ActsTrk::ConstParameters ActsTrk::TrackSummaryContainer::parameters(
    ActsTrk::IndexType itrack) const {
  return m_trackBackend->at(itrack)->paramsEigen();
}

ActsTrk::ConstCovariance ActsTrk::TrackSummaryContainer::covariance(
    ActsTrk::IndexType itrack) const {
  return m_trackBackend->at(itrack)->covParamsEigen();
}

void ActsTrk::TrackSummaryContainer::fillFrom(
    ActsTrk::MutableTrackSummaryContainer& mtb) {
  m_surfaces = std::move(mtb.m_surfaces);
}

void ActsTrk::TrackSummaryContainer::restoreDecorations() {
  m_decorations = ActsTrk::detail::restoreDecorations(m_trackBackend->getConstStore(), staticVariables);
}

std::vector<Acts::HashedString> ActsTrk::TrackSummaryContainer::dynamicKeys_impl() const {
  std::vector<Acts::HashedString> result;
  for ( const auto& d: m_decorations) {
    if (staticVariableHashes.count(d.hash) == 1) {
      continue;
    }
    result.push_back(d.hash);
  }
  return result;
}

void ActsTrk::TrackSummaryContainer::decodeSurfaces(const xAOD::TrackSurfaceContainer* src, const Acts::GeometryContext& geoContext) {
  for ( auto xAODSurfacePtr: *src) {
    m_surfaces.push_back( decodeSurface(xAODSurfacePtr, geoContext));
  }
}




////////////////////////////////////////////////////////////////////
// write api
////////////////////////////////////////////////////////////////////
ActsTrk::MutableTrackSummaryContainer::MutableTrackSummaryContainer() {

  m_mutableTrackBackend = std::make_unique<xAOD::TrackSummaryContainer>();
  m_mutableTrackBackendAux = std::make_unique<xAOD::TrackSummaryAuxContainer>();
  m_mutableTrackBackend->setStore(m_mutableTrackBackendAux.get());

  TrackSummaryContainer::m_trackBackend = m_mutableTrackBackend.get();
}

ActsTrk::MutableTrackSummaryContainer::MutableTrackSummaryContainer(
    MutableTrackSummaryContainer&& other) {
  m_mutableTrackBackend = std::move(other.m_mutableTrackBackend);
  m_mutableTrackBackendAux = std::move(other.m_mutableTrackBackendAux);
  m_mutableTrackBackend->setStore(m_mutableTrackBackendAux.get());
  TrackSummaryContainer::m_trackBackend = m_mutableTrackBackend.get();

  m_surfaces = std::move(other.m_surfaces);
  m_decorations = std::move(other.m_decorations);
}

// move assignment operator
ActsTrk::MutableTrackSummaryContainer& ActsTrk::MutableTrackSummaryContainer::operator = (
    ActsTrk::MutableTrackSummaryContainer&& other) noexcept {
  //NB. restoreDecorations may throw a GaudiException, resulting in a call to terminate()
  // because the function is annotated 'noexcept'
  m_mutableTrackBackend = std::exchange(other.m_mutableTrackBackend, nullptr);
  m_mutableTrackBackendAux = std::exchange(other.m_mutableTrackBackendAux, nullptr);
  //setStore throws an exception of type SG::ExcUntrackedSetStore, SG::ExcCLIDMismatch
  //resulting in a call to terminate() because the function is marked 'noexcept'
  m_mutableTrackBackend->setStore(m_mutableTrackBackendAux.get());
  TrackSummaryContainer::m_trackBackend = m_mutableTrackBackend.get();

  m_surfaces = std::move(other.m_surfaces);
  m_decorations = std::move(other.m_decorations);

  //restore decorations
  // restoreDecorations may throw a GaudiException or SG::ExcBadVarName 
  // resulting in a call to terminate() because the function is marked 'noexcept'
  restoreDecorations(); 

  // invalidate vector type components of 'other'
  other.m_surfaces.clear();
  other.m_decorations.clear();

  return *this;
}

ActsTrk::IndexType ActsTrk::MutableTrackSummaryContainer::addTrack_impl() {
  m_mutableTrackBackend->push_back(std::make_unique<xAOD::TrackSummary>());
  m_mutableTrackBackend->back()->resize();
  // ACTS assumes default to be pion, xAOD::ParticleHypothesis == 0 is geantino
  m_mutableTrackBackend->back()->setParticleHypothesis(xAOD::pion);
  m_surfaces.push_back(nullptr);
  return m_mutableTrackBackend->size() - 1;
}

void ActsTrk::MutableTrackSummaryContainer::removeTrack_impl(
    ActsTrk::IndexType itrack) {
  if (itrack >= m_mutableTrackBackend->size()) {
    throw std::out_of_range("removeTrack_impl");
  }
  m_mutableTrackBackend->erase(m_mutableTrackBackend->begin() + itrack);
}

// this in fact may be a copy from other MutableTrackSymmaryContainer
void ActsTrk::MutableTrackSummaryContainer::copyDynamicFrom_impl(
    ActsTrk::IndexType itrack, Acts::HashedString key,
    const std::any& src_ptr) {
  if ( staticVariableHashes.count(key) == 1)  { return; }
  for ( const auto& d: m_decorations) {
    if (d.hash != key) { continue; }
    d.copier(m_mutableTrackBackendAux.get(), itrack, d.auxid, src_ptr);
  }
}

std::any ActsTrk::MutableTrackSummaryContainer::component_impl(
    Acts::HashedString key, ActsTrk::IndexType itrack) {
  std::any result = ::component_impl(*m_mutableTrackBackend, key, itrack);
  if (result.has_value()) {
    return result;
  }
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      return d.setter(m_mutableTrackBackendAux.get(), itrack, d.auxid);
    }
  }
  throw std::runtime_error("TrackSummaryContainer no such component " +
                           std::to_string(key));
}

ActsTrk::Parameters ActsTrk::MutableTrackSummaryContainer::parameters(
    ActsTrk::IndexType itrack) {
  return m_mutableTrackBackend->at(itrack)->paramsEigen();
}

ActsTrk::Covariance ActsTrk::MutableTrackSummaryContainer::covariance(
    ActsTrk::IndexType itrack) {
  return m_mutableTrackBackend->at(itrack)->covParamsEigen();
}

void ActsTrk::MutableTrackSummaryContainer::ensureDynamicColumns_impl(
    const MutableTrackSummaryContainer& other) {
  for (auto& d : other.m_decorations) {
    m_decorations.push_back(d);
  }
}

void ActsTrk::MutableTrackSummaryContainer::ensureDynamicColumns_impl(
    const TrackSummaryContainer& other) {
  for (auto& d : other.m_decorations) {
    m_decorations.push_back(d);
  }
}

void ActsTrk::MutableTrackSummaryContainer::reserve(ActsTrk::IndexType size) {
  m_mutableTrackBackend->reserve(size);
}

void ActsTrk::MutableTrackSummaryContainer::clear() {
  m_mutableTrackBackend->clear();
  m_surfaces.clear();
}

void ActsTrk::MutableTrackSummaryContainer::setReferenceSurface_impl(
    ActsTrk::IndexType itrack, std::shared_ptr<const Acts::Surface> surface) {
  m_surfaces[itrack] = surface;
}

void ActsTrk::MutableTrackSummaryContainer::encodeSurfaces(xAOD::TrackSurfaceAuxContainer* dest, const Acts::GeometryContext& geoContext) {
  dest->resize(m_surfaces.size());
  size_t destIndex = 0;
  // go over all surfaces and for each free surface record persistent version of it in the xAOD
  // at the same time store index to it updating TrackSummary
  for ( ActsTrk::IndexType index = 0, eindex = m_surfaces.size(); index < eindex; ++index ) {
    if ( m_surfaces[index] == nullptr or m_surfaces[index]->geometryId().value() != 0  ) {
      m_mutableTrackBackend->at(index)->setSurfaceIndex(Acts::kTrackIndexInvalid);
    } else {
      m_mutableTrackBackend->at(index)->setSurfaceIndex(destIndex);
      encodeSurface(dest, destIndex, m_surfaces[index].get(), geoContext);
      destIndex++;
    }
  }
  dest->resize(destIndex);
}

void ActsTrk::MutableTrackSummaryContainer::setParticleHypothesis_impl(ActsTrk::IndexType itrack, const Acts::ParticleHypothesis& particleHypothesis) {
  m_mutableTrackBackend->at(itrack)->setParticleHypothesis(ActsTrk::ParticleHypothesis::convert(particleHypothesis));
}
