/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#undef NDEBUG
#include <atomic>
#include <array>
#include <vector>
#include <cassert>
#include <iostream>
#include <limits>
#include <cstdint>
#include <type_traits>
#include <span>

#include "InDetRawData/PhaseIIPixelRawDataContainerMT.h"

#include <memory>
#include <algorithm>
#include <cstring>
#include <thread>
#include "FNVHash.h"

// create toy ROIs i.e. collections of unique random module indices.
std::vector< std::vector< unsigned int> > makeROIs( unsigned int max_modules, unsigned int max_ROIs, unsigned int max_modules_per_ROI ) {
   std::vector< std::vector< unsigned int> >  rois;
   rois.resize( max_ROIs );
   for ( std::vector<unsigned int> &roi : rois ) {
      //coverity[DC.WEAK_CRYPTO]
      unsigned int n_modules = rand() % max_modules_per_ROI;
      roi.reserve(n_modules);
      for (unsigned int module_i=0; module_i<n_modules; ++module_i) {
         //coverity[DC.WEAK_CRYPTO]
         unsigned int a_module =  rand() % max_modules;
         if (std::find(roi.begin(), roi.end(), a_module) == roi.end()) {
            roi.push_back(a_module);
         }
      }
   }
   return rois;
}

// create from toy ROIs, non overlapping "ROIs"
// create from collections of unique module indices non-overlapping collections of unique module indices
// i.e. each module index is at most in one of the collections.
std::vector<std::vector<unsigned int> > removeOverlap(unsigned int max_modules, const std::vector<std::vector<unsigned int> > &rois) {
   std::vector<std::vector<unsigned int> > rois_out;
   rois_out.resize(rois.size());
   std::vector<bool> used(max_modules,false);
   unsigned int roi_i=0;
   for (const std::vector<unsigned int> &an_roi : rois ) {
      std::vector<unsigned int> &dest = rois_out[roi_i];
      dest.reserve( an_roi.size());
      for (unsigned int module_i : an_roi) {
         if (!used.at(module_i)) {
            used[module_i]=true;
            dest.push_back(module_i);
         }
      }
      ++roi_i;
   }
   return rois_out;
}

// create from non overlapping ROIs a simple association which associates a module index to the ROI index which contains the module index
std::vector<std::pair<unsigned int, unsigned int> > findContainer(unsigned int max_modules, const std::vector<std::vector<unsigned int> > &rois) {
   std::vector<std::pair<unsigned int,unsigned int> > used_modules;
   std::vector<unsigned int> container_index(max_modules, std::numeric_limits<unsigned int>::max());
   unsigned int n_modules_used=0;
   unsigned int roi_i=0;
   for (const std::vector<unsigned int> &an_roi : rois ) {
      for (unsigned int module_i : an_roi) {
         if (container_index[module_i]==std::numeric_limits<unsigned int>::max()) {
            container_index[module_i]=roi_i;
            ++n_modules_used;
         }
      }
      ++roi_i;
   }
   used_modules.reserve(n_modules_used);
   unsigned int module_i=0;
   for(unsigned int container_i : container_index ) {
      if (container_index[module_i]!=std::numeric_limits<unsigned int>::max()) {
         used_modules.push_back(std::make_pair(module_i, container_i) );
      }
      ++module_i;
   }

   return used_modules;
}

// create a new pixel hit container for at most max_modules which can be distributed over at most container_list_size
// independent data containers
std::unique_ptr< PhaseIIPixelRawDataContainer>  createPixelRawDataContainer(unsigned int max_modules,
                                                                            unsigned int container_list_size) {
   std::unique_ptr< PhaseIIPixelRawDataContainer> ret = std::make_unique< PhaseIIPixelRawDataContainer>(max_modules,
                                                                                                        container_list_size);
   return ret;
}

// create toy pixel hit data for the given set of modules, where at most max_hits_per_module are added per module
// the hits will get coordinates on a matrix n_cols x n_rows, where no attempt is made to avoid hit overlaps
// the hit data will be stored in the hit data container of the specified container.
void fillPixelRawDataContainer ( unsigned int container_i,
                        std::span<unsigned int> modules,
                        unsigned int max_hits_per_module,
                        unsigned int n_cols,
                        unsigned int n_rows,
                        PhaseIIPixelRawDataContainer &container) {
   [[maybe_unused]] unsigned int n_rejected_ranges=0u;

   // get the actual hit data container for the given container index
   PhaseII::PixelRawDataContainer &rdo_data = container.data(container_i);
   rdo_data.reserve( max_hits_per_module * modules.size());
   for (unsigned int module : modules) {

      // test that the no hit data was registered for this module
      PhaseII::DataRange range = container.range(module);
      if (!range.empty()) continue;

      // throw number of hits to be generated for this module
      //coverity[DC.WEAK_CRYPTO]
      unsigned int n_rdos = rand() % max_hits_per_module;
      if (n_rdos>0) {
         // reserve storage and initialize range data pointing to the first element to be used for this module
         // the range is empty initially.
         rdo_data.reserve( rdo_data.size() + n_rdos);
         using RangeSize_t = PhaseII::DataRange::RangeSize_t;
         using ContainerIndex_t = PhaseII::DataRange::ContainerIndex_t;
         PhaseII::DataRange new_range( rdo_data.size(),
                              static_cast<RangeSize_t>( 0u) ,
                              static_cast<ContainerIndex_t>(container_i));

         // fill the random hit data
         for (unsigned int rdo_i=0; rdo_i< n_rdos; ++rdo_i) {
            //coverity[DC.WEAK_CRYPTO]
            unsigned int coordinate_pair = rand();
            //coverity[DC.WEAK_CRYPTO]
            unsigned int rand_data_word = rand();
            rdo_data.emplace_back(std::array<std::int16_t, 2>{ static_cast<std::int16_t>((coordinate_pair >>16) % n_cols),
                                                               static_cast<std::int16_t>((coordinate_pair) % n_rows) },
                                  PhaseII::PixelRawDataContainer::makeWord( PhaseII::PixelRawDataContainer::getToT(rand_data_word ),
                                                                   PhaseII::PixelRawDataContainer::getBCID(rand_data_word ),
                                                                   PhaseII::PixelRawDataContainer::getLVL1A(rand_data_word ),
                                                                   PhaseII::PixelRawDataContainer::getLVL1ID(rand_data_word )));

         }
         assert( rdo_data.size() >= new_range.beginIndex()
                 && static_cast<std::size_t>(rdo_data.size() - new_range.beginIndex()) < std::numeric_limits<RangeSize_t>::max());

         // update range to the final number of elements
         new_range.setSize( static_cast<RangeSize_t>( rdo_data.size() - new_range.beginIndex()) );
         // ... and register the new hit range, or if somehow a range was already registered in the mean time
         // erase the hit data which was just added here to the end of the container.
         static_assert( std::is_same_v<PhaseIIPixelRawDataContainer::T_RangeTypeBase, PhaseII::DataRange>);
         n_rejected_ranges += !(container.registerOrEraseNewData( module, new_range));
      }
   }
   assert( n_rejected_ranges==0);
}

// dump the contents of the entire hit container collection
void dump(const PhaseIIPixelRawDataContainer &rdo_container) {
   // first create a proxy representing the entire hit data collection.
   auto rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(rdo_container);
   using PixelRawDataContainerProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataContainerProxy;
   using PixelRawDataProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataProxy;
   // this proxy can be used to iterate over the modules
   for (PixelRawDataContainerProxy module_rdo_container_proxy : rdo_container_collection_proxy) {
      // Each of the proxy representing the hits of a module can be used to
      for (PixelRawDataProxy rdo_proxy : module_rdo_container_proxy) {
         std::cout << module_rdo_container_proxy.index().rangeIndex() << " :";
         for (auto elm :  rdo_proxy.coordinates()) {
            std::cout << " " << elm;
         }
         std::cout << std::endl;
      }
   }
}

// test: proxy read write to read only conversion; module proxy adapter; child index computation.
void conversionTest(PhaseIIPixelRawDataContainer &rdo_container ) {
   if (rdo_container.empty()) return;

   auto rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(rdo_container);
   using PixelRawDataContainerProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataContainerProxy;
   using PixelRawDataContainerProxyRW = PhaseII::PixelRawDataContainerCollectionTypes<Utils::AccessPolicy::ReadWrite>::RawDataContainerProxy;
   using PixelRawDataProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataProxy;
   using PixelRawDataProxyRW = PhaseII::PixelRawDataContainerCollectionTypes<Utils::AccessPolicy::ReadWrite>::RawDataProxy;

   static_assert( Utils::isConvertableToReadOnlyProxy<PixelRawDataProxy,PixelRawDataProxyRW> );

   {
      // test proxy conversion and index recovery
      auto a_module_proxy = rdo_container_collection_proxy[0];
      static_assert( std::is_same_v<decltype(a_module_proxy), PixelRawDataContainerProxy> || std::is_same_v<decltype(a_module_proxy), PixelRawDataContainerProxyRW>);
#ifndef NDEBUG
      auto a_module_proxy_back = rdo_container_collection_proxy.back();

      // get the index which can be used as index for the access operator [] of the parent proxy
      std::size_t index0 = rdo_container_collection_proxy.computeChildElementIndex(a_module_proxy);
      std::size_t back_index = rdo_container_collection_proxy.computeChildElementIndex(a_module_proxy_back);

      // test that these indices indeed recover the same element
      auto element0 = rdo_container_collection_proxy[index0];
      auto element_back = rdo_container_collection_proxy[back_index];
      assert( element0.index() == a_module_proxy.index() && &element0.container() == &a_module_proxy.container());
      assert( element_back.index() == a_module_proxy_back.index() && &element_back.container() == &a_module_proxy_back.container());
#endif
   }

   for (auto module_proxy : rdo_container_collection_proxy) {
      if (!module_proxy.empty()) {
         auto pixel_proxy = module_proxy[0];
         static_assert( std::is_same_v<decltype(pixel_proxy), PixelRawDataProxy> || std::is_same_v<decltype(pixel_proxy), PixelRawDataProxyRW>);

#ifndef NDEBUG
         auto pixel_proxy_back = module_proxy.back();

         // get the index which can be used as index for the access operator [] of the parent proxy
         std::size_t index0 = module_proxy.computeChildElementIndex(pixel_proxy);
         std::size_t back_index = module_proxy.computeChildElementIndex(pixel_proxy_back);

         // test that these indices indeed recover the same element
         auto element0 = module_proxy[index0];
         auto element_back = module_proxy[back_index];
         assert( element0.index() == pixel_proxy.index() && &element0.container() == &pixel_proxy.container());
         assert( element_back.index() == pixel_proxy_back.index() && &element_back.container() == &pixel_proxy_back.container());
#endif


         break;
      }
   }
}

// test iteration over all pixel hits using proxies
std::size_t test(const PhaseIIPixelRawDataContainer &rdo_container) {
   std::size_t sum{};
   auto rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(rdo_container);
   using PixelRawDataContainerProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataContainerProxy;
   using PixelRawDataProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataProxy;
   for (PixelRawDataContainerProxy module_rdo_container_proxy : rdo_container_collection_proxy) {
      for (PixelRawDataProxy rdo_proxy : module_rdo_container_proxy) {
         for (auto elm :  rdo_proxy.coordinates()) {
            sum+= elm;
         }
      }
   }
   return sum;
}

// test iteration over all pixel hits using read-write proxies
std::size_t testNonConst(PhaseIIPixelRawDataContainer &rdo_container) {
   std::size_t sum{};
   auto rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(rdo_container);
   using PixelRawDataContainerProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataContainerProxy;
   using PixelRawDataContainerProxyRW = PhaseII::PixelRawDataContainerCollectionTypes<Utils::AccessPolicy::ReadWrite>::RawDataContainerProxy;
   using PixelRawDataProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataProxy;
   using PixelRawDataProxyRW = PhaseII::PixelRawDataContainerCollectionTypes<Utils::AccessPolicy::ReadWrite>::RawDataProxy;

   auto a_module_proxy = rdo_container_collection_proxy[0];
   static_assert( std::is_same_v<decltype(a_module_proxy), PixelRawDataContainerProxy> || std::is_same_v<decltype(a_module_proxy), PixelRawDataContainerProxyRW>);

   static_assert( Utils::isConvertableToReadOnlyProxy<PixelRawDataProxy,PixelRawDataProxyRW> );
   for (auto module_rdo_container_proxy : rdo_container_collection_proxy) {
      for (auto rdo_proxy : module_rdo_container_proxy) {
         // just to check that the proxy allows to modify the data.
         [[maybe_unused]] std::array<std::int16_t,2> &coordinates = rdo_proxy.coordinates();
         for (auto elm :  rdo_proxy.coordinates()) {
            sum+= elm;
         }
      }
   }
   return sum;
}


// alternative test which iterates explicitly over the hit data
std::size_t testAlt(const PhaseIIPixelRawDataContainer &rdo_container) {
   std::size_t sum{};
   for (unsigned int module_i =0; module_i<rdo_container.size(); ++ module_i) {
      PhaseII::DataRange range = rdo_container.range(module_i);
      const PhaseII::PixelRawDataContainer &data = rdo_container.data(range.containerIndex());
      for (unsigned int rdo_i=range.beginIndex(); rdo_i < range.endIndex(); ++rdo_i) {
         for (auto elm :  data.coordinates(rdo_i)) {
            sum += elm;
         }
      }
   }
   return sum;
}

// count hits in the pixel hit container
std::size_t countHits(const PhaseIIPixelRawDataContainer &rdo_container) {
   std::size_t sum{};
   auto rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(rdo_container);
   using PixelRawDataContainerProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataContainerProxy;
   for (PixelRawDataContainerProxy module_rdo_container_proxy : rdo_container_collection_proxy) {
      sum += module_rdo_container_proxy.size();
   }
   return sum;
}

// count modules in the given non overlapping ROIs (or double count otherwise)
std::size_t countModules(const std::vector< std::vector< unsigned int> > &rois) {
   std::size_t sum{};
   for (const std::vector<unsigned int> &roi : rois) {
      sum += roi.size();
   }
   return sum;
}


// Test which fills one pixel hit container per ROI for several ROIs in one thread per ROI.
// The input data is read from a hit container.
// Meant to compare the performance wrt. to an implementation which only fills the ROI hit
// data of the various ROIs into a single pixel hit container.
std::size_t roiFillNonMT(const PhaseIIPixelRawDataContainer &rdo_container,
                         const std::vector<std::vector<unsigned int> > &rois,
                         const std::vector<std::pair<unsigned int, unsigned int> > &used_modules) {
   // container for which the Range is not atomic
   using PixelRawDataContainerNonMT = PhaseII::IndexedRanges<PhaseII::PixelRawDataContainer, PhaseII::DataRange >;

   // create one hit container per ROI (or thread) without an atomic range structure
   std::vector<PixelRawDataContainerNonMT> roi_rdo_container;
   roi_rdo_container.reserve(rois.size());
   for (unsigned int roi_i=0; roi_i< rois.size() ; ++roi_i) {
      roi_rdo_container.emplace_back(rdo_container.size(), 1);
   }

   // create one thread per ROI which fills the per thread hit container from the shared input hit container with the
   // hit data of that particular ROI.
   std::vector<std::thread> threads;
   threads.reserve(rois.size());
   unsigned int roi_i=0;
   std::atomic<unsigned int> n_container_changes=0u;
   for (const std::vector<unsigned int> &roi : rois) {
      // create a new thread
      threads.emplace_back( [&a_roi_rdo_container=roi_rdo_container[roi_i], &roi, &rdo_container, &n_container_changes] () {
         // the thread will iterate over the modules of a single ROI. For each ROI a separate container collection
         // is used, so all threads fill their containers completely independently of each other, and only
         // a single container will be used for all modules of an ROI.
         PhaseII::PixelRawDataContainer *rdo_container_dest = &a_roi_rdo_container.data(0);
         unsigned int n_container_changes_per_roi=0u;
         for (unsigned int module_i : roi ) {
            // get the hit range
            const PhaseII::DataRange &range = rdo_container.range(module_i);
            const PhaseII::PixelRawDataContainer &rdo_data_src = rdo_container.data(range.containerIndex());
            // iterate over the hit range  and copy the hit data into the per ROI output hit container
            // The range guard will keep track of the element range.
            PhaseII::ContainerRangeGuard<PhaseII::DataRange, PhaseII::PixelRawDataContainer *>
               dest_range_guard(rdo_container_dest);
            for (unsigned int rdo_idx = range.beginIndex(); rdo_idx <range.endIndex(); ++rdo_idx) {
               using CoordType = std::remove_cvref_t<decltype( rdo_data_src.coordinates(rdo_idx) )>;
               const auto orig_ptr=dest_range_guard.ptr();
               addDataForModule(a_roi_rdo_container,
                                dest_range_guard,
                                CoordType(rdo_data_src.coordinates(rdo_idx)),
                                rdo_data_src.dataWord(rdo_idx));
               n_container_changes_per_roi += (dest_range_guard.ptr() != orig_ptr);
            }
            if (!dest_range_guard.empty()) {
               // now register the new hit range. No range will have been registered for the module
               // so the just filled hit data is never erased.
               a_roi_rdo_container.registerOrEraseNewData(module_i,dest_range_guard.range());
               // the container will stay the same for all modules for the non MT container collection
               // but to remain consistent with the MT version the currently used pointer is
               // retrieved from the guard to ensure that the same container is used for the
               // next module.
               rdo_container_dest = dest_range_guard.ptr();
            }
         }
         n_container_changes += n_container_changes_per_roi;
      });
      ++roi_i;
   }
   // wait for all threads to finish
   for (std::thread &a_thread : threads) {
      a_thread.join();
   }
   // only a single container is used per ROI, i.e. the container will not change when processing the modules
   // of a single ROI:
   assert( n_container_changes == 0 );
   // compute a FNV hash from the coordinates of all hits of all modules of all ROIs, in the module index order
   // only using the data of each module at most once.
   FNVHash fnvHash;
   if (!rois.empty()) {
   for (auto [module_index, container_index]  : used_modules) {
      assert( container_index < roi_rdo_container.size());
      const PhaseII::DataRange &range = roi_rdo_container[container_index].range(module_index);
      const PhaseII::PixelRawDataContainer &data = roi_rdo_container[container_index].data( range.containerIndex());
      for (unsigned int rdo_idx = range.beginIndex(); rdo_idx <range.endIndex(); ++rdo_idx) {
         for (auto elm :  data.coordinates(rdo_idx)) {
            fnvHash.add(elm);
         }
      }
   }
   }

   // return the result FNV hash
   return fnvHash.value();
}


// The same test as above, but use a single container collection, which provides at least one container per ROI. The
// the module range is now atomic, and newly filled hit data will be erased if hit data was already
// registered for the module by a different thread, which may happen if the ROIs overlap.
std::size_t roiFillMT(const PhaseIIPixelRawDataContainer &rdo_container,
                      const std::vector<std::vector<unsigned int> > &rois) {
   static_assert( std::atomic<PhaseII::DataRange>::is_always_lock_free );
   std::atomic<unsigned int> n_rejected_work=0u;


   // to match work of nonMT version, unused containers are created per ROI
   using PixelRawDataContainerNonMT = PhaseII::IndexedRanges<PhaseII::PixelRawDataContainer, PhaseII::DataRange >;
   std::vector<PixelRawDataContainerNonMT> dummy;
   dummy.reserve(rois.size()-1); // one less because, also the real output container is created (below)
   for (unsigned int roi_i=0; roi_i< rois.size() ; ++roi_i) {
      dummy.emplace_back(rdo_container.size(), 1);
   }

   // create the actual output container
   // the first argument is the max wafer hash
   // the second argument the best estimate of the number processes which fill the container concurrently
   // i.e. 1 for offline reco, and O(50) for HLT.
   PhaseIIPixelRawDataContainerMT roi_rdo_container(rdo_container.size(), rois.size());

   // create one thread per ROI which fills the per thread hit container from the shared input hit container with the
   // hit data of that particular ROI.
   std::vector<std::thread> threads;
   threads.reserve(rois.size());
   std::atomic<unsigned int> n_container_changes=0;
   for (const std::vector<unsigned int> &roi : rois) {
      threads.emplace_back( [&a_roi_rdo_container=roi_rdo_container, &roi, &rdo_container, &n_rejected_work, &n_container_changes] () {
         // each thread will try to copy the input hit data of all modules of an ROI to the output hit container
         PhaseIIPixelRawDataContainerMT::ContainerPtr rdo_container_dest = a_roi_rdo_container.getNewContainerPtr();
         // estimate the number RDOs for this container (rdo_container_dest)
         std::size_t n_rdos_expected=0;
         unsigned int multiplier=1;
         for (unsigned int module_i : roi ) {
            const PhaseII::DataRange &range = rdo_container.range(module_i);
            n_rdos_expected+=range.size();
         }
         n_rdos_expected *= multiplier;

         // to test the situation in which the container capacity is exceeded, reserve fewer elements than
         // needed i.e. "n_rdos_expected/3" instead of "n_rdos_expected"
         rdo_container_dest->reserve(n_rdos_expected/3);

         unsigned int n_container_changes_per_roi=0u;
         for (unsigned int module_i : roi ) {
            // module_i is the ID hash

            // skip if there is already RDO data registered for this module
            if (!a_roi_rdo_container.range(module_i).load().empty()) continue;

            // the input range
            // @TODO should use the proxies for retrieving the input hit data
            const PhaseII::DataRange &range = rdo_container.range(module_i);
            const PhaseII::PixelRawDataContainer &rdo_data_src = rdo_container.data(range.containerIndex());

            // The range guard keeps track of the element range and the container which will
            // contain the hits for the current module. In the best case the container will be the
            // same as the container which is passed to the range guard below, but if the
            // container capacity is exceeded the container may change.
            PhaseII::ContainerRangeGuard<PhaseII::DataRange, PhaseIIPixelRawDataContainerMT::ContainerPtr>
               dest_range_guard(rdo_container_dest);
            for (unsigned int rdo_idx = range.beginIndex(); rdo_idx <range.endIndex(); ++rdo_idx) {
               using CoordType = std::remove_cvref_t<decltype( rdo_data_src.coordinates(rdo_idx) )>;
               // add coordinates and data word for one hit, and if the capacity of the RDO container
               // is exceeded create a new container, copy the already added data for the current module
               // to this new container, From this moment on, the data will be added to the new container.
               unsigned int is_id=dest_range_guard.ptr().containerId();
               addDataForModule(a_roi_rdo_container,
                                dest_range_guard,
                                CoordType(rdo_data_src.coordinates(rdo_idx)),
                                rdo_data_src.dataWord(rdo_idx));
               n_container_changes_per_roi += (dest_range_guard.ptr().containerId() != is_id);
            }
            if (!dest_range_guard.empty()) {
               // register the RDO range for this module, or erase the newly added
               if (!a_roi_rdo_container.registerOrEraseNewData(module_i,dest_range_guard.range())) {
                  ++n_rejected_work;
               }
               // in the process of adding hits to the original container, its capacity may have
               // been exceeded and the container may have been changed for the current module. To
               // ensure that the same container is used for the next module get the container, that
               // contains the hits for the current module from the the range_guard.
               rdo_container_dest = dest_range_guard.ptr();
            }
         }
         n_container_changes += n_container_changes_per_roi;
      });
   }
   // wait for all threads to finish
   for (std::thread &a_thread : threads) {
      a_thread.join();
   }

   // compute a FNV hash  from the coordinates of all hits of all modules of all ROIs, in the module index order
   // only using the data of each module at most once.
   FNVHash fnvHash;
   auto rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(roi_rdo_container);
   using PixelRawDataContainerProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataContainerProxy;
   using PixelRawDataProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataProxy;

   for (PixelRawDataContainerProxy module_rdo_container_proxy : rdo_container_collection_proxy) {
      for (PixelRawDataProxy rdo_proxy : module_rdo_container_proxy) {
         for (auto elm :  rdo_proxy.coordinates()) {
            fnvHash.add(elm);
         }
      }
   }
   std::cout << "rejected work " << n_rejected_work << ", container changes " << n_container_changes << std::endl;
   return fnvHash.value();
}

// test skeleton which does some of the work of the roiFillMT or roiFillNonMT, which is not to
// be considered when comparing the timing.
// this method will create one hit container per ROI, create one thread per ROI, which will not
// do any work, wait for the threads to finish and compute the FNV hash using the input
// hit data.
std::size_t roiFillOverhead(const PhaseIIPixelRawDataContainer &rdo_container,
                    const std::vector<std::vector<unsigned int> > &rois,
                    const std::vector<std::pair<unsigned int, unsigned int> > &used_modules) {
   // create one hit container per ROI (unused)
   std::vector<PhaseIIPixelRawDataContainer> roi_rdo_container;
   roi_rdo_container.reserve(rois.size());
   for (unsigned int roi_i=0; roi_i< rois.size() ; ++roi_i) {
      roi_rdo_container.emplace_back(rdo_container.size(), 1);
   }

   // create and start one thread per ROI, where non of the threads will do any work
   std::vector<std::thread> threads;
   threads.reserve( rois.size());
   for (const std::vector<unsigned int> &roi : rois) {
      (void) roi;
      threads.emplace_back( [] () {
      });
   }

   // wait for the threads to finish
   for (std::thread &a_thread : threads) {
      a_thread.join();
   }

   // compute the FNV hash using the coordinates of all hits of all modules of all ROIs, but
   // get the hit data from the input collection. The per module hit data is process in module
   // index order and each module is only counted once.
   FNVHash fnvHash;
   for (auto [module_index, container_index] : used_modules) {
      assert( container_index < roi_rdo_container.size());
      const PhaseII::DataRange &range = rdo_container.range(module_index);
      const PhaseII::PixelRawDataContainer &data = rdo_container.data( range.containerIndex());
      for (unsigned int rdo_idx = range.beginIndex(); rdo_idx <range.endIndex(); ++rdo_idx) {
         for (auto elm :  data.coordinates(rdo_idx)) {
            fnvHash.add(elm);
         }
      }
   }
   // return the FNV hash
   return fnvHash.value();
}

using CheckSum=std::size_t;
using Param1=const PhaseIIPixelRawDataContainer *;
using Param2=std::vector< std::vector< unsigned int> >;
using Param4=std::vector< std::pair<unsigned int,unsigned int> >;
int main(int argc, char **argv) {
   // default arguments
    unsigned int max_modules=6;
    unsigned int max_hits_per_module=6;
    unsigned int max_modules_per_ROI=4;
    unsigned int max_ROIs=2;
    unsigned int n_columns = 32;
    unsigned int n_rows = 16;

    // by default do not run the multi-threaded tests to avoid spawning threads during the CI.
    bool thread_test=false;

    //process command line arguments for running benchmarks
    for (int arg_i=1; arg_i<argc; ++arg_i) {
      if ((strcmp(argv[arg_i],"--dim")==0) && arg_i+2<argc) {
         n_columns = static_cast<unsigned int>(atoi(argv[++arg_i]));
         n_rows = static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if ((strcmp(argv[arg_i],"--n-modules")==0) && arg_i+1<argc) {
         max_modules = static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if ((strcmp(argv[arg_i],"--n-rois")==0) && arg_i+1<argc) {
         max_ROIs = static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if ((strcmp(argv[arg_i],"--n-rois")==0) && arg_i+1<argc) {
         max_ROIs = static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if ((strcmp(argv[arg_i],"--max-hits")==0 || strcmp(argv[arg_i],"--max-hits-per-module")==0) && arg_i+1<argc) {
         max_hits_per_module = static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if ((strcmp(argv[arg_i],"--max-modules-per-ROI")==0 || strcmp(argv[arg_i],"--modules-per-ROI")==0) && arg_i+1<argc) {
         max_modules_per_ROI = static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if ((strcmp(argv[arg_i],"--thread-test")==0)) {
         thread_test=true;
      }
      else {
         // provide some help in case of incorrect arguments
         std::cerr << "ERROR unhandled arg " << arg_i << " / " << (argc-1) << " : " << argv[arg_i] << std::endl;
         std::cerr << "USAGE: " << argv[0]
                   << " [--dim cols rows] [--n-modules N] [--n-rois N] [--max-hits/--max-hits-per-module] [--max-modules-per-ROI/--modules-per-ROI N]"
                   << " [--thread-test]"
                   << "\n\n"
                   << "EXAMPLE: " << argv[0] << " --dim 800 768 --n-modules 18192 --n-rois 200 --max-hits 200 --max-modules-per-ROI 500 --thread-test"
                   << std::endl;
         return 1;
      }
    }
    if (max_modules == 0){
      std::cerr << "Max modules cannot be zero.\n";
      return 1;
    }
    
    // create toy data
    unsigned int max_containers=max_ROIs;
    //coverity[TAINTED_SCALAR]
    //first create random ROIs
    std::vector< std::vector< unsigned int> > rois = makeROIs( max_modules, max_ROIs, max_modules_per_ROI );

    // create auxiliary containers from the ROI collection, which are overlap free:
    std::vector< std::vector< unsigned int> > rois_no_overlap=removeOverlap(max_modules, rois);
    // .. and provide pairs of module index and associated ROI index for all modules in module index order
    std::vector< std::pair<unsigned int,unsigned int> > container_index=findContainer(max_modules, rois_no_overlap);

    // copy the toy data to RDO containers, which is used as an input in the tests.
    std::unique_ptr< PhaseIIPixelRawDataContainer>  rdo_container = createPixelRawDataContainer(max_modules, max_containers);
    // @TODO could use rois_no_overlap, but hit data only created if nothing had been registered yet for the corresponding
    // module, so this does not matter.
    if (max_hits_per_module == 0 or n_columns == 0){
      std::cerr << "Neither max_hits_per_module nor n_columns cannot be zero.\n";
      return 1;
    }
    for (unsigned int roi_i=0; roi_i<rois.size(); ++roi_i) {
       fillPixelRawDataContainer(roi_i, rois[roi_i], max_hits_per_module, n_columns, n_rows, *rdo_container);
    }

    std::cout << "Hits total " << countHits(*rdo_container) << " total modules in ROis " << countModules(rois) << std::endl;

    if (!thread_test) {
       // only run the iteration tests once and compare the resulting output
       conversionTest(*rdo_container);
       std::size_t sum0 =testNonConst(*rdo_container);
       std::size_t sum1 =test(*rdo_container);
       std::size_t sum2 =testAlt(*rdo_container);
       std::cout << " sum " << sum1 << " alt " << sum2 << std::endl;
       return sum0 == sum1 && sum1 == sum2 ? 0 :  1;
    }
    else {
       // @TODO restore the original benchmark functionality and optionally run each test multiple times
       //       and measure the time ?
       // only run the thread tests once and compare the resulting check sums
       // To estimate the overhead of creating the one container collection per ROI for the non-MT version,
       // the thread creation and the computation of the FNV hash at the end, run a dummy.
       std::size_t checksum0 = roiFillOverhead(*rdo_container,rois_no_overlap,container_index);
       // for the non-MT version use input data for which the overlap region in some ROIs are removed
       // such that a module appears at most once in all ROIs together.
       std::size_t checksum1 = roiFillNonMT(*rdo_container,rois_no_overlap,container_index);
       // for the MT version the input data may contain overlapping ROIs, As a consequence part of the
       // work is eventually done multiple times, although only one result is stored.
       std::size_t checksum2 = roiFillMT(*rdo_container,rois);
       std::cout << "thread test check sum overhead: " << checksum0
                 << " non-MT " << checksum1 << (checksum1==checksum0 ? " (ok)" : " (DIFFERS)")
                 << " MT "     << checksum2 << (checksum2==checksum0 ? " (ok)" : " (DIFFERS)")
                 << std::endl;
       return (checksum0 == checksum1 && checksum0 == checksum2 ? 0 : 1);
    }
 }
