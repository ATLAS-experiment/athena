#ifndef AUXDATACACHELIST_H
#define AUXDATACACHELIST_H

#include "src/detail/MeasurementContainerWithDimension.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxDataCacheCollection.h"
#include "xAODInDetMeasurement/StripClusterAuxDataCacheCollection.h"
//#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"

#include <variant>
#include <memory> 
template<Utils::AccessPolicy accessPolicy> struct PixelClusterAuxDataCache;
template<Utils::AccessPolicy accessPolicy> struct StripClusterAuxDataCache;



namespace ActsTrk::detail {
template <typename T_AuxDataCache>
struct ClusterAuxDataCacheWithClusterAccess;
using PixelClusterAuxDataCacheProxy =  ClusterAuxDataCacheWithClusterAccess<PixelClusterAuxDataCacheCollection >;
using StripClusterAuxDataCacheProxy =  ClusterAuxDataCacheWithClusterAccess<StripClusterAuxDataCacheCollection >;
}

//using AuxDataCaches = std::variant< std::unique_ptr<PixelClusterAuxDataCacheProxy >, std::unique_ptr<StripClusterAuxDataCacheProxy >  >;

namespace ActsTrk::detail {
template <class derived_t>
class AuxDataCacheList : protected MeasurementContainerListWithDimension<derived_t,
                                                                      ContainerRefWithDim<PixelClusterAuxDataCacheProxy,2>,
                                                                         //ContainerRefWithDim<xAOD::StripClusterContainer,1>,
                                                                         ContainerRefWithDim<StripClusterAuxDataCacheProxy,1>,
                                                                         ContainerRefWithDim<xAOD::HGTDClusterContainer,3> >  {
public:
   using BASE=MeasurementContainerListWithDimension<derived_t,
                                                    ContainerRefWithDim<PixelClusterAuxDataCacheProxy,2>,
                                                    // ContainerRefWithDim<xAOD::StripClusterContainer,1>,
                                                    ContainerRefWithDim<StripClusterAuxDataCacheProxy,1>,
                                                    ContainerRefWithDim<xAOD::HGTDClusterContainer,3> >;
   using measurement_container_variant_t = BASE::measurement_container_variant_t;
   AuxDataCacheList();
   
   ~AuxDataCacheList();


   static constexpr std::size_t getMeasurementDimMax() {
      return BASE::dimMax();
   }
   static void dumpVariantTypes(std::ostream &out) {
      BASE::dumpVariantTypes(out);
   }

   std::size_t size() const {
      return BASE::size();
   }

   const typename BASE::measurement_container_variant_t &at(std::size_t container_index) const {
      return BASE::at(container_index);
   }
   const std::vector< typename BASE::measurement_container_variant_t > &containerList() const { return BASE::containerList();}

   void setContainer(std::size_t container_index, const xAOD::UncalibratedMeasurementContainer &container);

private:
   template <typename T_MeasurementContainer, typename T_AuxDataCache>
   void setContainerCreateCache(std::size_t container_index, const xAOD::UncalibratedMeasurementContainer &container);
   
   std::vector<std::variant<std::unique_ptr<PixelClusterAuxDataCacheProxy>,
                            std::unique_ptr<StripClusterAuxDataCacheProxy>
                                            > > m_caches; 
};
}
#endif
