/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ATLASUNCALIBSROUCELINACCESOR_H
#define ATLASUNCALIBSROUCELINACCESOR_H

#include "ActsGeometry/ATLASSourceLink.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"


// #include "MakeDerivedVariant.h"
#include "MeasurementContainerWithDimension.h"
#include <variant>
#include <vector>
#include <utility>

namespace ActsTrk {
// Helper class to describe ranges of measurements
// the range provides the measurement collection index and  element index range (begin, end)
  struct MeasurementRange : public std::pair<unsigned int, unsigned int>
  {
    MeasurementRange() : std::pair<unsigned int, unsigned int>(std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max()) {}
    static constexpr unsigned int CONTAINER_IDX_SHIFT = 28;
    static constexpr unsigned int CONTAINER_IDX_MASK = (1u << 31) | (1u << 30) | (1u << 29) | (1u << 28);
    static constexpr unsigned int ELEMENT_IDX_MASK = ~CONTAINER_IDX_MASK;
    static unsigned int createRangeValue(unsigned int container_idx, unsigned int index)
    {
      assert(container_idx < (1u << (32 - CONTAINER_IDX_SHIFT)));
      assert((index & CONTAINER_IDX_MASK) == 0u);
      return (container_idx << CONTAINER_IDX_SHIFT) | index;
    }
    void setRangeBegin(std::size_t container_idx, unsigned int element_idx)
    {
      assert(container_idx < (1u << (32 - CONTAINER_IDX_SHIFT)));
      this->first = MeasurementRange::createRangeValue(container_idx, element_idx);
    }
    void setRangeEnd(std::size_t container_idx, unsigned int element_idx)
    {
      this->second = MeasurementRange::createRangeValue(container_idx, element_idx);
    }
    unsigned int containerIndex() const
    {
      assert((this->first & CONTAINER_IDX_MASK) == (this->second & CONTAINER_IDX_MASK));
      return (this->first & CONTAINER_IDX_MASK) >> CONTAINER_IDX_SHIFT;
    }
    unsigned int elementBeginIndex() const
    {
      assert((this->first & CONTAINER_IDX_MASK) == (this->second & CONTAINER_IDX_MASK));
      return this->first & ELEMENT_IDX_MASK;
    }
    unsigned int elementEndIndex() const
    {
      assert((this->first & CONTAINER_IDX_MASK) == (this->second & CONTAINER_IDX_MASK));
      return this->second & ELEMENT_IDX_MASK;
    }
    bool empty() const { return this->first == this->second; }
  };

   // List of measurement ranges and the measurement container targeted by the ranges.
   template <typename T_MeasurementContainerList >
   class GenMeasurementRangeList : public std::vector<MeasurementRange>
   {
   public:
      using MeasurementContainer = typename T_MeasurementContainerList::measurement_container_variant_t;
   private:
      T_MeasurementContainerList m_measurementContainerList;

   public:

      const std::vector< MeasurementContainer > &measurementContainerList() const { return  m_measurementContainerList.containerList(); }

      // set container, resizing if necessary. That is just in case we call addMeasurements out of order or not for 2 types of measurements
      void setContainer(unsigned int container_index, const xAOD::UncalibratedMeasurementContainer *container) {
         if (container) {
            // @TODO allow for container == nullprt ?
            m_measurementContainerList.setContainer(container_index, *container);
         }
      }
      std::size_t numContainers() const { return m_measurementContainerList.size(); }

      const MeasurementContainer &container(unsigned index) const { return m_measurementContainerList.at(index); }
   };

  /// Accessor for the above source link container
  ///
  /// It wraps up a few lookup methods to be used in the Combinatorial Kalman
  /// Filter
  template <typename T_MeasurementRangeList>
  class GenUncalibSourceLinkAccessor
  {
  private:
    const std::vector<Acts::GeometryIdentifier> *m_orderedGeoIds;
    const T_MeasurementRangeList *m_measurementRanges;

  public:
    using MeasurementContainer = typename T_MeasurementRangeList::MeasurementContainer;
    class BaseIterator
    {
    public:
       BaseIterator(const std::vector< MeasurementContainer > *containerList,
                    unsigned int container_index,
                    unsigned int element_index)
          : m_containerList(containerList),
            m_containerIndex(container_index),
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
         const xAOD::UncalibratedMeasurementContainer *base_container
            = std::visit([](const auto &a) -> const xAOD::UncalibratedMeasurementContainer *{return  a.containerPtr(); },
                         (*m_containerList)[m_containerIndex] );
         assert( m_index < base_container->size());
         return Acts::SourceLink{ makeATLASUncalibSourceLink( (*base_container)[m_index] )};
      }

      const std::vector< MeasurementContainer > &measurementContainerList() const { return *m_containerList; }
      unsigned int containerIndex() const { return m_containerIndex; }
      unsigned int index() const { return m_index; }

      using value_type = unsigned int;
      using difference_type = unsigned int;
      using pointer = const xAOD::UncalibratedMeasurementContainer **;
      using reference = const xAOD::UncalibratedMeasurementContainer *;
      using iterator_category = std::input_iterator_tag;

    private:
       const std::vector< MeasurementContainer > *m_containerList;
       unsigned int m_containerIndex;
       unsigned int m_index;
    };

    using Iterator = Acts::SourceLinkAdapterIterator<BaseIterator>;
    GenUncalibSourceLinkAccessor(const std::vector<Acts::GeometryIdentifier> &ordered_geoIds,
                              const T_MeasurementRangeList &measurement_ranges)
        : m_orderedGeoIds(&ordered_geoIds),
          m_measurementRanges(&measurement_ranges)
    {
    }
    // get the range of elements with requested geoId
    std::pair<Iterator, Iterator> range(const Acts::Surface &surface) const
    {
      std::vector<Acts::GeometryIdentifier>::const_iterator
          geo_iter = std::lower_bound(m_orderedGeoIds->begin(), m_orderedGeoIds->end(), surface.geometryId());
      if (geo_iter == m_orderedGeoIds->end() || *geo_iter != surface.geometryId() || (*m_measurementRanges).at(geo_iter - m_orderedGeoIds->begin()).empty())
      {
        return {Iterator(BaseIterator(nullptr, 0u, 0u)),
                Iterator(BaseIterator(nullptr, 0u, 0u))};
      }

      assert(static_cast<std::size_t>(geo_iter - m_orderedGeoIds->begin()) < m_measurementRanges->size());
      const MeasurementRange &range = (*m_measurementRanges).at(geo_iter - m_orderedGeoIds->begin());
      // const xAOD::UncalibratedMeasurementContainer *container
      //    = std::visit( [](const auto &a) -> const xAOD::UncalibratedMeasurementContainer * { return a},
      //                  m_measurementRanges->container(range.containerIndex()));
      return {Iterator(BaseIterator(&measurementContainerList(), range.containerIndex(), range.elementBeginIndex())),
              Iterator(BaseIterator(&measurementContainerList(), range.containerIndex(), range.elementEndIndex()))};
    }
    const MeasurementContainer &container(unsigned index) const { return m_measurementRanges->container(index); }
    const std::vector< MeasurementContainer > &measurementContainerList() const { return  m_measurementRanges->measurementContainerList(); }

  };

  class AtlasMeasurementContainerList : public MeasurementContainerListWithDimension< AtlasMeasurementContainerList,
                                                                                      ContainerRefWithDim<xAOD::PixelClusterContainer,2>,
                                                                                      ContainerRefWithDim<xAOD::StripClusterContainer,1> >
  {
  public:

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
  using UncalibSourceLinkAccessor = GenUncalibSourceLinkAccessor< MeasurementRangeList >;

}
#endif
