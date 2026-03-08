/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PhaseIIPixelRawDataContainerCnv.h"

#include "InDetIdentifier/PixelID.h"

#include <memory>

StatusCode PhaseIIPixelRawDataContainerCnv::initialize() {
   ATH_CHECK( PhaseIIPixelRawDataContainerCnvBase::initialize() );
   ATH_CHECK( detStore()->retrieve(m_idHelper, "PixelID") );
   return StatusCode::SUCCESS;
}

PixelRDO_Container_PERS* PhaseIIPixelRawDataContainerCnv::createPersistent(PhaseIIPixelRawDataContainer* transCont) {
  assert( transCont);
  std::unique_ptr< InDetRawDataContainer_p2 >   persObj( std::make_unique<InDetRawDataContainer_p2>() );
  persObj->m_collections.reserve( transCont->size() );
  unsigned int n_rdos=0;

  auto rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(*transCont);
  using PixelRawDataContainerProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataContainerProxy;
  using PixelRawDataProxy = PhaseII::PixelRawDataContainerCollectionTypes<>::RawDataProxy;
  for (PixelRawDataContainerProxy module_rdo_container_proxy : rdo_container_collection_proxy) {
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

  persObj->m_rawdata.reserve( n_rdos );
  for (PixelRawDataContainerProxy module_rdo_container_proxy : rdo_container_collection_proxy) {
     if (!module_rdo_container_proxy.empty()) {
        IdentifierHash id_hash(module_rdo_container_proxy.identifyHash());
        Identifier wafer_id = m_idHelper->wafer_id(id_hash);
        assert( module_rdo_container_proxy.identifyHash() < persObj->m_collections.size());
        assert( persObj->m_rawdata.size() == persObj->m_collections[ module_rdo_container_proxy.identifyHash() ].m_begin );
        for (PixelRawDataProxy rdo_proxy : module_rdo_container_proxy) {
           persObj->m_rawdata.emplace_back();
           persObj->m_rawdata.back().m_rdoId = m_idHelper->pixel_id(wafer_id,
                                                                    rdo_proxy.coordinates()[0],
                                                                    rdo_proxy.coordinates()[1]).get_identifier32().get_compact();
           persObj->m_rawdata.back().m_word = rdo_proxy.dataWord();
        }
        assert( persObj->m_rawdata.size() == persObj->m_collections[ module_rdo_container_proxy.identifyHash() ].m_end);
     }
  }
  return persObj.release();
}

PhaseIIPixelRawDataContainer* PhaseIIPixelRawDataContainerCnv::createTransient(const Token* token) {
  // @TODO add support for  p0 guid  7F2C09B6-0B47-4957-8BBA-EDC665A290AC i.e. initial version Pixel1RawData
  // @TODO add support for  p2 guid  DA76970C-E019-43D2-B2F9-25660DCECD9D i.e. for t/p separated version with InDetRawDataContainer_p1
  static const pool::Guid   TP2_guid("7138342E-0A80-4A32-A387-2842A01C2539"); // for t/p separated version with InDetRawDataContainer_p2
  if( compareClassGuid(token, TP2_guid) ) {
    std::unique_ptr< InDetRawDataContainer_p2 >   persCont( poolReadObject< InDetRawDataContainer_p2 >(token) );
    std::unique_ptr<PhaseIIPixelRawDataContainer> transCont(std::make_unique<PhaseIIPixelRawDataContainer>(m_idHelper->wafer_hash_max(),
                                                                                                           1 /* a single data container */));

    unsigned int n_rdos_total=0u;
    for (  const InDetRawDataCollection_p1 &a_collection : persCont->m_collections) {
       assert( a_collection.m_hashId < transCont->size() );
       assert( a_collection.m_hashId == m_idHelper->wafer_hash(Identifier(a_collection.m_id)));
       unsigned int n_rdos = a_collection.m_end - a_collection.m_begin;

       n_rdos_total += n_rdos;
    }
    assert( persCont->m_collections.size() <= transCont->size());

    using RangeBeginIndex_t = PhaseII::DataRange::RangeBeginIndex_t;
    using RangeSize_t = PhaseII::DataRange::RangeSize_t;
    using ContainerIndex_t = PhaseII::DataRange::ContainerIndex_t;
    constexpr ContainerIndex_t container_i=0u;

    PhaseII::PixelRawDataContainer &rdo_container_dest = transCont->data(container_i);

    rdo_container_dest.reserve(n_rdos_total);
    assert(persCont->m_rawdata.size() == n_rdos_total);

    [[maybe_unused]] unsigned int n_rejected_ranges=0u;
    for (  const InDetRawDataCollection_p1 &a_collection : persCont->m_collections) {

       PhaseII::DataRange new_range( static_cast<RangeBeginIndex_t>(rdo_container_dest.size()),
                                     static_cast<RangeSize_t>(0u) ,
                                     container_i);
       // @TODO should use the ContainerRangeGuard, and the convenience method addDataForModule
       for (unsigned int rdo_i = a_collection.m_begin; rdo_i < a_collection.m_end; ++rdo_i) {
          assert( rdo_i < persCont->m_rawdata.size() );
          assert( rdo_i == rdo_container_dest.size());
          Identifier pixel_id (m_idHelper->pixel_id( persCont->m_rawdata[rdo_i].m_rdoId ));
          assert( m_idHelper->phi_index(pixel_id) < std::numeric_limits<short int>::max());
          assert( m_idHelper->eta_index(pixel_id) < std::numeric_limits<short int>::max());
          rdo_container_dest.emplace_back( std::array<std::int16_t,2>{static_cast<std::int16_t>(m_idHelper->phi_index(pixel_id)),
                                                                      static_cast<std::int16_t>(m_idHelper->eta_index(pixel_id))},
                                      persCont->m_rawdata[rdo_i].m_word);
       }

       // @TODO remove :
       assert( new_range.beginIndex() == a_collection.m_begin);
       assert( rdo_container_dest.size() == a_collection.m_end);

       assert( static_cast<std::size_t>(rdo_container_dest.size() - new_range.beginIndex()) < std::numeric_limits<RangeSize_t>::max() );
       // update the number of elements in this range.
       std::size_t capped_size = rdo_container_dest.size() - new_range.beginIndex();
       if (capped_size >= std::numeric_limits<PhaseII::DataRange::RangeSize_t>::max()) {
          ATH_MSG_WARNING( "Too many RDOs registered for module hash " << a_collection.m_hashId
                           << " : " << capped_size << " !< " << std::numeric_limits<PhaseII::DataRange::RangeSize_t>::max()
                           << ". Elements above maximum possible size will not be accessible.");
          capped_size =std::numeric_limits<PhaseII::DataRange::RangeSize_t>::max();
       }
       new_range.setSize(static_cast<RangeSize_t>( capped_size));

       // register the new hit range
       n_rejected_ranges += !(transCont->registerOrEraseNewData( a_collection.m_hashId, new_range));
       // since only this converter registers ranges in this container, the just added range of elements should never be erased:
       assert( n_rejected_ranges == 0u);

    }
    return transCont.release();
  }
  throw std::runtime_error("Unsupported persistent version in PhaseIIPixelRawDataContainerCnv");
}
