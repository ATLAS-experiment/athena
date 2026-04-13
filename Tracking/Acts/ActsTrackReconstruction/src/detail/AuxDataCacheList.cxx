#include "AuxDataCacheList.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxDataCacheCollection.h"
#include "xAODInDetMeasurement/StripClusterAuxDataCacheCollection.h"

#include "AtlasUncalibSourceLinkAccessor.h"
#include "AuxDataCacheList.icc"

#include  <stdexcept>

namespace ActsTrk::detail {

   template <class derived_t>
   AuxDataCacheList<derived_t>::AuxDataCacheList() {}
   
   template <class derived_t>
   AuxDataCacheList<derived_t>::~AuxDataCacheList() {}

   template <class derived_t>
   template <typename T_MeasurementContainer, typename T_AuxDataCache>
   void AuxDataCacheList<derived_t>::setContainerCreateCache(std::size_t container_index, const xAOD::UncalibratedMeasurementContainer &container) {
      assert( dynamic_cast<const T_MeasurementContainer *>(&container) == static_cast<const T_MeasurementContainer *>(&container));
      auto cache = std::make_unique<ClusterAuxDataCacheWithClusterAccess<T_AuxDataCache> >( static_cast<const T_MeasurementContainer &>(container));
      BASE::setContainer(container_index, *cache);
      m_caches.emplace_back(std::move(cache));
   }

   template <class derived_t>
   void AuxDataCacheList<derived_t>::setContainer(std::size_t container_index, const xAOD::UncalibratedMeasurementContainer &container) {
      if (dynamic_cast<const xAOD::PixelClusterContainerAlt*>(&container) != nullptr) {
         AuxDataCacheList<derived_t>::setContainerCreateCache<xAOD::PixelClusterContainerAlt,
                                                              PixelClusterAuxDataCacheCollection >(container_index,container);
      }
      else if (dynamic_cast<const xAOD::StripClusterContainerAlt*>(&container) != nullptr) {
          AuxDataCacheList<derived_t>::setContainerCreateCache<xAOD::StripClusterContainerAlt,
                                                               StripClusterAuxDataCacheCollection >(container_index,container);
      }
      // else if (dynamic_cast<const xAOD::StripClusterContainerAlt*>(&container) != nullptr) {
      //    assert( dynamic_cast<const xAOD::StripClusterContainerAlt *>(&container) == static_cast<const xAOD::StripClusterContainerAlt *>(&container));
      //    BASE::setContainer(container_index, *static_cast<const xAOD::StripClusterContainerAlt *>(&container));
      // }
      else if (dynamic_cast<const xAOD::HGTDClusterContainer*>(&container) != nullptr) {
         assert( dynamic_cast<const xAOD::HGTDClusterContainer *>(&container) == static_cast<const xAOD::HGTDClusterContainer *>(&container));
         BASE::setContainer(container_index, *static_cast<const xAOD::HGTDClusterContainer *>(&container));
      }
      else {
         throw std::runtime_error(std::string("Unsupported measurement type ") + typeid(container).name());
      }
   }

   template class AuxDataCacheList<ActsTrk::detail::AtlasMeasurementContainerList>;
}

