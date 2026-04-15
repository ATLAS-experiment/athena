/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#pragma GCC optimize ("O0")
#undef NDEBUG
#include "ActsGeometry/ActsDetectorElement.h"
#include "src/detail/TrackFindingMeasurements.h"

#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"
#include "ActsGeometry/SurfaceOfMeasurementUtil.h"
#include "xAODInDetMeasurement/PixelClusterAuxDataCache.h"
#include "xAODInDetMeasurement/StripClusterAuxDataCache.h"
// #include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "AuxDataCacheList.icc"

// template <typename T_Container>
// concept has_getPerModuleMeasurementRanges = requires(T_Container &&a) { a.getPerModuleMeasurementRanges(); };
namespace {
   template <typename T_Container>
   concept has_containerPerRange = requires(T_Container &&a, const PhaseII::DataRange &range) { a.container(range); };
}

namespace ActsTrk::detail {
  namespace {
  std::array< unsigned int, static_cast<unsigned int>(xAOD::UncalibMeasType::nTypes)> makeDetectorTypes() {

    // MeasurementTypes xAOD::UncalibMeasType::
    // // InDet
    // PixelClusterType = 1,
    // StripClusterType = 2,
    // // Muon
    // MdtDriftCircleType = 3,
    // RpcStripType = 4,
    // TgcStripType = 5,
    // MMClusterType = 6,
    // sTgcStripType = 7,
    // // HGTD
    // HGTDClusterType = 8,
    // // Do not add anything after nTypes
    // nTypes
    //
    // to detector typese ActsTrk::DetectorType
    //  /// Inner detector legacy
    // Pixel,
    // Sct,
    // /// Maybe the Sct / Pixel for Itk become seperate entries?
    // Trt,
    // Hgtd,
    // /// MuonSpectrometer
    // Mdt,  /// Monitored Drift Tubes
    // Rpc,  /// Resitive Plate Chambers
    // Tgc,  /// Thin gap champers
    // Csc,  /// Maybe not needed in the migration
    // Mm,   /// Micromegas (NSW)
    // sTgc,  /// Small Thing Gap chambers (NSW)
    // UnDefined
      
     std::array< unsigned int, static_cast<unsigned int>(xAOD::UncalibMeasType::nTypes)> detectorTypes;
     std::fill(detectorTypes.begin(),detectorTypes.end(),static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined));
     detectorTypes[static_cast<unsigned int>(xAOD::UncalibMeasType::PixelClusterType)]=static_cast<unsigned int>(ActsTrk::DetectorType::Pixel);
     detectorTypes[static_cast<unsigned int>(xAOD::UncalibMeasType::StripClusterType)]=static_cast<unsigned int>(ActsTrk::DetectorType::Sct);
     detectorTypes[static_cast<unsigned int>(xAOD::UncalibMeasType::HGTDClusterType)]=static_cast<unsigned int>(ActsTrk::DetectorType::Hgtd);
     return detectorTypes;
  }
  }

  TrackFindingMeasurements::TrackFindingMeasurements([[maybe_unused]] std::size_t nMeasurementContainerMax)
  {}
  //     : m_measurementOffsets(nMeasurementContainerMax, 0ul) {
  //   m_containers.reserve(nMeasurementContainerMax);
  // }

  void TrackFindingMeasurements::addMeasurements(std::size_t typeIndex,
                                                const xAOD::UncalibratedMeasurementContainer &clusterContainer,
                                                const DetectorElementToActsGeometryIdMap &detectorElementToGeoid,
                                                const MeasurementIndex *measurementIndex) {
    unsigned int typeIndex32 = static_cast<unsigned int>(typeIndex);
    static const std::array< unsigned int, static_cast<unsigned int>(xAOD::UncalibMeasType::nTypes)> detectorTypes=makeDetectorTypes();
    // if (typeIndex < m_measurementOffsets.size())
    //   m_measurementOffsets[typeIndex] = m_measurementsTotal;
    // if (!(typeIndex < m_containers.size()))
    //   m_containers.resize(typeIndex + 1);
    // m_containers[typeIndex] = &clusterContainer;
    // if (measurementIndex)
    //   m_surfaceIndices.resize(measurementIndex->size());

    // if (m_measurementRanges.empty()) {
    //   // try to reserve needed space,
    //   // this however will reserve more than necessary not just the space needed for the surfaces of
    //   // all the measurements that are going to be added (e.g. pixel+strips).
    //   m_measurementRanges.reserve(detectorElementToGeoid.size());
    // }
    m_measurementRanges.setContainer(typeIndex32, &clusterContainer);
    if (!clusterContainer.empty()) {
       unsigned int detector_type_i = detectorTypes.at(static_cast<unsigned int>(clusterContainer.front()->type()));
       std::visit( [&](auto &container) {
          using T_Container = std::remove_cvref_t<decltype(container.container())>;
          using PixelClusterAuxDataCacheProxy =  ClusterAuxDataCacheWithClusterAccess<PixelClusterAuxDataCacheCollection >;
          
          using StripClusterAuxDataCacheProxy =  ClusterAuxDataCacheWithClusterAccess<StripClusterAuxDataCacheCollection >;
          // || std::is_same_v<T_Container,xAOD::StripClusterContainerAlt>
          static_assert(    std::is_same_v<T_Container,PixelClusterAuxDataCacheProxy>
                         || std::is_same_v<T_Container,StripClusterAuxDataCacheProxy>
                         || std::is_same_v<T_Container,xAOD::HGTDClusterContainer>);
          
          if constexpr(traits::has_moduleIndex<T_Container>) {
             m_measurementRanges.setRange( detector_type_i, container.container().moduleIndex().m_range, typeIndex32);
             
             // using AllClusterProxy=typename traits::ElementProxies<T_Container>::AllClusterProxy<Utils::AccessPolicy::Const>;
             // AllClusterProxy clusters(container.containerPtr());
             // if (!clusters.empty()) {             
             //    auto first_cluster  = AllClusterProxy(static_cast<const T_AuxDataCache *>(this)).front();
             // }
          }
          else {
              auto max_iter=std::max_element( container.container().begin(), container.container().end(), [](const auto *measurement_a, const auto *measurement_b) {
                 return measurement_a->identifierHash() < measurement_b->identifierHash();
              });
              if (max_iter != container.container().end()) {
                 m_measurementRanges.customRanges().emplace_back();
                 m_measurementRanges.customRanges().back().resize((*max_iter)->identifierHash()+1u);
                 // if the cotainer does not provide a moduleIndex then the per module element range list needs to be rebuilt:
                 std::size_t n_elements = clusterContainer.size();
                 unsigned int idx = 0;
                 unsigned int begin_idx=0u;
                 xAOD::DetectorIDHashType current_idHash = clusterContainer[idx]->identifierHash();
                 for (; ++idx < n_elements; ) {
                    const auto *measurement = clusterContainer[idx];
                    xAOD::DetectorIDHashType idHash = measurement->identifierHash();
                    if (idHash != current_idHash) {
                       if (static_cast<uint16_t>(idx - begin_idx) != idx-begin_idx) {
                          throw std::domain_error("Exceeded maximum number of measurements per hash ID.");
                       }
                       m_measurementRanges.customRanges().back().at(current_idHash)=PhaseII::DataRange( begin_idx,
                                                                                                        idx - begin_idx,
                                                                                                        0u).makeCompact();
                       current_idHash=idHash;
                       begin_idx=idx;
                    }
                 }
                 if (static_cast<uint16_t>(idx - begin_idx) != idx-begin_idx) {
                    throw std::domain_error("Exceeded maximum number of measurements per hash ID.");
                 }
                 m_measurementRanges.customRanges().back().at(current_idHash)=PhaseII::DataRange( begin_idx,
                                                                                                  idx - begin_idx,
                                                                                                  0u).makeCompact();
                 m_measurementRanges.setRange( detector_type_i,
                                               std::span<ActsTrk::detail::DataRangeValueType>(m_measurementRanges.customRanges().back().begin(),
                                                                                              m_measurementRanges.customRanges().back().end()),
                                               typeIndex32);
              }
          }
       }, m_measurementRanges.container(typeIndex32));
    }
  }

  void TrackFindingMeasurements::setDetectorElementStatus(const std::array< const InDet::SiDetectorElementStatus *,
                                                          static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)+1u> &det_el_status_per_arr) {
     
     m_detectorElementStatusPerDetectorType=det_el_status_per_arr;
     m_measurementRanges.setDetectorElementStatus(m_detectorElementStatusPerDetectorType);
   }
   
  std::unique_ptr<MeasurementRangeListFlat>
  TrackFindingMeasurements::createMeasurementRangesForced(const ActsTrk::Seed &seed,
                                                          const MeasurementIndex &measurementIndex) const {
    static const std::array< unsigned int, static_cast<unsigned int>(xAOD::UncalibMeasType::nTypes)> detectorTypes=makeDetectorTypes();

    std::unique_ptr<MeasurementRangeListFlat> measurementRangesForced=std::make_unique<ActsTrk::detail::MeasurementRangeListFlat>();
    measurementRangesForced->setContainerList( m_measurementRanges.getFullContainerList() );
    measurementRangesForced->reserve(seed.sp().size());  // wrong for strip seeds, but just means an extra allocation in this rare case
    
    for (const xAOD::SpacePoint *sp : seed.sp()) {
      for (const xAOD::UncalibratedMeasurement *measurement : sp->measurements()) {
        
        unsigned int detector_type_i = detectorTypes.at(static_cast<unsigned int>(measurement->type()));
        if (detector_type_i>=static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)) {
           throw std::range_error("Unsupported detector type.");
        }
        unsigned int container_index = m_measurementRanges.getContainerIndex( detector_type_i, measurement->identifierHash());
        if (container_index >= measurementRangesForced->numContainers()) {
           throw std::range_error("Invalid container index");
        }
        const PhaseII::DataRange &module_range = m_measurementRanges.getRange(detector_type_i, measurement->identifierHash());
        if (measurement->index() < module_range.beginIndex() || measurement->index() >=module_range.endIndex()) {
           
           unsigned int index = measurement->index();
           auto abstract_container = m_measurementRanges.container(container_index);
           bool valid_index = std::visit([index, module_range](const auto &concrete_container){
              using T_Container = std::remove_cvref_t<decltype(concrete_container.container())>;
              if constexpr(has_containerPerRange<T_Container>) {
                 const auto &container_for_module= concrete_container.container().container(module_range);
                 std::size_t container_sz= container_for_module.size();
                 return index<container_sz;
              }
              else {
                 return false;
              }
           }, abstract_container);
           if (!valid_index) {
              throw std::range_error(std::string("Element index ")+std::to_string(index)+" not within range for module");
           }
        }
        auto ret = measurementRangesForced->insert(std::make_pair(measurementRangesForced->makeKey(detector_type_i, measurement->identifierHash() ),
                                                                  std::make_pair(PhaseII::DataRange(static_cast<unsigned>(measurement->index()),
                                                                                                    1u,
                                                                                                    module_range.containerIndex() | static_cast<std::uint16_t>(1u<<15)
                                                                                                    ),
                                                                                 container_index)));
      }
    }
    measurementRangesForced->setDetectorElementStatus(m_detectorElementStatusPerDetectorType);
    return measurementRangesForced;
  }

}  // namespace ActsTrk::detail
