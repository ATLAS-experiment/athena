/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PhaseIIStripRawDataContainerCnv.h"

#include "InDetIdentifier/SCT_ID.h"
#include "InDetEventAthenaPool/SCT3_RawData_p4.h"
#include "InDetEventAthenaPool/InDetRawDataCollection_p1.h"

#include <memory>
#include <stdexcept>

StatusCode PhaseIIStripRawDataContainerCnv::initialize() {
   ATH_CHECK( PhaseIIStripRawDataContainerCnvBase::initialize() );
   ATH_CHECK( detStore()->retrieve(m_idHelper, "SCT_ID") );
   return StatusCode::SUCCESS;
}

StripRDO_Container_PERS* PhaseIIStripRawDataContainerCnv::createPersistent(PhaseIIStripRawDataContainer* transCont) {
  assert( transCont);
  std::unique_ptr< SCT_RawDataContainer_p4 >   persObj( std::make_unique<SCT_RawDataContainer_p4>() );
  persObj->m_collections.reserve( transCont->size() );
  unsigned int n_rdos=0;
  auto rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(*transCont);
  using StripRawDataContainerProxy = PhaseII::StripRawDataContainerCollectionTypes<>::RawDataContainerProxy;
  using StripRawDataProxy = PhaseII::StripRawDataContainerCollectionTypes<>::RawDataProxy;
  // convert element index and count total number of elements in all containers.
  for (StripRawDataContainerProxy module_rdo_container_proxy : rdo_container_collection_proxy) {
     if (!module_rdo_container_proxy.empty()) {
        IdentifierHash id_hash(module_rdo_container_proxy.identifyHash());
        assert( n_rdos + module_rdo_container_proxy.size() < std::numeric_limits<unsigned int>::max());
        persObj->m_collections.emplace_back();
        persObj->m_collections.back().m_id = m_idHelper->wafer_id(id_hash ).get_identifier32().get_compact();
        persObj->m_collections.back().m_hashId = id_hash.value();
        persObj->m_collections.back().m_begin = n_rdos;
        persObj->m_collections.back().m_end = static_cast<unsigned int>(n_rdos+module_rdo_container_proxy.size());
        n_rdos += module_rdo_container_proxy.size();
     }
  }

  // determine the raw data type, which must be the same for the entire collection.
  PhaseII::StripRawDataContainer::ERawDataType type=PhaseII::StripRawDataContainer::UNKNOWN;
  bool is_empty=true;
  for (StripRawDataContainerProxy module_rdo_container_proxy : rdo_container_collection_proxy) {
     if (!module_rdo_container_proxy.empty()) {
        StripRawDataProxy rdo_proxy = module_rdo_container_proxy[0];
        type=rdo_proxy.dataType();
        is_empty=false;
        break;
     }
  }

  if (!is_empty) {
     if (type == PhaseII::StripRawDataContainer::SCT1) {
        persObj->m_rawdata.reserve( n_rdos );
        for (StripRawDataContainerProxy module_rdo_container_proxy : rdo_container_collection_proxy) {
           if (!module_rdo_container_proxy.empty()) {
              IdentifierHash id_hash(module_rdo_container_proxy.identifyHash());
              Identifier wafer_id = m_idHelper->wafer_id(id_hash);
              assert( module_rdo_container_proxy.identifyHash() < persObj->m_collections.size());
              assert( persObj->m_rawdata.size() == persObj->m_collections[ module_rdo_container_proxy.identifyHash() ].m_begin );
              for (StripRawDataProxy rdo_proxy : module_rdo_container_proxy) {
                 persObj->m_rawdata.emplace_back();
                 persObj->m_rawdata.back().m_rdoId = m_idHelper->strip_id(wafer_id,
                                                                          rdo_proxy.coordinates()[0],
                                                                          rdo_proxy.coordinates()[1]).get_identifier32().get_compact();
                 persObj->m_rawdata.back().m_word = rdo_proxy.dataWord();
              }
              assert( persObj->m_rawdata.size() == persObj->m_collections[ module_rdo_container_proxy.identifyHash() ].m_end);
           }
        }
     }
     else if (type == PhaseII::StripRawDataContainer::SCT3) {
        // @TODO support persistification of SCT3 Raw data ? For which data was that used ?
        throw std::runtime_error("Strip data type 3 not implemented in PhaseIIStripRawDataContainer");
     }
     else {
        throw std::runtime_error("Strip data type unhandled by the PhaseIIStripRawDataContainer");
     }
  }
  return persObj.release();
}

PhaseIIStripRawDataContainer* PhaseIIStripRawDataContainerCnv::createTransient() {
  //  @TODO support  p0_guid 7F2C09B6-0B47-4957-8BBA-EDC665A290AC i.e. initial container
  //  @TOOD support TP1_guid DA76970C-E019-43D2-B2F9-25660DCECD9D i.e. t/p separated version with InDetRawDataContainer_p1
  static const pool::Guid   SCT_TP4_guid("6C7540BE-E85C-4777-BC1C-A9FF11460F54"); // for t/p separated version with SCT_RawDataContainer_p4

  if( compareClassGuid(SCT_TP4_guid) ) {
     std::unique_ptr< SCT_RawDataContainer_p4 >   persCont( poolReadObject< SCT_RawDataContainer_p4 >() );
    std::unique_ptr<PhaseIIStripRawDataContainer> transCont(std::make_unique<PhaseIIStripRawDataContainer>(m_idHelper->wafer_hash_max(),
                                                                                                           1 /* one slot */ ));

    PhaseII::StripRawDataContainer::ERawDataType type = PhaseII::StripRawDataContainer::UNKNOWN;
    if (persCont->m_sct3data.size() != 0){
       type = PhaseII::StripRawDataContainer::SCT3;
    }
    else if (persCont->m_rawdata.size() != 0){
       type = PhaseII::StripRawDataContainer::SCT1;
    }
    else {
       throw std::runtime_error("Unhandled persistent SCT raw data container.");
    }

    unsigned int n_rdos_total=0u;
    for (  const InDetRawDataCollection_p1 &a_collection : persCont->m_collections) {
       assert( a_collection.m_hashId < transCont->size() );
       assert( a_collection.m_hashId == m_idHelper->wafer_hash(Identifier(a_collection.m_id)));
       unsigned int n_rdos = a_collection.m_end - a_collection.m_begin;

       assert( n_rdos_total == a_collection.m_begin);
       assert( n_rdos_total+n_rdos == a_collection.m_end);
       n_rdos_total += n_rdos;
    }

    using RangeBeginIndex_t = decltype(PhaseII::DataRange::m_beginIndex);
    using RangeSize_t = decltype(PhaseII::DataRange::m_n);
    using ContainerIndex_t = decltype(PhaseII::DataRange::m_containerIndex);
    constexpr ContainerIndex_t slot_i=0u;

    PhaseII::StripRawDataContainer &rdo_container_dest = transCont->data(slot_i);
    rdo_container_dest.reserve(n_rdos_total);
    assert(persCont->m_rawdata.size() == n_rdos_total);

    [[maybe_unused]] unsigned int n_rejected_ranges=0u;
    if (type==PhaseII::StripRawDataContainer::SCT1) {
       assert(persCont->m_rawdata.size() == n_rdos_total);
       // type 1
       for (  const InDetRawDataCollection_p1 &a_collection : persCont->m_collections) {
          // @TODO should use the ContainerRangeGuard, and the convenience method addDataForModule
          PhaseII::DataRange new_range( static_cast<RangeBeginIndex_t>(rdo_container_dest.size()),
                                        static_cast<RangeSize_t>(0u) ,
                                        slot_i);
          for (unsigned int rdo_i = a_collection.m_begin; rdo_i < a_collection.m_end; ++rdo_i) {
             assert( rdo_i < persCont->m_rawdata.size() );
             Identifier strip_id(persCont->m_rawdata[rdo_i].m_rdoId);
             int strip = m_idHelper->strip( strip_id);
             assert( strip <= std::numeric_limits<short>::max() && strip >= std::numeric_limits<short>::min());
             assert( rdo_i == rdo_container_dest.size());
             rdo_container_dest.emplace_back( std::array<std::int16_t,1>{static_cast<short>(strip)},
                                         persCont->m_rawdata[rdo_i].m_word
                                         );
          }

          // @TODO remove :
          assert( new_range.beginIndex() == a_collection.m_begin);
          assert( rdo_container_dest.size() == a_collection.m_end);

          assert( static_cast<std::size_t>(rdo_container_dest.size() - new_range.beginIndex()) < std::numeric_limits<RangeSize_t>::max() );
          // update the number of elements in this range.
          new_range.m_n  = static_cast<RangeSize_t>( rdo_container_dest.size() - new_range.beginIndex());
          // register the new hit range
          n_rejected_ranges += !(transCont->registerOrEraseNewData( a_collection.m_hashId, new_range));
          // since only this converter registers ranges in this container, the just added range of elements should never be erased:
          assert( n_rejected_ranges == 0u);
       }
       assert( transCont->data(0).size() == n_rdos_total);
       rdo_container_dest.setDataType( type );
    }
    else {
       //type3
       assert(persCont->m_sct3data.size() == n_rdos_total);
       for (  const InDetRawDataCollection_p1 &a_collection : persCont->m_collections) {
          // @TODO should use the ContainerRangeGuard, and the convenience method addDataForModule
          PhaseII::DataRange new_range( static_cast<RangeBeginIndex_t>(rdo_container_dest.size()),
                                        static_cast<RangeSize_t>(0u) ,
                                        slot_i);
          for (unsigned int rdo_i = a_collection.m_begin; rdo_i < a_collection.m_end; ++rdo_i) {
             assert( rdo_i < persCont->m_sct3data.size() );
             const SCT3_RawData_p4 &persObj=persCont->m_sct3data[rdo_i];
             int strip = persObj.getStrip();
             assert( strip <= std::numeric_limits<short>::max() && strip >= std::numeric_limits<short>::min());

             // P: Bits 0-10 for group size, Bits 11-13 for time bin
             // T: Bits 0-10 for group size, Bits 22-24 for time bin
             unsigned int word = static_cast<unsigned int>(persObj.m_word);
             unsigned int group_size = word & 0x7FF;
             unsigned int time_bin  = (word>>11) & 0x7;
             constexpr unsigned int errors =0u;
             assert( rdo_i == rdo_container_dest.size());
             rdo_container_dest.emplace_back( std::array<std::int16_t,1>{static_cast<short>(strip)},
                                         PhaseII::StripRawDataContainer::makeWord(group_size, time_bin,errors)
                                         );
          }
          // @TODO remove :
          assert( new_range.beginIndex() == a_collection.m_begin);
          assert( rdo_container_dest.size() == a_collection.m_end);

          assert( static_cast<std::size_t>(rdo_container_dest.size() - new_range.beginIndex()) < std::numeric_limits<RangeSize_t>::max() );
          // update the number of elements in this range.
          new_range.m_n  = static_cast<RangeSize_t>( rdo_container_dest.size() - new_range.beginIndex());
          // register the new hit range
          n_rejected_ranges += !(transCont->registerOrEraseNewData( a_collection.m_hashId, new_range));
          // since only this converter registers ranges in this container, the just added range of elements should never be erased:
          assert( n_rejected_ranges == 0u);
       }
       assert( transCont->data(0).size() == n_rdos_total);
       rdo_container_dest.setDataType( type );
    }
    return transCont.release();
  }
  throw std::runtime_error("Unsupported persistent version in PhaseIIStripRawDataContainerCnv");
}
