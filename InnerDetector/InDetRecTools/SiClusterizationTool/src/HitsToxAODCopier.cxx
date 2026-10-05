/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "HitsToxAODCopier.h"
namespace InDet {
HitsToxAODCopier::HitsToxAODCopier(const std::string& name,
                                   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}


StatusCode HitsToxAODCopier::initialize() {
  ATH_CHECK(m_pixelRDOTool.retrieve());
  ATH_CHECK(m_pixelRdoContainerKey.initialize());
  ATH_CHECK(m_pixelOutputKey.initialize());
  ATH_CHECK(m_stripRdoContainerKey.initialize());
  ATH_CHECK(m_stripOutputKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_pixelIdHelper, "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_stripIdHelper, "SCT_ID"));

  return StatusCode::SUCCESS;
}

StatusCode HitsToxAODCopier::execute(const EventContext& context) const {
  ATH_CHECK(exportPixel(context));
  ATH_CHECK(exportStrip(context));
  return StatusCode::SUCCESS;
}

StatusCode HitsToxAODCopier::exportPixel(const EventContext& context) const {
  SG::ReadHandle<PixelRDO_Container> rdoContainer =
      SG::makeHandle(m_pixelRdoContainerKey, context);
  ATH_CHECK(rdoContainer.isValid());

  auto output = std::make_unique<xAOD::BaseContainer>();
  auto outputAux = std::make_unique<xAOD::AuxContainerBase>();
  output->setStore(outputAux.get());
  static const SG::Accessor<int> col("col");
  static const SG::Accessor<int> row("row");
  static const SG::Accessor<int> tot("tot");
  static const SG::Accessor<int> eta_module("eta_module");
  static const SG::Accessor<int> phi_module("phi_module");
  static const SG::Accessor<int> layer_disk("layer_disk");
  static const SG::Accessor<int> barrel_ec("barrel_ec");
  static const SG::Accessor<uint64_t> id("detid");

  for (const auto *collection : *rdoContainer) {
    const InDetDD::SiDetectorElement* element =
        m_pixelRDOTool->checkCollection(*collection, context);
    if (element != nullptr) {
      std::vector<InDet::UnpackedPixelRDO> hits =
          m_pixelRDOTool->getUnpackedPixelRDOs(*collection, *m_pixelIdHelper,
                                               element, context);
      for (auto hit : hits) {
        auto* item = new SG::AuxElement();
        output->push_back(item);
        col(*item) = hit.COL;
        row(*item) = hit.ROW;
        tot(*item) = hit.TOT;
        eta_module(*item) = m_pixelIdHelper->eta_module(hit.ID);
        phi_module(*item) = m_pixelIdHelper->phi_module(hit.ID);
        layer_disk(*item) = m_pixelIdHelper->layer_disk(hit.ID);
        barrel_ec(*item)  = m_pixelIdHelper->barrel_ec(hit.ID);
        id(*item) = hit.ID.get_compact();
      }
    }
  }
  auto outputHandle = SG::makeHandle(m_pixelOutputKey, context);
  ATH_CHECK(outputHandle.record(std::move(output), std::move(outputAux)));

  return StatusCode::SUCCESS;
}


StatusCode HitsToxAODCopier::exportStrip(const EventContext& context) const {
  SG::ReadHandle<SCT_RDO_Container> rdoContainer =
      SG::makeHandle(m_stripRdoContainerKey, context);
  ATH_CHECK(rdoContainer.isValid());

  auto output = std::make_unique<xAOD::BaseContainer>();
  auto outputAux = std::make_unique<xAOD::AuxContainerBase>();
  output->setStore(outputAux.get());
  static const SG::Accessor<int> strip("strip");
  static const SG::Accessor<int> side("side");
  static const SG::Accessor<int> eta_module("eta_module");
  static const SG::Accessor<int> phi_module("phi_module");
  static const SG::Accessor<int> layer_disk("layer_disk");
  static const SG::Accessor<int> barrel_ec("barrel_ec");
  static const SG::Accessor<uint64_t> id("detid");

  for ( const auto *collection: *rdoContainer) {
    if ( collection == nullptr) continue;
    // const IdentifierHash idHash = rdos->identifyHash();
    for ( const auto *hit: *collection) {
        auto* item = new SG::AuxElement();
        output->push_back(item);
        strip(*item) = m_stripIdHelper->strip(hit->identify());
        side(*item) = m_stripIdHelper->side(hit->identify());
        eta_module(*item) = m_stripIdHelper->eta_module(hit->identify());
        phi_module(*item) = m_stripIdHelper->phi_module(hit->identify());
        layer_disk(*item) = m_stripIdHelper->layer_disk(hit->identify());
        barrel_ec(*item)  = m_stripIdHelper->barrel_ec(hit->identify());
        id(*item) = hit->identify().get_compact();
    }
  }

  auto outputHandle = SG::makeHandle(m_stripOutputKey, context);
  ATH_CHECK(outputHandle.record(std::move(output), std::move(outputAux)));
  return StatusCode::SUCCESS;
}

}  // namespace InDet