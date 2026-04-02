/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ATLASUNCALIBSROUCELINACCESOR_H
#define ATLASUNCALIBSROUCELINACCESOR_H

#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ATLASSourceLink.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"

//#include "xAODInDetMeasurement/PixelClusterAuxDataCacheCollection.h"
//#include "xAODInDetMeasurement/StripClusterAuxDataCacheCollection.h"
// #include "xAODInDetMeasurement/StripClusterAuxDataCache.h"

#include "InDetReadoutGeometry/SiDetectorElementStatus.h"

#include "src/detail/MeasurementContainerWithDimension.h"
#include "src/detail/AuxDataCacheList.h"

#include <variant>
#include <vector>
#include <unordered_map>
#include <utility>
#include <ranges>
#include <limits>
#include <algorithm>

namespace {
   template <typename T_Container>
   concept has_interfaceObject = requires(T_Container &&a, unsigned int cacheIndex, unsigned int index)
      { a.getInterfaceObject(cacheIndex, index); };
}

namespace ActsTrk::detail {

   using abstract_measurement_range_t = PhaseII::DataRange;
  
   // List of measurement ranges and the measurement container targeted by the ranges.
   template <typename T_MeasurementContainerList >
   class GenMeasurementRangeList
   {
   public:
      using MeasurementContainer = typename T_MeasurementContainerList::measurement_container_variant_t;
   private:
      T_MeasurementContainerList m_measurementContainerList;
      std::array< unsigned int, static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)+1u > invalidContainerIndices() {
         std::array< unsigned int, static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)+1u > perDetectorContainerIndex;
         std::fill(perDetectorContainerIndex.begin(),perDetectorContainerIndex.end(), std::numeric_limits<unsigned int>::max());
         return perDetectorContainerIndex;
      }
      std::array< unsigned int, static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)+1u > m_perDetectorContainerIndex = invalidContainerIndices();
      std::array< std::span<const PhaseII::DataRange>, static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)+1u > m_perDetectorRanges{};
      const std::array< const InDet::SiDetectorElementStatus *,
                        static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)+1u> *m_detectorElementStatusPerDetectorType{};
      std::vector< std::vector<PhaseII::DataRange> > m_customRanges;

   public:
      void setRange(unsigned int detector_type_i, std::span<const PhaseII::DataRange> per_module_measurement_ranges, unsigned int container_index) {
         m_perDetectorRanges.at(detector_type_i)=per_module_measurement_ranges;
         m_perDetectorContainerIndex.at(detector_type_i)=container_index;
      }
      std::vector< std::vector<PhaseII::DataRange> > &customRanges() { return m_customRanges; }

      const std::vector< MeasurementContainer > &measurementContainerList() const { return  m_measurementContainerList.containerList(); }

      static const PhaseII::DataRange &at(const std::span<const PhaseII::DataRange> &perDetectorRanges, unsigned int id_hash) {
         if (id_hash>=perDetectorRanges.size()) {
            static const PhaseII::DataRange emptyRange{};
            return emptyRange;
            //throw std::range_error("Invalid id hash for per detector measurement ranges.");
         }
         else {
            return perDetectorRanges[id_hash];
         }
      }
      unsigned int getContainerIndex( unsigned int detector_type_i, [[maybe_unused]] unsigned int id_hash) const {
         unsigned int detector_container_index = m_perDetectorContainerIndex.at(detector_type_i);
         if (detector_container_index> numContainers()) {
            throw std::range_error("No container registered for detector type.");
         }
         return detector_container_index;
      }
      const PhaseII::DataRange &getRange(unsigned int detector_type_i, unsigned int id_hash) const {
         return at(m_perDetectorRanges.at(detector_type_i),id_hash);
      }
      
      std::tuple<const MeasurementContainer *, abstract_measurement_range_t, bool >
      getMeasurementRange(unsigned int detector_type_i, unsigned int id_hash) const {
         const PhaseII::DataRange &data_range = at(m_perDetectorRanges.at(detector_type_i),id_hash);
         bool measurement_expected=measurementExpected(detector_type_i, id_hash);
         abstract_measurement_range_t range= ( (measurement_expected)
                                              ? data_range
                                               : abstract_measurement_range_t(std::numeric_limits<unsigned int>::max(),
                                                                              0u,
                                                                              static_cast<unsigned int>(data_range.containerIndex())));
            
         return { !range.empty() ? &container(m_perDetectorContainerIndex[detector_type_i]) : nullptr,
                  std::move(range), false};
      }
      // set container, resizing if necessary. That is just in case we call addMeasurements out of order or not for 2 types of measurements
      void setContainer(unsigned int container_index, const xAOD::UncalibratedMeasurementContainer *container) {
         if (container) {
            // @TODO allow for container == nullprt ?
            m_measurementContainerList.setContainer(container_index, *container);
         }
      }
      std::size_t numContainers() const { return m_measurementContainerList.size(); }

      const MeasurementContainer &container(unsigned index) const { return m_measurementContainerList.at(index); }
      void setDetectorElementStatus(const std::array< const InDet::SiDetectorElementStatus *,
                                                      static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)+1u> &det_el_status_per_arr) {
         m_detectorElementStatusPerDetectorType=&det_el_status_per_arr;
      }
      bool measurementExpected(unsigned int detector_type_i, unsigned int id_hash) const {
         const InDet::SiDetectorElementStatus * det_el_status = (*m_detectorElementStatusPerDetectorType)[detector_type_i];
         return !det_el_status || det_el_status->isGood(id_hash);
      }
      const T_MeasurementContainerList &getFullContainerList() const {
         return m_measurementContainerList;
      }
   };

   // List of measurement ranges and the measurement container targeted by the ranges.
   template <typename T_MeasurementContainerList >
   class GenMeasurementRangeListFlat : public std::vector<std::pair<std::size_t, std::pair<abstract_measurement_range_t, unsigned int> > >
   {
   public:
      using MeasurementContainer = typename T_MeasurementContainerList::measurement_container_variant_t;
      using MeasurementRangeContainer = std::vector<std::pair<std::size_t, std::pair<abstract_measurement_range_t, unsigned int> >>;
      using MeasurementRangeContainer::MeasurementRangeContainer;
   private:
      const T_MeasurementContainerList *m_measurementContainerList{};
      const std::array< const InDet::SiDetectorElementStatus *,
                        static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)+1u> *m_detectorElementStatusPerDetectorType{};
      bool m_forced = true;
   public:
      void setContainerList(const T_MeasurementContainerList &container_list) {
         m_measurementContainerList = &container_list;
      }

      const std::vector< MeasurementContainer > &measurementContainerList() const { return  m_measurementContainerList->containerList(); }

      // // set container, resizing if necessary. That is just in case we call addMeasurements out of order or not for 2 types of measurements
      // void setContainer(unsigned int container_index, const xAOD::UncalibratedMeasurementContainer *container) {
      //    if (container) {
      //       // @TODO allow for container == nullprt ?
      //       m_measurementContainerList.setContainer(container_index, *container);
      //    }
      // }
      std::size_t numContainers() const { return m_measurementContainerList->size(); }

      const MeasurementContainer &container(unsigned index) const { return m_measurementContainerList->at(index); }

      static std::uint64_t makeKey(unsigned int detector_type_i, unsigned int id_hash) {
         return (static_cast<std::uint64_t>(detector_type_i)<<32) | id_hash;
      }
      std::tuple<const MeasurementContainer *, abstract_measurement_range_t, bool >
      getMeasurementRange(unsigned int detector_type_i, unsigned int id_hash) const {
         auto range_iter = find( makeKey(detector_type_i, id_hash));
         if (range_iter == end()) {
            return {nullptr, abstract_measurement_range_t{}, m_forced};
         }
         else {
            // @TODO find source container, 
            bool measurement_expected=measurementExpected(detector_type_i, id_hash);
            abstract_measurement_range_t range= ( (measurement_expected)
                                                  ? range_iter->second.first
                                                  : abstract_measurement_range_t(std::numeric_limits<unsigned int>::max(), 0u, 0u));
            assert( !measurement_expected || range.beginIndex() <= range.endIndex());
            // if surface marked as defect
            return { m_forced || measurement_expected ? &(container(range_iter->second.second)) : nullptr,
                     std::move(range), m_forced};
         }
      }
      // required std::unordered_map methods compatible with GenMeasurementRangeList
      MeasurementRangeContainer::const_iterator find(const MeasurementRangeContainer::value_type::first_type &key) const {
        return std::find_if(begin(), end(),
                            [&key](const auto &c) {
                              return c.first == key;
                            });
      }

      std::pair<MeasurementRangeContainer::iterator, bool> insert(std::pair<std::size_t, std::pair<abstract_measurement_range_t, unsigned int> > && value) {
        emplace_back(std::move(value));
        return {std::prev(end()), true};
      }
      void setDetectorElementStatus(const std::array< const InDet::SiDetectorElementStatus *,
                                    static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)+1u> &det_el_status_per_arr) {
         m_detectorElementStatusPerDetectorType=&det_el_status_per_arr;
      }
      bool measurementExpected(unsigned int detector_type_i, unsigned int id_hash) const {
         const InDet::SiDetectorElementStatus * det_el_status = (*m_detectorElementStatusPerDetectorType)[detector_type_i];
         return !det_el_status || det_el_status->isGood(id_hash);
      }
   };

  /// Accessor for the above source link container
  ///
  /// It wraps up a few lookup methods to be used in the Combinatorial Kalman
  /// Filter
  template <typename T_MeasurementRangeList>
  class GenUncalibSourceLinkAccessor
  {
  private:
    const T_MeasurementRangeList *m_measurementRanges;

  public:
    using MeasurementContainer = typename T_MeasurementRangeList::MeasurementContainer;
    class BaseIterator
    {
    public:
       BaseIterator(const std::vector< MeasurementContainer > *containerList,
                    std::uint16_t container_index,
                    std::uint16_t cache_index,
                    unsigned int element_index)
          : m_containerList(containerList),
            m_containerIndex(container_index),
            m_cacheIndex(cache_index),
            m_index(element_index)
      {
      }
      BaseIterator &operator++()
      {
        ++m_index;
        return *this;
      }
      bool operator==(const BaseIterator &a) const { return m_index == a.m_index && m_containerIndex == a.m_containerIndex; }

      Acts::SourceLink operator*() const
      {
         // @TODO avoid double indirection
         const xAOD::UncalibratedMeasurement *interface_object
            = std::visit([cacheIndex=m_cacheIndex, index=m_index](const auto &a) -> const xAOD::UncalibratedMeasurement *{
               if constexpr(has_interfaceObject<std::remove_cvref_t<decltype(*a.containerPtr())> >) {
                  assert( index < a.containerPtr()->selection().size());
                  
                  return  a.containerPtr()->getInterfaceObject(cacheIndex, index);
               }
               else {
                  assert( index < a.containerPtr()->size());
                  return  (*a.containerPtr())[index];
               }
            },
               (*m_containerList)[m_containerIndex] );
         return Acts::SourceLink{ makeATLASUncalibSourceLink( interface_object )};
      }

      const std::vector< MeasurementContainer > &measurementContainerList() const { return *m_containerList; }
      std::uint16_t containerIndex() const { return m_containerIndex; }
      std::uint16_t cacheIndex() const { return m_cacheIndex; }
      unsigned int index() const { return m_index; }

      using value_type = unsigned int;
      using difference_type = unsigned int;
      using pointer = const xAOD::UncalibratedMeasurementContainer **;
      using reference = const xAOD::UncalibratedMeasurementContainer *;
      using iterator_category = std::input_iterator_tag;

    private:
       const std::vector< MeasurementContainer > *m_containerList;
       std::uint16_t m_containerIndex;
       std::uint16_t m_cacheIndex;
       unsigned int m_index;
    };

    using Iterator = Acts::SourceLinkAdapterIterator<BaseIterator>;
    GenUncalibSourceLinkAccessor(const T_MeasurementRangeList &measurement_ranges)
        : m_measurementRanges(&measurement_ranges)
    {
    }
    // get the range of elements with requested geoId
    std::pair<Iterator, Iterator> range(const Acts::Surface &surface) const
    {
       const Acts::SurfacePlacementBase* detector_element = surface.surfacePlacement();
       const ActsDetectorElement *acts_detector_element = detector_element ? static_cast<const ActsDetectorElement*>(detector_element) : nullptr;
       if (acts_detector_element) {
          unsigned int detector_type_i =static_cast<unsigned int>(acts_detector_element->detectorType());
          unsigned int id_hash = acts_detector_element->identifyHash();

          unsigned int container_index = m_measurementRanges->getContainerIndex(detector_type_i, id_hash);
          if (container_index< m_measurementRanges->numContainers()) {
             const PhaseII::DataRange &range = m_measurementRanges->getRange(detector_type_i, id_hash);
             return {Iterator(BaseIterator(&measurementContainerList(),
                                           container_index,
                                           /*cache index: */ range.containerIndex(),
                                           range.beginIndex())),
                     Iterator(BaseIterator(&measurementContainerList(),
                                           container_index,
                                           /*cache index: */ range.containerIndex(),
                                           range.endIndex()))};
          }
       }
       constexpr std::uint16_t zero_us=0;
       return {Iterator(BaseIterator(nullptr, zero_us, zero_us, 0u)),
               Iterator(BaseIterator(nullptr, zero_us, zero_us, 0u))};
    }
    const MeasurementContainer &container(unsigned index) const { return m_measurementRanges->container(index); }
    const std::vector< MeasurementContainer > &measurementContainerList() const { return  m_measurementRanges->measurementContainerList(); }

  };

  class AtlasMeasurementContainerList : public AuxDataCacheList<AtlasMeasurementContainerList>
  {
  public:
     using BASE = AuxDataCacheList<AtlasMeasurementContainerList>;
     using BASE::BASE;
     ~AtlasMeasurementContainerList();

     template <std::size_t DIM>
     static bool isDimension(const SG::AuxVectorBase &container) {
         static const xAOD::PosAccessor<DIM> acc{"localPositionDim" + std::to_string(DIM)};
         return container.isAvailable(acc.auxid());
     }

     // to support 2D and 3D pixel measurements
     // @note to support 3D pixel measurements, still need to add ContainerRefWithDim<xAOD::PixelClusterContainer,3> as
     //       template paramter to MeasurementContainerListWithDimension
     unsigned int getDimension(const xAOD::PixelClusterContainer &container) {
        if (isDimension<2>(container.auxbase())) { return 2u; }
        else if (isDimension<3>(container.auxbase())) { return 3u; }
        else {
           throw std::runtime_error("Unsupported dimension for PixelClusterContainer");
        }
     }
  };

  using MeasurementRangeList = GenMeasurementRangeList< AtlasMeasurementContainerList >;
  using MeasurementRangeListFlat = GenMeasurementRangeListFlat< AtlasMeasurementContainerList >;
  using UncalibSourceLinkAccessor = GenUncalibSourceLinkAccessor< MeasurementRangeList >;

}

#endif
