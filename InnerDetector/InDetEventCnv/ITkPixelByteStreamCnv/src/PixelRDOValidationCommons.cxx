/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PixelRDOValidationCommons.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include <cstddef>

StatusCode PixelRDOValidationCommons::initialize()
{
  ATH_CHECK(detStore()->retrieve(m_idHelper, "PixelID"));
  return StatusCode::SUCCESS;
}

StatusCode PixelRDOValidationCommons::execute(const EventContext& ctx) const
{
  size_t nb_mon_rdos = 0, nb_ref_rdos = 0;

  TSortedRDOContainer reference_rdos;
  ATH_CHECK(makeSortedReferenceRDOs(ctx, reference_rdos, nb_ref_rdos));

  TSortedRDOContainer monitored_rdos;
  ATH_CHECK(makeSortedMonitoredRDOs(ctx, monitored_rdos, nb_mon_rdos));

  if (nb_ref_rdos == 0) {
    ATH_MSG_ERROR("empty reference container");
    return StatusCode::FAILURE;
  }
  if (nb_mon_rdos == 0) {
    ATH_MSG_ERROR("empty monitored container");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("nb modules: ref: " << reference_rdos.size() << ", mon: "
    << monitored_rdos.size());
  ATH_MSG_DEBUG("nb RDOs: ref: " << nb_ref_rdos << ", mon: " << nb_mon_rdos);

  size_t err_module_missing_from_ref = 0;
  size_t err_module_missing_from_mon = 0;
  size_t err_rdo_missing_from_ref = 0;
  size_t err_rdo_missing_from_mon = 0;
  size_t err_rdo_contents = 0;

  // Copy reference module IDs to check if some are not in monitored
  std::set<Identifier> ref_modules;
  for (auto const & [module_id, _]: reference_rdos) {
    ref_modules.insert(module_id);
  }

  for (auto const & [module_id, mon_module_rdos]: monitored_rdos) {
    auto const ref_module_rdos_it = reference_rdos.find(module_id); 
    if (ref_module_rdos_it == reference_rdos.end()) {
      ATH_MSG_DEBUG("module missing from ref: " << module_id);
      err_module_missing_from_ref += 1;
      continue;
    }

    // This modules exists in mon
    ref_modules.erase(module_id);

    auto const & ref_module_rdos = ref_module_rdos_it->second;

    // Copy reference RDO IDs to check if some are not in monitored
    std::set<Identifier> ref_rdo_ids;
    for (auto const & [ref_rdo_id, _]: ref_module_rdos) {
      ref_rdo_ids.insert(ref_rdo_id);
    }
  
    for (auto const & [mon_rdo_id, mon_rdo_word]: mon_module_rdos) {
      auto const ref_rdo_it = ref_module_rdos.find(mon_rdo_id);
      if (ref_rdo_it == ref_module_rdos.end()) {
        ATH_MSG_DEBUG("RDO missing from ref"
          << ": module ID: " << module_id
          << ", RDO ID: " << mon_rdo_id);
        err_rdo_missing_from_ref += 1;
        continue;
      }

      // This RDO exists in mon
      ref_rdo_ids.erase(mon_rdo_id);

      auto const & ref_rdo = *ref_rdo_it;
      if (mon_rdo_word != ref_rdo.second) {
        ATH_MSG_DEBUG("RDO words mismatch"
          << ": module ID: " << module_id
          << ", RDO id: " << mon_rdo_id 
          << ", ref: " << ref_rdo.second
          << ", mon: " << mon_rdo_word);
        err_rdo_contents += 1;
      }
    }

    // All remaining RDO IDs in ref_rdo_ids were not in monitored
    for (auto const & rdo_id: ref_rdo_ids) {
      ATH_MSG_DEBUG("RDO missing from mon"
        << ": module ID: " << module_id
        << ", RDO ID:" << rdo_id);
      err_rdo_missing_from_mon += 1;
    }		
  }

  // All remaining module in ref_modules were not in monitored
  for (auto const & module_id: ref_modules) {
    ATH_MSG_DEBUG("module missing from mon: " << module_id);
    err_module_missing_from_mon += 1;
  }

  if (err_module_missing_from_mon || err_module_missing_from_ref
      || err_rdo_missing_from_mon || err_rdo_missing_from_ref
      || err_rdo_contents)
  {
    ATH_MSG_ERROR("error summary"
      << " (total collections: " << reference_rdos.size()
      << ", total RDOs: ref: " << nb_ref_rdos << ", mon: " << nb_mon_rdos << ")"
      << ": module missing from ref: " << err_module_missing_from_ref
      << ", module missing from mon: " << err_module_missing_from_mon
      << ", RDO missing from ref: " << err_rdo_missing_from_ref
      << ", RDO missing from mon: " << err_rdo_missing_from_mon
      << ", RDO contents: " << err_rdo_contents
    );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG("containers match");
  }

  return StatusCode::SUCCESS;
}

StatusCode PixelRDOValidationCommons::makeSortedRDOs(
  SG::ReadHandle<PixelRDO_Container> & sg_rdos_handle,
  TSortedRDOContainer & sorted_rdos, size_t & nb_sorted_rdos) const
{
  sorted_rdos.clear();
  nb_sorted_rdos = 0;

  for (const auto* coll : *sg_rdos_handle) {
    if (coll == nullptr || coll->size() == 0) continue;
    std::map<Identifier, unsigned int> & module_rdos = sorted_rdos[coll->identify()];
    for (const PixelRDORawData* rdo : *coll) {
      module_rdos[rdo->identify()] = rdo->getWord();
      nb_sorted_rdos += 1;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode PixelRDOValidationCommons::makeSortedPhaseIIRDOs(
  SG::ReadHandle<PhaseIIPixelRawDataContainer> & sg_rdos_handle,
  TSortedRDOContainer & sorted_rdos, size_t & nb_sorted_rdos) const
{
  sorted_rdos.clear();
  nb_sorted_rdos = 0;

  using PixelRawDataContainerProxy =
    PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy;
  using PixelRawDataProxy = PhaseII::PixelRawDataTypeTraits<>::RawDataProxy;
  auto pixel_rdo_container_collection_proxy =
    PhaseII::makeRawDataCollectionProxy(*sg_rdos_handle);

  for (PixelRawDataContainerProxy module_rdo_container_proxy :
      pixel_rdo_container_collection_proxy) {
    if (module_rdo_container_proxy.empty()) {
      continue;
    }
    const IdentifierHash module_hash = module_rdo_container_proxy.identifyHash();
    //The Phase-II EDM does not store the module ID: retrieve it from its hash
    const Identifier module_id = m_idHelper->wafer_id(module_hash);
    std::map<Identifier, unsigned int> & module_rdos = sorted_rdos[module_id];

    for (PixelRawDataProxy rdo: module_rdo_container_proxy) {
      //The Phase-II EDM does not store the RDO ID: make it from its consituents
      const int16_t row = rdo.coordinates()[0];
      const int16_t col = rdo.coordinates()[1];
      Identifier rdo_id = m_idHelper->pixel_id(module_id, row, col);
      module_rdos[rdo_id] = rdo.dataWord();
      nb_sorted_rdos += 1;
    }
  }
  return StatusCode::SUCCESS;
}
