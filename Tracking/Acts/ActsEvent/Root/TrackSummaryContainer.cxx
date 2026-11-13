/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/TrackSummaryContainer.h"
#include <Acts/EventData/Types.hpp>
#include <stdexcept>
#include "xAODTracking/TrackSummary.h"
#include "ActsEvent/ParticleHypothesisEncoding.h"
#include "ActsGeoUtils/SurfaceEncoding.h"



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
    case "nSplitHits"_hash:
      return container.at(itrack)->nSplitHitsPtr();
    case "tipIndex"_hash:
      return container.at(itrack)->tipIndexPtr();
    case "stemIndex"_hash:
      return container.at(itrack)->stemIndexPtr();

    default:
      return std::any();
  }
}
}  // namespace



namespace ActsTrk{
// this is list of xAOD container variable names that are "hardcoded" in TrackSummary_v1
// their compatibility is maintained by the unit tests: AllStaticxAODVariablesAreKnown
const std::set<std::string> TrackSummaryContainer::staticVariables = {
    "params", "covParams", "nMeasurements", "nHoles",   "chi2f",
    "ndf",    "nOutliers", "nSharedHits",   "nSplitHits", "tipIndex", "stemIndex",
    "particleHypothesis", "surfaceIndex"};

using namespace Acts::HashedStringLiteral;
const std::set<Acts::HashedString> TrackSummaryContainer::staticVariableHashes = [](){
  std::set<Acts::HashedString> result;
  for (const auto& s: TrackSummaryContainer::staticVariables) {
    result.insert(Acts::hashStringDynamic(s));
  }
  return result;
}();


TrackSummaryContainer::TrackSummaryContainer(
    const DataLink<xAOD::TrackSummaryContainer>& link)
    : m_trackBackend(link)
    {}

const Acts::Surface* TrackSummaryContainer::referenceSurface_impl(
    IndexType itrack) const {
  if (itrack >= m_surfaces.size())
    throw std::out_of_range(
        "TrackSummaryContainer index out of range when accessing reference "
        "surface");
  return m_surfaces[itrack].get();
}

Acts::ParticleHypothesis TrackSummaryContainer::particleHypothesis_impl(IndexType itrack) const{
  return ParticleHypothesis::convert( static_cast<xAOD::ParticleHypothesis>(m_trackBackend->at(itrack)->particleHypothesis()));
}

std::size_t TrackSummaryContainer::size_impl() const {
  return m_trackBackend->size();
}

std::string_view TrackSummaryContainer::columnName_impl(Acts::HashedString key) const {
    for (auto& d : m_decorations) {
        if (d.hash == key) {
            return d.name;
        }
    }
    throw std::runtime_error(std::format("{:}() - Key is unknown {:}",__func__, key));
    return "";
}
const std::type_info& TrackSummaryContainer::columnDataType_impl(Acts::HashedString key) const {
    for (auto& d : m_decorations) {
        if (d.hash == key) {
            return *SG::AuxTypeRegistry::instance().getType(d.auxid);
        }
    }
    throw std::runtime_error(std::format("{:}() - Key is unknown {:}",__func__, key));
    return typeid(const void*);
}

// 
std::any TrackSummaryContainer::component_impl(
    Acts::HashedString key, IndexType itrack) const {
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
  throw std::runtime_error(std::format("TrackSummaryContainer no such component {:}", key));
}

ConstParameters TrackSummaryContainer::parameters(
    IndexType itrack) const {
  return m_trackBackend->at(itrack)->paramsEigen();
}

ConstCovariance TrackSummaryContainer::covariance(
    IndexType itrack) const {
  return m_trackBackend->at(itrack)->covParamsEigen();
}

void TrackSummaryContainer::fillFrom(
    MutableTrackSummaryContainer& mtb) {
  m_surfaces = std::move(mtb.m_surfaces);
}

void TrackSummaryContainer::restoreDecorations() {
  m_decorations = detail::restoreDecorations(m_trackBackend->getConstStore(), staticVariables);
}

std::vector<Acts::HashedString> TrackSummaryContainer::dynamicKeys_impl() const {
  std::vector<Acts::HashedString> result;
  for ( const auto& d: m_decorations) {
    if (staticVariableHashes.count(d.hash) == 1) {
      continue;
    }
    result.push_back(d.hash);
  }
  return result;
}

void TrackSummaryContainer::decodeSurfaces(const xAOD::TrackSurfaceContainer* src) {
  m_surfaces.reserve(src->size());
  for ( auto xAODSurfacePtr: *src) {
    m_surfaces.push_back( decodeSurface(xAODSurfacePtr));
  }
}
void TrackSummaryContainer::excludeDecoration(std::string_view decorName) {
  m_excluded.insert(Acts::hashStringDynamic(decorName));
}




////////////////////////////////////////////////////////////////////
// write api
////////////////////////////////////////////////////////////////////
MutableTrackSummaryContainer::MutableTrackSummaryContainer() :
  m_mutableTrackBackend(std::make_unique<xAOD::TrackSummaryContainer>()),
  m_mutableTrackBackendAux(std::make_unique<xAOD::TrackSummaryAuxContainer>())
{
  m_mutableTrackBackend->setStore(m_mutableTrackBackendAux.get());
  TrackSummaryContainer::m_trackBackend = m_mutableTrackBackend.get();
}

MutableTrackSummaryContainer::MutableTrackSummaryContainer(
    MutableTrackSummaryContainer&& other) :
  m_mutableTrackBackend(std::move(other.m_mutableTrackBackend)),
  m_mutableTrackBackendAux(std::move(other.m_mutableTrackBackendAux))
{
  m_mutableTrackBackend->setStore(m_mutableTrackBackendAux.get());
  TrackSummaryContainer::m_trackBackend = m_mutableTrackBackend.get();

  m_surfaces = std::move(other.m_surfaces);
  m_decorations = std::move(other.m_decorations);
}

// move assignment operator
//coverity[exn_spec_violation]
MutableTrackSummaryContainer& MutableTrackSummaryContainer::operator = (
    MutableTrackSummaryContainer&& other) noexcept {
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
void MutableTrackSummaryContainer::ensureDynamicColumns_impl(const Acts::ConstVectorTrackContainer & /*other*/) {
#ifdef ACTIVATE_LATER
    ensureDynamicColumns(m_excluded, other, *this);
#endif
}
IndexType MutableTrackSummaryContainer::addTrack_impl() {
  m_mutableTrackBackend->push_back(std::make_unique<xAOD::TrackSummary>());
  m_mutableTrackBackend->back()->resize();
  // ACTS assumes default to be pion, xAOD::ParticleHypothesis == 0 is geantino
  m_mutableTrackBackend->back()->setParticleHypothesis(xAOD::pion);
  m_surfaces.push_back(nullptr);
  return m_mutableTrackBackend->size() - 1;
}

void MutableTrackSummaryContainer::removeTrack_impl(
    IndexType itrack) {
  if (itrack >= m_mutableTrackBackend->size()) {
    throw std::out_of_range("removeTrack_impl track backend");
  }
  if (itrack >= m_surfaces.size()) {
    throw std::out_of_range("removeTrack_impl surfaces");
  }
  m_mutableTrackBackend->erase(m_mutableTrackBackend->begin() + itrack);
  m_surfaces.erase(m_surfaces.begin() + itrack);
}

// this in fact may be a copy from other MutableTrackSymmaryContainer
void MutableTrackSummaryContainer::copyDynamicFrom_impl(
    IndexType itrack, Acts::HashedString key,
    const std::any& src_ptr) {
  if ( staticVariableHashes.count(key) == 1 || m_excluded.count(key))  { return; }
  for ( const auto& d: m_decorations) {
    if (d.hash != key) { continue; }
    d.copier(m_mutableTrackBackendAux.get(), itrack, d.auxid, src_ptr);
  }
}

std::any MutableTrackSummaryContainer::component_impl(
    Acts::HashedString key, IndexType itrack) {
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

Parameters MutableTrackSummaryContainer::parameters(
    IndexType itrack) {
  return m_mutableTrackBackend->at(itrack)->paramsEigen();
}

Covariance MutableTrackSummaryContainer::covariance(
    IndexType itrack) {
  return m_mutableTrackBackend->at(itrack)->covParamsEigen();
}

void MutableTrackSummaryContainer::ensureDynamicColumns_impl(
    const MutableTrackSummaryContainer& other) {
    m_decorations.insert_range(m_decorations.end(), other.m_decorations);
 
}

void MutableTrackSummaryContainer::ensureDynamicColumns_impl(
    const TrackSummaryContainer& other) {
    m_decorations.insert_range(m_decorations.end(), other.m_decorations);  
}

void MutableTrackSummaryContainer::reserve(IndexType size) {
  m_mutableTrackBackend->reserve(size);
}

void MutableTrackSummaryContainer::clear() {
  m_mutableTrackBackend->clear();
  m_surfaces.clear();
}

void MutableTrackSummaryContainer::setReferenceSurface_impl(
    IndexType itrack, std::shared_ptr<const Acts::Surface> surface) {
  m_surfaces[itrack] = std::move(surface);
}

void MutableTrackSummaryContainer::encodeSurfaces(xAOD::TrackSurfaceAuxContainer* dest, const Acts::GeometryContext& geoContext) {
  dest->resize(m_surfaces.size());
  size_t destIndex = 0;
  // go over all surfaces and for each free surface record persistent version of it in the xAOD
  // at the same time store index to it updating TrackSummary
  for ( IndexType index = 0, eindex = m_surfaces.size(); index < eindex; ++index ) {
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

void MutableTrackSummaryContainer::setParticleHypothesis_impl(IndexType itrack, const Acts::ParticleHypothesis& particleHypothesis) {
  m_mutableTrackBackend->at(itrack)->setParticleHypothesis(ParticleHypothesis::convert(particleHypothesis));
}
}