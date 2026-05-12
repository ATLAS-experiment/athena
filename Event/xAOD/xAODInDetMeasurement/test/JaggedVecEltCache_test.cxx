/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include <xAODInDetMeasurement/StripClusterContainer.h>
#include <xAODInDetMeasurement/StripClusterAuxContainer.h>
#include "xAODCore/VariableStruct.h"
#include "xAODInDetMeasurement/JaggedVecEltCache.h"
#include <iostream>
#include <cassert>
#include <random>
#include <algorithm>
#include <utility>


template <typename T_Container>
int compareRDOList(const T_Container &a, const T_Container &b) {
   assert (a.size() == b.size());
   for (unsigned int cluster_i=0; cluster_i<a.size(); ++cluster_i) {
      auto rdo_list_a = a[cluster_i]->rdoList();
      auto rdo_list_b = b[cluster_i]->rdoList();
      assert (rdo_list_a.size() == rdo_list_b.size());
      for (unsigned int rdo_i=0; rdo_i<rdo_list_a.size(); ++rdo_i) {
         assert( rdo_list_a[rdo_i] == rdo_list_b[rdo_i]);
      }
   }
   return 0;
}


struct StripAuxDataCache : xAOD::VariableStruct {
   StripAuxDataCache(SG::AuxVectorData& cont, unsigned int n_cluster_rdos)
      : xAOD::VariableStruct(cont),
        rdoList(cont, xAOD::StripCluster::rdoListAcc(), n_cluster_rdos)
   {}
   xAOD::xAODInDetMeasurement::Utilities::JaggedVecEltCache<Identifier::value_type> rdoList;
};

void testFillStripClusterContainer(xAOD::StripClusterContainer &strip_cluster_container, std::vector<std::vector<Identifier::value_type> > &rdo_lists) {
  for (std::vector<Identifier::value_type> &rdos : rdo_lists) {
     strip_cluster_container.push_back(new xAOD::StripCluster);
     strip_cluster_container.back()->setRDOlist( std::span(rdos.begin(),rdos.end()) );
  }
}
void testFillStripClusterContainerBypass(xAOD::StripClusterContainer &strip_cluster_container, const std::vector<std::vector<Identifier::value_type> > &rdo_lists) {
  strip_cluster_container.push_new(rdo_lists.size(), []() {return new xAOD::StripCluster;});
  unsigned int n_rdos=std::accumulate(rdo_lists.begin(),rdo_lists.end(),0u,[](unsigned int sum, const std::vector<Identifier::value_type> &rdos) {
     return sum + rdos.size();
  });
  StripAuxDataCache cache(strip_cluster_container,n_rdos);
  unsigned int cluster_i=0;
  --cluster_i;
  for (const std::vector<Identifier::value_type> &rdos : rdo_lists) {
     ++cluster_i;
     unsigned int rdo_i = cluster_i>0 ? cache.rdoList.getBeginIndex(cluster_i) : 0u;
     for (const Identifier::value_type &rdo : rdos) {
        cache.rdoList.setValue(rdo_i,rdo);
        ++rdo_i;
     }
     cache.rdoList.updateEndIndex(cluster_i,rdo_i);
  }
  assert( cluster_i+1 == strip_cluster_container.size());
  assert( cluster_i>0u && cache.rdoList.getEndIndex(cluster_i) == n_rdos);
  assert( cache.rdoList.nElements() == n_rdos );
  assert( cache.rdoList.nObjects() == strip_cluster_container.size() );
}

int test_jaggedVecSetBypass()
{
   std::vector<std::vector<Identifier::value_type> > rdo_lists;
   for (unsigned int i=0; i<10; ++i) {
      rdo_lists.emplace_back();
      for (unsigned int j=0; j<i; ++j) {
         rdo_lists.back().push_back( ((i+1u)<<16)| (j+1u) );
      }
   }
   std::random_device rd;
   std::mt19937 g(rd());
   std::shuffle(rdo_lists.begin(), rdo_lists.end(), g);

   // fill strip cluster container using interface objects and accessor
   xAOD::StripClusterContainer strip_cluster_container;
   xAOD::StripClusterAuxContainer strip_cluster_container_aux;
   strip_cluster_container.setStore(&strip_cluster_container_aux);
   testFillStripClusterContainer(strip_cluster_container,rdo_lists);
   // fill rdos bypassing the accessor
   xAOD::StripClusterContainer strip_cluster_container_alt;
   xAOD::StripClusterAuxContainer strip_cluster_container_alt_aux;
   strip_cluster_container_alt.setStore(&strip_cluster_container_alt_aux);
   testFillStripClusterContainerBypass(strip_cluster_container_alt,rdo_lists);

   return compareRDOList(strip_cluster_container, strip_cluster_container_alt);
}

int main ()
{
  return test_jaggedVecSetBypass();
}
