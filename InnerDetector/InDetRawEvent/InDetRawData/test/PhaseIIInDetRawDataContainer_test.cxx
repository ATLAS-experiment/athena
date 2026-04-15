/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "InDetRawData/PhaseIIPixelRawDataContainerMT.h"

#include <thread>
#include <memory>
#include <atomic>
#include <array>
#include <chrono>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <cstring>
#include <cstdlib>

#include "FNVHash.h"

using namespace std::literals::chrono_literals;
struct Work {
   std::uint32_t eventIndex = std::numeric_limits<unsigned int>::max();
   std::uint16_t roiIndex = std::numeric_limits<std::uint16_t>::max();
   std::uint8_t  slotIndex  = std::numeric_limits<std::uint8_t>::max();
   enum Type : std::uint8_t {
      kNoWork,
      kOk,
      kAbort
   } status {};
   Work(Type a_type,
        unsigned int an_event = std::numeric_limits<std::uint32_t>::max(),
        unsigned int a_roi = std::numeric_limits<std::uint16_t>::max(),
        unsigned int a_slot = std::numeric_limits<std::uint8_t>::max())
      : eventIndex(an_event), roiIndex(a_roi), slotIndex(a_slot), status(a_type)
   {}
};

// Ring buffer based work queue supporting one writer and multiple readers
struct WorkQueue {
   WorkQueue(unsigned int n_slots) {
      m_ringQueue.resize(n_slots,Work(Work::kNoWork));
   }
   // Get work descriptor from the queue
   Work getWork() {
      for (unsigned int current=m_read; current!= m_write; ) {
         Work current_work = m_ringQueue[ current % m_ringQueue.size() ];
         if (m_read.compare_exchange_weak(current, current+1)) {
            ++m_retrievedWorked;
            return current_work;
         }
      }
      return Work(m_abort ? Work::kAbort : Work::kNoWork);
   }
   // Push a new work descriptor to the queue
   bool pushWork(Work &&work) {
      if (m_write - m_read>=m_ringQueue.size()) return false;
      unsigned int current_write_index = m_write;
      m_ringQueue[current_write_index % m_ringQueue.size() ]=std::move(work);
      if (!m_write.compare_exchange_strong(current_write_index, current_write_index+1, std::memory_order::seq_cst)) return false;
      ++m_pushedWorked;
      return true;
   }
   // Wait for a new slot becomes available to push new work
   void waitForFreeSlot() {
      unsigned int current = m_read;
      if (m_write - current>=m_ringQueue.size()) {
         while (current == m_read && !m_abort) {
            ++m_waitForPush;
            std::this_thread::sleep_for(m_sleepForFreeSlot);
         }
      }
   }
   // Wait until new work becomes available
   void waitForWork() {
      unsigned int current_write = m_write;
      while (current_write == m_write && !m_abort) {
         ++m_waitForEvent;
         std::this_thread::sleep_for(m_sleepForWork);
      }
   }
   // Mark abort status
   // the readers and writers should honor the status and abort.
   void abort() {
      m_abort=true;
   }
   std::vector<Work> m_ringQueue;
   std::atomic<unsigned int> m_read {};
   std::atomic<unsigned int> m_write {};

   static std::chrono::duration<std::int64_t, std::nano> sleepForFreeSlot() {
      using namespace std::chrono_literals;
      return 1us;
   }
   static std::chrono::duration<std::int64_t, std::nano> sleepForWork() {
      using namespace std::chrono_literals;
      return 1us;
   }

   std::chrono::duration<std::int64_t, std::nano> m_sleepForFreeSlot=sleepForFreeSlot();
   std::chrono::duration<std::int64_t, std::nano> m_sleepForWork=sleepForWork();
   std::atomic<unsigned int > m_pushedWorked{};
   std::atomic<unsigned int > m_retrievedWorked{};
   std::atomic<unsigned int > m_waitForPush{};
   std::atomic<unsigned int > m_waitForEvent{};

   bool m_abort=false;
};

// structure to hold toy data of one event
struct Event {
   std::vector<std::vector<unsigned int> > rois;                // module hashes of modules relevant for an ROI per ROI
   std::vector<std::vector<std::array<std::int16_t,2> > > hits; // hit coordinates per module
   std::vector<std::vector<unsigned int > > dataWord;           // hit data word per module
};

// structure to hold toy events
// toy events are used multiple times in random order
struct EventList {
   std::size_t size() const {
      return eventIndex.size();
   }
   auto begin() const {
      return eventIndex.begin();
   }
   auto end()const {
      return eventIndex.end();
   }
   Event &getEvent(unsigned int event_index) {
      assert( event_index < eventIndex.size() );
      assert( eventIndex[event_index] < events.size() );
      return events[eventIndex[event_index]];
   }
   const Event &getEvent(unsigned int event_index) const {
      assert( event_index < eventIndex.size() );
      assert( eventIndex[event_index] < events.size() );
      return events[eventIndex[event_index]];
   }
   std::vector<unsigned int> eventIndex; // the order in which the toy events should appear
   std::vector<Event> events;            // the pool of toy events
};

// inequality operator to compare a vector and span of const atomics of the same type.
template<typename T>
bool operator!=(const std::vector<T> &a, std::span<const std::atomic<T> > b) {
   if (a.size() != b.size()) return true;
   for (unsigned int i=0; i<a.size(); ++i) {
      if (a[i] != b[i]) return true;
   }
   return false;
}
// inequality operator to compare a vector and span of atomics of the same type.
template<typename T>
bool operator!=(const std::vector<T> &a, std::span<std::atomic<T> > b) {
   if (a.size() != b.size()) return true;
   for (unsigned int i=0; i<a.size(); ++i) {
      if (a[i] != b[i]) return true;
   }
   return false;
}

// structure to store the results of the processing
// the result is one hash per ROI and event.
struct Results {
   static std::vector<unsigned int> makeOffsets(const EventList &events) {
      std::vector<unsigned int> offset;
      offset.reserve( events.size());
      offset.push_back(0u);
      for (unsigned int event_index : events) {
         assert( event_index < events.events.size());
         const Event& event = events.events[event_index];
         offset.push_back(offset.back()+event.rois.size());
      }
      return offset;
   }

   // The constructor will alllocate storage for the results per ROI and event.
   Results(const EventList &events) : resultOffset(makeOffsets(events)), results(resultOffset.back()) {}

   // compute a hash for one event and ROI using the toy data.
   static unsigned int computeHash(const Event &event, unsigned int roi_i) {
      assert( roi_i < event.rois.size() );
      const std::vector<unsigned int> & roi = event.rois[roi_i];
      FNVHash hash;
      for (unsigned int module_i : roi) {
         assert( module_i < event.hits.size());
         for (unsigned int hit_i=0; hit_i<event.hits[module_i].size(); ++hit_i) {
            hash.add(event.hits[module_i][hit_i]);
            hash.add(event.dataWord[module_i][hit_i]);
         }
      }
      return hash.value();
   }
   // compute a hash for one event and the given ROIs using the data stored in the output container (MT-version).
   static unsigned int computeHash(const PhaseIIPixelRawDataContainerMT &raw_data_container, const std::vector<unsigned int> &roi) {
      FNVHash hash;
      PhaseII::PixelRawDataTypeTraits<>::ContainerCollectionProxy
         rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(raw_data_container);
      for (unsigned int module_i : roi) {
         assert( module_i < rdo_container_collection_proxy.size());
         PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy
            module_proxy = rdo_container_collection_proxy[module_i];
         for (PhaseII::PixelRawDataTypeTraits<>::RawDataProxy
                 rdo_proxy : module_proxy) {
            hash.add(rdo_proxy.coordinates());
            hash.add(rdo_proxy.dataWord());
         }
      }
      return hash.value();
   }
   // compute a hash for one event and the given ROIs using the data stored in the output container (nonMT-version).
   static unsigned int computeHash(const PhaseIIPixelRawDataContainer &raw_data_container, const std::vector<unsigned int> &roi) {
      FNVHash hash;
      PhaseII::PixelRawDataTypeTraits<>::ContainerCollectionProxy
         rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(raw_data_container);
      for (unsigned int module_i : roi) {
         assert( module_i < rdo_container_collection_proxy.size());
      PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy
            module_proxy = rdo_container_collection_proxy[module_i];
         for (PhaseII::PixelRawDataTypeTraits<>::RawDataProxy
                 rdo_proxy : module_proxy) {
            hash.add(rdo_proxy.coordinates());
            hash.add(rdo_proxy.dataWord());
         }
      }
      return hash.value();
   }

   // compute hashes for one event using the toy data.
   static std::vector<unsigned int> computeHash(const Event &event) {
      std::vector<unsigned int> hash_codes;
      hash_codes.reserve( event.rois.size());
      for (unsigned int roi_i=0; roi_i < event.rois.size(); ++roi_i) {
         hash_codes.push_back(computeHash(event, roi_i));
      }
      return hash_codes;
   }
   // get the results for one event obtained from the output container (non-const version)
   std::span<std::atomic<unsigned int> > eventResults(unsigned int event_index) {
      assert( event_index+1 < resultOffset.size() );
      return std::span(results.begin() + resultOffset[event_index], results.begin() + resultOffset[event_index+1]);
   }
   // get the results for one event obtained from the output container (const version)
   std::span<const std::atomic<unsigned int> > eventResults(unsigned int event_index) const {
      assert( event_index+1 < resultOffset.size());
      return std::span(results.begin() + resultOffset[event_index], results.begin() + resultOffset[event_index+1]);
   }
   // compare the results with the expectation from the toy data.
   // return true if the results agree with the expectation.
   bool compareResults(const EventList &events) const {
      std::vector<unsigned int> tmp_hashes;
      for (unsigned int event_index=0; event_index < events.eventIndex.size(); ++event_index) {
         const Event& event = events.getEvent(event_index);
         tmp_hashes = computeHash(event);
         if (tmp_hashes != eventResults(event_index)) return false;
      }
      return true;
   }

   std::vector< unsigned int> resultOffset; // offset to the results for the first ROI of an event.
   std::vector< std::atomic<unsigned int> > results; //the results of all evnets and ROIs
};

struct AtomicTypelessPtr {
   void *set(void *new_ptr) {
      void *old_ptr = m_ptr;
      if (!m_ptr.compare_exchange_strong(old_ptr, new_ptr, std::memory_order::seq_cst)) return new_ptr;
      return old_ptr;
   }
   std::atomic<void *> m_ptr{};
};

// unique_ptr
// setting is atomic.
template <typename T>
struct AtomicUniquePtr : private AtomicTypelessPtr {
   AtomicUniquePtr() = default;

   // try to set a new pointer
   // will either delete the new or old pointer
   void set(std::unique_ptr<T> &&new_ptr) {
      std::unique_ptr<T> ptr( static_cast<T *>(AtomicTypelessPtr::set(new_ptr.release())) );
   }
   std::unique_ptr<T> release() {
      std::unique_ptr<T> ptr( static_cast<T *>(AtomicTypelessPtr::set(nullptr)) );
      return ptr;
   }
   T *ptr() { return static_cast< T *>(this->m_ptr.load()); }
   const T *ptr() const { return static_cast< T *>(this->m_ptr.load()); }
};

// structure to provide output containers for multiple concurrent events.
template <bool MT>
struct EventStoreImpl {
   EventStoreImpl(unsigned int concurrent_events)
      : eventData(concurrent_events),
        toProcess(concurrent_events)
   {}
   // get a free slot id
   unsigned int freeSlot() {
      unsigned int slot_i=0;
      for (uint64_t value : toProcess) {
         if (value == 0ul) {
            return slot_i;
         }
         ++slot_i;
      }
      return std::numeric_limits<unsigned int>::max();
   }
   // create a new event
   unsigned int getNewEvent(unsigned int max_modules, [[maybe_unused]] unsigned int container_list_size, unsigned int n_rois) {
      unsigned int slot_i=freeSlot();
      assert( eventData.size() == toProcess.size() );
      std::uint64_t should_to_process=0ul;
      if (slot_i < eventData.size() && toProcess[slot_i].compare_exchange_strong(should_to_process, static_cast<std::uint64_t>(n_rois),  std::memory_order::seq_cst)) {
         if constexpr(MT) {
            eventData[slot_i].set(std::make_unique< PhaseIIPixelRawDataContainerMT>(max_modules,
                                                                                    container_list_size));
         }
         else {
            eventData[slot_i].clear();
            eventData[slot_i].reserve(n_rois);
            for (unsigned int roi_i=0; roi_i < n_rois; ++roi_i) {
               eventData[slot_i].emplace_back( std::make_unique<PhaseIIPixelRawDataContainer>(max_modules,1u));
            }
         }
         return slot_i;
      }
      return std::numeric_limits<unsigned int>::max();
   }
   // get the output container to be used for a certain event slot and eventually ROI.
   auto getEventDataValidForRoi(unsigned int slotIndex, [[maybe_unused]] unsigned int roiIndex) {
      assert(slotIndex < eventData.size() );
      if constexpr(MT) {
         return eventData[slotIndex].ptr();
      }
      else {
         assert(roiIndex < eventData[slotIndex].size() );
         return eventData[slotIndex][roiIndex].get();
      }
   }

   std::conditional<MT,
                    std::vector< AtomicUniquePtr<PhaseIIPixelRawDataContainerMT> >,
                    std::vector< std::vector<std::unique_ptr<PhaseIIPixelRawDataContainer>> > >::type eventData;
   std::vector< std::atomic< uint64_t > > toProcess;
};

using EventStoreMT=EventStoreImpl<true>;       // event store which provides an output container which supports concurrent filling
using EventStoreNonMT=EventStoreImpl<false>;   // event store which provides an output container per ROI.

// function which pushes all the events to the work queue.
template <typename T_EventStore>
void eventLoop(const EventList &events, unsigned int max_modules, unsigned int initial_container_list_size, WorkQueue &queue, T_EventStore &store) {
   unsigned int wait_for_free_slot=0; // for debugging
   for (unsigned int event_index=0; event_index < events.size(); ++event_index) {
      const Event &event= events.getEvent(event_index);
      unsigned int slot_i;
      for (;;) {
         slot_i=store.getNewEvent(max_modules, std::max(initial_container_list_size, static_cast<unsigned int>(event.rois.size())), event.rois.size());
         if (slot_i<store.eventData.size()) break;
         std::this_thread::sleep_for(queue.m_sleepForFreeSlot);
         ++wait_for_free_slot;
      }
      assert( slot_i == static_cast<std::uint8_t>(slot_i));
      assert( event.rois.size() == static_cast<std::uint16_t>(event.rois.size()));
      for (unsigned int roi_i=0; roi_i < event.rois.size(); ++roi_i) {
         while (!queue.pushWork(Work(Work::kOk, event_index, static_cast<std::uint16_t>(roi_i), static_cast<std::uint8_t>(slot_i)))) {
            queue.waitForFreeSlot();
         }
      }
   }
   queue.abort();
   (void) wait_for_free_slot;
}

// function which copies the toy data of an ROI of one event to an output container (MT version)
void storeEventData(const EventList &events, EventStoreMT &store, unsigned int eventIndex, std::uint16_t roiIndex, std::uint8_t slotIndex) {
   unsigned int n_rejected_work=0u; // for debugging
   assert( slotIndex < store.eventData.size() );
   PhaseIIPixelRawDataContainerMT *a_roi_rdo_container = store.eventData[slotIndex].ptr();
   assert( a_roi_rdo_container );
   PhaseIIPixelRawDataContainerMT::ContainerPtr rdo_container_dest = a_roi_rdo_container->getNewContainerPtr();
   const Event &event= events.getEvent(eventIndex);
   assert(roiIndex < event.rois.size());
   // "estimate" number of RDOs for this ROI
   unsigned int n_rdos=0u;
   for (unsigned int module_i : event.rois[roiIndex]) {
      n_rdos += event.hits[module_i].size();
   }
   // fill output container
   using CoordType = std::array<std::int16_t,2>;
   rdo_container_dest->reserve(n_rdos);
   for (unsigned int module_i : event.rois[roiIndex]) {
      PhaseII::ContainerRangeGuard<PhaseII::DataRange, PhaseIIPixelRawDataContainerMT::ContainerPtr>
         dest_range_guard(rdo_container_dest);
      assert(module_i < event.hits.size() && module_i < event.dataWord.size());
      assert(event.hits[module_i].size() == event.dataWord[module_i].size());
      for (unsigned int hit_i=0; hit_i < event.hits[module_i].size(); ++hit_i) {
         unsigned int event_data_word = event.dataWord[module_i][hit_i];
         addDataForModule(*a_roi_rdo_container,
                          dest_range_guard,
                          CoordType(event.hits[module_i][hit_i]),
                          PhaseII::PixelRawDataContainer::makeWord( PhaseII::PixelRawDataContainer::getToT(event_data_word ),
                                                                    PhaseII::PixelRawDataContainer::getBCID(event_data_word ),
                                                                    PhaseII::PixelRawDataContainer::getLVL1A(event_data_word ),
                                                                    PhaseII::PixelRawDataContainer::getLVL1ID(event_data_word )));
      }

      if (!dest_range_guard.empty()) {
         // register the RDO range for this module, or erase the newly added
         if (!a_roi_rdo_container->registerOrEraseNewData(module_i,dest_range_guard.range())) {
            ++n_rejected_work;
         }
         // in the process of adding hits to the original container, its capacity may have
         // been exceeded and the container may have been changed for the current module. To
         // ensure that the same container is used for the next module get the container, that
         // contains the hits for the current module from the the range_guard.
         rdo_container_dest = dest_range_guard.ptr();
      }
   }
   (void) n_rejected_work;
}

template<typename T>
struct ContainerPtrWithId {
   unsigned int containerId() const { return id; }

   T *operator->() { return m_ptr; }
   const T *operator->() const { return m_ptr; }
   T &operator*() { return *m_ptr; }
   const T &operator*() const { return *m_ptr; }

   T *m_ptr;
   unsigned int id;
};

// function which copies the toy data of an ROI of one event to an output container (non-MT version)
void storeEventData(const EventList &events, EventStoreNonMT &store, unsigned int eventIndex, std::uint16_t roiIndex, std::uint8_t slotIndex) {
   unsigned int n_rejected_work=0u; // for debugging
   assert( slotIndex < store.eventData.size() );
   assert( roiIndex < store.eventData[slotIndex].size() );
   PhaseIIPixelRawDataContainer *a_roi_rdo_container = store.eventData[slotIndex][roiIndex].get();
   assert( a_roi_rdo_container );
   //   assert( a_roi_rdo_container->containerListCapacity()>slotIndex);
   using ContainerPtr = ContainerPtrWithId<PhaseIIPixelRawDataContainer::DataContainerType>;
   PhaseIIPixelRawDataContainer::DataContainerType *rdo_container_dest = &(a_roi_rdo_container->data(0u));

   const Event &event= events.getEvent(eventIndex);
   assert(roiIndex < event.rois.size());
   // "estimate" number of RDOs for this ROI
   unsigned int n_rdos=0u;
   for (unsigned int module_i : event.rois[roiIndex]) {
      n_rdos += event.hits[module_i].size();
   }
   // fill output container
   using CoordType = std::array<std::int16_t,2>;
   rdo_container_dest->reserve(n_rdos);
   for (unsigned int module_i : event.rois[roiIndex]) {
      PhaseII::ContainerRangeGuard<PhaseII::DataRange, ContainerPtr >
         dest_range_guard(ContainerPtr{rdo_container_dest,0u});
      assert(module_i < event.hits.size() && module_i < event.dataWord.size());
      assert(event.hits[module_i].size() == event.dataWord[module_i].size());
      for (unsigned int hit_i=0; hit_i < event.hits[module_i].size(); ++hit_i) {
         // store the data of one hit
         unsigned int event_data_word = event.dataWord[module_i][hit_i];
         rdo_container_dest->emplace_back(CoordType(event.hits[module_i][hit_i]),
                                          PhaseII::PixelRawDataContainer::makeWord( PhaseII::PixelRawDataContainer::getToT(event_data_word ),
                                                                                    PhaseII::PixelRawDataContainer::getBCID(event_data_word ),
                                                                                    PhaseII::PixelRawDataContainer::getLVL1A(event_data_word ),
                                                                                    PhaseII::PixelRawDataContainer::getLVL1ID(event_data_word )));

      }
      if (!dest_range_guard.empty()) {
         // register the RDO range for this module, or erase the newly added
         if (!a_roi_rdo_container->registerOrEraseNewData(module_i,dest_range_guard.range())) {
            ++n_rejected_work;
         }
         // in the process of adding hits to the original container, its capacity may have
         // been exceeded and the container may have been changed for the current module. To
         // ensure that the same container is used for the next module get the container, that
         // contains the hits for the current module from the the range_guard.
         rdo_container_dest = dest_range_guard.ptr().m_ptr;
      }
   }
   (void) n_rejected_work;
}

// process the work of the work queue
template <typename T_EventStore>
void processEvent(const EventList &events, WorkQueue &queue, Results &results, T_EventStore &store) {
   unsigned int n_processed=0; // for debugging
   unsigned int n_waited=0;    // for debugging

   for(;;) {
      // get the work descriptor
      Work work = queue.getWork();
      if (work.status == Work::kOk) {
         // get the toy event specified by the work descriptor
         const Event &event= events.getEvent(work.eventIndex);

         // copy the toy data of the specified ROI to the output container of this event.
         assert( work.roiIndex < event.rois.size());
         storeEventData(events, store, work.eventIndex, work.roiIndex, work.slotIndex);
         // compute the hash for the ROI and store it in the results container
         const auto  *a_roi_rdo_container = store.getEventDataValidForRoi(work.slotIndex, work.roiIndex);
         assert( a_roi_rdo_container );
         unsigned int hash = Results::computeHash(*a_roi_rdo_container, event.rois[work.roiIndex]);
         auto event_results = results.eventResults(work.eventIndex);
         assert( work.roiIndex < event_results.size());

         event_results[work.roiIndex]=hash;
         ++n_processed;
         assert( store.toProcess[work.slotIndex] > 0 );
         --store.toProcess[work.slotIndex];
      }
      else if (work.status == Work::kAbort) {
         break;
      }
      else {
         queue.waitForWork();
         ++n_waited;
      }
   }
   (void) n_processed;
   (void) n_waited;
}

// create toy events
// a toy event contains a certain random number of hits at random locations per module,
// has a certain random number of ROis consisting of a certain random number of modules.
EventList makeEvents(unsigned int n_modules, unsigned int n_rows, unsigned int n_columns, unsigned int n_events, unsigned int n_event_indices,
                     unsigned int min_rois, unsigned int max_rois,
                     unsigned int min_modules_per_roi, unsigned int max_modules_per_roi,
                     unsigned int min_hits_per_module, unsigned int max_hits_per_module) {
   EventList events;
   events.events.reserve( n_events);
   unsigned int n_extra_hits=max_hits_per_module - min_hits_per_module;
   unsigned int n_extra_rois=max_rois - min_rois;
   unsigned int n_extra_modules = max_modules_per_roi - min_modules_per_roi;
   for (unsigned int event_i=0; event_i<n_events; ++event_i) {
      events.events.emplace_back();
      events.events.back().hits.resize(n_modules);
      events.events.back().dataWord.resize(n_modules);
      for (unsigned int module_i=0; module_i<events.events.back().hits.size(); ++module_i) {
         std::vector<std::array<std::int16_t,2> > &module_hits=events.events.back().hits[module_i];
         std::vector<unsigned int> &module_data_word=events.events.back().dataWord[module_i];
         unsigned int n_hits= (n_extra_hits > 0 ? rand() % (n_extra_hits) : 0u) + min_hits_per_module;
         module_hits.reserve(n_hits);
         for (unsigned int  hit_i=0; hit_i< n_hits; ++hit_i) {
            module_hits.push_back( std::array<std::int16_t, 2> { static_cast<std::int16_t>(rand() % n_rows),
                                                                 static_cast<std::int16_t>(rand() % n_columns) });

            unsigned int rand_data_word = rand();
            module_data_word.push_back(PhaseII::PixelRawDataContainer::makeWord( PhaseII::PixelRawDataContainer::getToT(rand_data_word ),
                                                                                 PhaseII::PixelRawDataContainer::getBCID(rand_data_word ),
                                                                                 PhaseII::PixelRawDataContainer::getLVL1A(rand_data_word ),
                                                                                 PhaseII::PixelRawDataContainer::getLVL1ID(rand_data_word )));
         }
      }
      unsigned int n_rois = min_rois + (n_extra_rois > 0 ? rand() % n_extra_rois : 0u);
      events.events.back().rois.reserve(n_rois);
      for (unsigned int roi_i=0; roi_i< n_rois; ++roi_i) {
         unsigned int n_modules_per_roi = min_modules_per_roi + (n_extra_modules>0 ? rand() % n_extra_modules : 0u);
         events.events.back().rois.emplace_back();
         events.events.back().rois.back().reserve( n_modules);
         for (unsigned int module_i=0; module_i<n_modules_per_roi; ++module_i) {
            events.events.back().rois.back().push_back( rand() % n_modules);
         }
      }
   }
   events.eventIndex.reserve( n_event_indices);
   assert( static_cast<unsigned int>(events.events.size()) == events.events.size());
   for (unsigned int event_i=0; event_i< n_event_indices; ++event_i) {
      events.eventIndex.push_back( rand() % events.events.size());
   }
   return events;
}

template <typename T_RDOProxy>
concept canModifyRDO = requires( T_RDOProxy &&a) { a.dataWord() = 0u;};

#define always_assert(expr) if (!(expr)) { throw std::runtime_error(std::string("assert failed: ")+#expr); } do {} while (0)

// test: proxy read write to read only conversion; module proxy adapter; child index computation.
void conversionTest(PhaseIIPixelRawDataContainerMT &rdo_container ) {
   if (rdo_container.empty()) return;

   PhaseII::PixelRawDataTypeTraits<PhaseII::AccessPolicy::Mutable>::ContainerCollectionProxy
      rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(rdo_container);
   using PixelRawDataContainerProxyRW = PhaseII::PixelRawDataTypeTraits<Utils::AccessPolicy::Mutable>::RawDataContainerProxy;
   using PixelRawDataProxy = PhaseII::PixelRawDataTypeTraits<>::RawDataProxy;
   using PixelRawDataProxyRW = PhaseII::PixelRawDataTypeTraits<Utils::AccessPolicy::Mutable>::RawDataProxy;

   static_assert( Utils::isConvertableToReadOnlyProxy<PixelRawDataProxy,PixelRawDataProxyRW> );

   {
      // find a module with hits
      unsigned int module_i=0;
      for (; module_i < rdo_container_collection_proxy.size(); ++module_i) {
         if (!rdo_container_collection_proxy[module_i].empty()) break;
      }
      always_assert( module_i < rdo_container_collection_proxy.size());
      if (module_i>=rdo_container_collection_proxy.size()) {
         throw std::runtime_error("An empty event/ROI should not exist.");
      }
      // test proxy conversion and index recovery
      PhaseII::PixelRawDataTypeTraits<PhaseII::AccessPolicy::Mutable>::RawDataContainerProxy
         a_module_proxy = rdo_container_collection_proxy[module_i];
      static_assert( std::is_same_v<decltype(a_module_proxy), PixelRawDataContainerProxyRW>);
      PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy
         const_module_proxy(a_module_proxy);
      assert( const_module_proxy.size() == a_module_proxy.size() );
      FNVHash hash_non_const;
      for (PhaseII::PixelRawDataTypeTraits<PhaseII::AccessPolicy::Mutable>::RawDataProxy rdo_proxy: a_module_proxy) {
         static_assert( canModifyRDO<decltype(rdo_proxy)>);
         hash_non_const.add(rdo_proxy.coordinates());
         hash_non_const.add(rdo_proxy.dataWord());
      }
      FNVHash hash_const;
      for (PhaseII::PixelRawDataTypeTraits<>::RawDataProxy  rdo_proxy : const_module_proxy) {
         static_assert( !canModifyRDO<decltype(rdo_proxy)>);
         hash_const.add(rdo_proxy.coordinates());
         hash_const.add(rdo_proxy.dataWord());
      }
      always_assert( hash_non_const.value() != 0u);
      always_assert( hash_non_const.value() == hash_const.value() );

      //      static_assert( std::is_same_v<decltype(a_module_proxy), PixelRawDataContainerProxy> || std::is_same_v<decltype(a_module_proxy), PixelRawDataContainerProxyRW>);
      PhaseII::PixelRawDataTypeTraits<PhaseII::AccessPolicy::Mutable>::RawDataContainerProxy
         a_module_proxy_back = rdo_container_collection_proxy.back();

      // get the index which can be used as index for the access operator [] of the parent proxy
      std::size_t index0 = rdo_container_collection_proxy.computeChildElementIndex(a_module_proxy);
      std::size_t back_index = rdo_container_collection_proxy.computeChildElementIndex(a_module_proxy_back);

      // test that these indices indeed recover the same element
      PhaseII::PixelRawDataTypeTraits<PhaseII::AccessPolicy::Mutable>::RawDataContainerProxy
         element0 = rdo_container_collection_proxy[index0];
      PhaseII::PixelRawDataTypeTraits<PhaseII::AccessPolicy::Mutable>::RawDataContainerProxy
         element_back = rdo_container_collection_proxy[back_index];
      always_assert( element0.index() == a_module_proxy.index() && &element0.container() == &a_module_proxy.container());
      always_assert( element_back.index() == a_module_proxy_back.index() && &element_back.container() == &a_module_proxy_back.container());
   }

   for (PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy module_proxy : rdo_container_collection_proxy) {
      if (!module_proxy.empty()) {
         PhaseII::PixelRawDataTypeTraits<>::RawDataProxy
            pixel_proxy(module_proxy[0]);
         static_assert( std::is_same_v<decltype(pixel_proxy), PixelRawDataProxy> || std::is_same_v<decltype(pixel_proxy), PixelRawDataProxyRW>);

         PhaseII::PixelRawDataTypeTraits<>::RawDataProxy
            pixel_proxy_back(module_proxy.back());

         // get the index which can be used as index for the access operator [] of the parent proxy
         std::size_t index0 = module_proxy.computeChildElementIndex(pixel_proxy);
         std::size_t back_index = module_proxy.computeChildElementIndex(pixel_proxy_back);

         // test that these indices indeed recover the same element
         PhaseII::PixelRawDataTypeTraits<>::RawDataProxy
            element0 = module_proxy[index0];
         PhaseII::PixelRawDataTypeTraits<>::RawDataProxy
            element_back = module_proxy[back_index];
         always_assert( element0.index() == pixel_proxy.index() && &element0.container() == &pixel_proxy.container());
         always_assert( element_back.index() == pixel_proxy_back.index() && &element_back.container() == &pixel_proxy_back.container());
         break;
      }
   }
}


// concurrency test
// will split the toy data into work pacakges each comprising a single ROI,
// the work packages are processed by n-threads.
// during each processing step the toy data of one ROI of one event is compied
// to the output container then hashes are computed for the total data of one ROI.
// and stored.
// Once all work packages are process the results are compared to the expectation.
template <class T_EventStore>
unsigned int threadTest(const EventList &events,
                        unsigned int n_modules,
                        unsigned int initial_container_list_size,
                        unsigned int n_queue_slots,
                        const std::vector<unsigned int> &n_threads) {
   std::vector<unsigned int>::const_iterator max_n_threads_iter = std::max_element(n_threads.begin(),n_threads.end());
   if (max_n_threads_iter == n_threads.end()) return 1;
   unsigned int total_error_count=0u;
   std::vector<std::thread> threads;
   threads.reserve(std::max(*max_n_threads_iter,1u)+1);

   // run the test using a certain number worker threads + one thread which fills the work
   // package into the queue.
   for (unsigned int n_worker_threads : n_threads ) {
      threads.clear();
      threads.reserve(n_worker_threads+1);

      Results results(events);
      T_EventStore store(n_worker_threads);

      std::cout << "Start processing " << n_worker_threads << std::endl;
      auto start = std::chrono::steady_clock::now();
      WorkQueue queue(n_queue_slots);
      // create the worker threads
      for (unsigned int work_i=0; work_i<n_worker_threads; ++work_i) {
         threads.emplace_back([&events,&queue, &results, &store]() {
            processEvent(events, queue, results,store);
         });
      }
      // create the work provider thread.
      threads.emplace_back([&events,n_modules, initial_container_list_size, &queue, &store]() {
         eventLoop(events, n_modules, initial_container_list_size, queue,store);
      } );
      for (std::thread &a_thread : threads) {
         a_thread.join();
      }
      auto end = std::chrono::steady_clock::now();
      std::chrono::duration<double> elapsed_seconds = end-start;
      unsigned int n_nonzero_results{};
      for (const std::atomic<unsigned int> &elm : results.results) {
         n_nonzero_results += (elm>0);
      }
      std::cout << "Finished processing. Elapsed time " << elapsed_seconds.count()
                << " s * " << (threads.size() -1) << " (threads)"
                << " = " << (elapsed_seconds.count() * (threads.size() -1)) << " s "
                << " work " << queue.m_pushedWorked << " -> " << queue.m_retrievedWorked
                << " non zero results " << n_nonzero_results << " / " << results.results.size()
                << " wait: push " << queue.m_waitForPush << " retrieve " << queue.m_waitForEvent
                << std::endl;

      // compare the results.
      threads.clear();
      std::vector<std::atomic<std::uint64_t> > errors(threads.capacity());
      for (std::atomic<std::uint64_t> &an_error : errors) {
         an_error=0u;
      }
      unsigned int batch_size = (events.eventIndex.size() + errors.size()-1) / errors.size();
      for (unsigned int part_i=0; part_i<errors.size(); ++part_i) {
         threads.emplace_back([part_i,&errors,&events, &results,batch_size](){
            unsigned int error_counter=0u;
            unsigned int start_event_index = batch_size * part_i;
            unsigned int end_event_index = std::min(start_event_index + batch_size, static_cast<unsigned int>(events.eventIndex.size()));

            std::vector<unsigned int> tmp_hashes;

            for (unsigned int event_index=start_event_index; event_index < end_event_index; ++event_index) {
               const Event& event = events.getEvent(event_index);
               tmp_hashes = Results::computeHash(event);
               if (tmp_hashes != results.eventResults(event_index)) {
                  ++error_counter;
               }
            }
            errors[part_i]=error_counter;
         });
      }
      for (std::thread &a_thread : threads) {
         a_thread.join();
      }
      unsigned int total_error_count_this=0u;
      for (const std::atomic<std::uint64_t> &an_error_count : errors) {
         total_error_count_this+=an_error_count.load();
      }
      total_error_count += total_error_count_this;
      std::cout << "errors " << total_error_count_this << std::endl;
      if (total_error_count!= 0u) {
         return total_error_count;
      }
   }
   return total_error_count;
}

std::unique_ptr<PhaseIIPixelRawDataContainerMT> makeTestContainer(const EventList &events, unsigned int event_index, unsigned int roi_index) {
   EventStoreMT store(1);
   const Event &event  = events.getEvent(event_index);
   unsigned int slot_i=store.getNewEvent(event.hits.size(), 1, event.rois.size());
   storeEventData(events, store, event_index, roi_index, slot_i);
   assert( store.eventData.size() > 0);
   std::unique_ptr<PhaseIIPixelRawDataContainerMT> test_container( store.eventData[0].release() );
   assert( !test_container->empty() );
   PhaseII::PixelRawDataTypeTraits<>::ContainerCollectionProxy
      rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(*test_container);
   unsigned int n_hits=0u;
   for (PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy module_proxy : rdo_container_collection_proxy) {
      n_hits += module_proxy.size();
   }
   always_assert( n_hits>0);
   return test_container;
}

struct EventGenParams {
   unsigned int n_modules=2164;
   unsigned int n_rows=384;
   unsigned int n_columns=400;
   unsigned int n_events=100;
   unsigned int n_event_indices=1000;

   unsigned int min_rois=10;
   unsigned int max_rois=100;
   unsigned int min_modules_per_roi=10;
   unsigned int max_modules_per_roi=100;
   unsigned int min_hits_per_module=10;
   unsigned int max_hits_per_module=200;
};

EventGenParams blitzParams() {
   return EventGenParams{ .n_modules=50,
      .n_rows=384,
      .n_columns=400,
      .n_events=10,
      .n_event_indices=30,
      .min_rois=10,
      .max_rois=30,
      .min_modules_per_roi=10,
      .max_modules_per_roi=50,
      .min_hits_per_module=10,
      .max_hits_per_module=200 };
}
EventGenParams shortParams() {
   return EventGenParams{.n_modules=2164,
                         .n_rows=384,
                         .n_columns=400,
                         .n_events=1000,
                         .n_event_indices=1000,
                         .min_rois=10,
                         .max_rois=200,
                         .min_modules_per_roi=10,
                         .max_modules_per_roi=200,
                         .min_hits_per_module=10,
                         .max_hits_per_module=200};
}

EventGenParams midParams() {
   return EventGenParams{ .n_modules=8164,
                          .n_rows=384,
                          .n_columns=400,
                          .n_events=1000,
                          .n_event_indices=1000,
                          .min_rois=10,
                          .max_rois=200,
                          .min_modules_per_roi=10,
                          .max_modules_per_roi=200,
                          .min_hits_per_module=10,
                          .max_hits_per_module=200};
}

int main(int argc, char **argv) {
   try {
   std::vector<unsigned int> n_threads;
   unsigned int initial_container_list_size=1;
   unsigned int n_queue_slots=256;
   EventGenParams params=blitzParams();

   bool do_non_mt_test=false;
   bool do_mt_test=true;
   bool do_conversion_test=true;

   unsigned int n_args=static_cast<unsigned int>(argc);
   for (unsigned int arg_i=1; arg_i<n_args; ++arg_i) {
      if (strcmp(argv[arg_i],"--threads")==0 && arg_i+1 <n_args) {
         char *ptr=argv[++arg_i];
         bool is_range=false;
         for(;;) {
            char *end_ptr=ptr;
            n_threads.push_back(strtol(ptr, &end_ptr, 10));
            if (is_range) {
               unsigned int end=n_threads.back();
               n_threads.pop_back();
               for (unsigned int thread_i=n_threads.back(); thread_i++<end; ) {
                  n_threads.push_back(thread_i);
               }
            }
            if (*end_ptr=='-' && !is_range) {
               is_range=true;
            }
            else if (*end_ptr!=',') {
               break;
            }
            else {
               is_range=false;
            }
            ptr=end_ptr+1;
         }
      }
      else if (strcmp(argv[arg_i],"--event-pool-size")==0 && arg_i+1 <n_args) {
         params.n_events=static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if (strcmp(argv[arg_i],"--events")==0 && arg_i+1 <n_args) {
         params.n_event_indices=static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if (strcmp(argv[arg_i],"--modules")==0 && arg_i+1 <n_args) {
         params.n_modules=static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if (strcmp(argv[arg_i],"--sensor-size")==0 && arg_i+2 <n_args) {
         params.n_rows=static_cast<unsigned int>(atoi(argv[++arg_i]));
         params.n_columns=static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if (strcmp(argv[arg_i],"--rois")==0 && arg_i+2 <n_args) {
         params.min_rois=static_cast<unsigned int>(atoi(argv[++arg_i]));
         params.max_rois=static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if (strcmp(argv[arg_i],"--roi-size")==0 && arg_i+2 <n_args) {
         params.min_modules_per_roi=static_cast<unsigned int>(atoi(argv[++arg_i]));
         params.max_modules_per_roi=static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if (strcmp(argv[arg_i],"--hits")==0 && arg_i+2 <n_args) {
         params.min_hits_per_module=static_cast<unsigned int>(atoi(argv[++arg_i]));
         params.max_hits_per_module=static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if (strcmp(argv[arg_i],"--init-raw-size")==0 && arg_i+1 <n_args) {
         initial_container_list_size=static_cast<unsigned int>(atoi(argv[++arg_i]));
      }
      else if (strcmp(argv[arg_i],"--short-test-params")==0 || strcmp(argv[arg_i],"--short-test")==0) {
         params=shortParams();
      }
      else if (strcmp(argv[arg_i],"--mid-test-params")==0 || strcmp(argv[arg_i],"--mid-test")==0) {
         params=midParams();
      }
      else if (strcmp(argv[arg_i],"--blitz-test-params")==0 || strcmp(argv[arg_i],"--blitz-test")==0) {
         params=blitzParams();
      }
      else if (strcmp(argv[arg_i],"--show-params")==0 || strcmp(argv[arg_i],"--show")==0) {
         std::cout << std::setw(-20) << "modules " << params.n_modules <<"\n"
                   << std::setw(-20) << "sensor " <<  params.n_rows << " x " << params.n_columns << "\n"
                   << std::setw(-20) << "event pool size " <<  params.n_events <<"\n"
                   << std::setw(-20) << "events " <<  params.n_event_indices << "\n"
                   << std::setw(-20) << "rois " <<  params.min_rois << " .. " << params.max_rois << "\n"
                   << std::setw(-20) << "roi size " <<  params.min_modules_per_roi << " .. " << params.max_modules_per_roi << "\n"
                   << std::setw(-20) << "hits " <<  params.min_hits_per_module << " .. " << params.max_hits_per_module << "\n"
                   << std::endl;
      }
      else if (strcmp(argv[arg_i],"--do-non-mt")==0) {
         do_non_mt_test=true;
      }
      else if (strcmp(argv[arg_i],"--no-mt")==0) {
         do_mt_test=false;
      }
      else if (strcmp(argv[arg_i],"--no-conversion-test")==0) {
         do_conversion_test=false;
      }
      else {
         if (strcmp(argv[arg_i],"--help")!=0
             && strcmp(argv[arg_i],"-h")!=0) {
            std::cout << "ERROR unhandled argument " << argv[arg_i] << "\n";
         }
         std::cout << "USAGE " << argv[0]
                   << " [--threads n-worker-threads[-n-worker-threads-end][,n-worker-threads]]"
                   << " [--events n-events]"
                   << " [--event-prool-size n-physical-events]"
                   << " [--modules n-modules]"
                   << " [--sensor-size rows cols]"
                   << " [--rois min max-rois-per-event]"
                   << " [--roi-size min max-modules-per-roi]"
                   << " [--hits min max-hits-per-module]"
                   << " [--init-raw-size initial-container-list-size]"
                   << " [--do-non-mt] [--no-mt] [--no-conversion-test]"
                   << " [ / --blitz-test-params / --mid-test-params / --short-test-params]"
                   << " [--show-params]"
                   << std::endl;
         return 1;
      }
   }
   if (n_threads.empty()) {
      n_threads.push_back(1u);
   }
   if (params.n_modules == 0){
     std::cout <<"modules parameter cannot be zero!"<<std::endl;
     return 1;
   }
   if ((params.n_columns == 0) or (params.n_rows == 0)){
     std::cout <<"neither rows nor columns parameters can be zero!"<<std::endl;
     return 1;
   }
   EventList events = makeEvents(params.n_modules, params.n_rows, params.n_columns, params.n_events, params.n_event_indices,
                                 params.min_rois, params.max_rois,
                                 params.min_modules_per_roi, params.max_modules_per_roi,
                                 params.min_hits_per_module,params.max_hits_per_module);

   if (do_conversion_test) {
      // test the container proxies.
      std::unique_ptr<PhaseIIPixelRawDataContainerMT> test_container = makeTestContainer(events, 0u, 0u);
      conversionTest( *test_container);
   }

   // threading test using a single container
   unsigned int total_error_count = do_mt_test ? threadTest<EventStoreMT>(events,
                                                                          params.n_modules,
                                                                          initial_container_list_size,
                                                                          n_queue_slots,
                                                                          n_threads) : 0u;
   if (total_error_count!=0u) return 1;
   if (do_non_mt_test) {
      // reference threading test using one container per roi
      total_error_count = threadTest<EventStoreNonMT>(events,
                                                      params.n_modules,
                                                      initial_container_list_size,
                                                      n_queue_slots,
                                                      n_threads);
   }

   return total_error_count==0u ? 0 : 1;
   }
   catch(std::exception &error) {
      std::cout << "Exception: " << error.what() << std::endl;
      abort();
   }
}
