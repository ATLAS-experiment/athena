/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialTrackWriter.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "GaudiKernel/IInterface.h"

#include <format>
#include "TTree.h"
#include "TFile.h"


namespace ActsTrk{
MaterialTrackWriter::~MaterialTrackWriter()  = default;

StatusCode MaterialTrackWriter::initialize() {
    // Check the input collection key
    ATH_CHECK(m_materialTrackCollectionKey.initialize());

    m_outputTree = new TTree(m_treeName.value().c_str(), "TTree from MaterialTrackWriter");
    Config_t cfg{};
    cfg.prePostStepInfo = m_prePostStep;
    cfg.surfaceInfo = m_storeSurface;
    cfg.volumeInfo = m_storeVolume;
    cfg.recalculateTotals = m_recalculateTotals;

    m_accessor = ActsPlugins::RootMaterialTrackIo{cfg};
    m_accessor.connectForWrite(*m_outputTree);
    ATH_CHECK(histSvc()->regTree(std::format("/{:}/{:}", m_outStream.value(), m_treeName.value()), m_outputTree));

    ATH_CHECK(m_trackingGeometryTool.retrieve(EnableTool{m_useTrackingGeo}));
    
    return StatusCode::SUCCESS;
}

StatusCode MaterialTrackWriter::execute (const EventContext& ctx) {
    // Get the collection from storegate
    const RecordedMaterialTrackCollection* materialTracks{nullptr};
    ATH_CHECK(SG::get(materialTracks, m_materialTrackCollectionKey, ctx));
    
    const Acts::GeometryContext geoContext = m_useTrackingGeo ?
                                              m_trackingGeometryTool->getGeometryContext(ctx).context()
                                           : Acts::GeometryContext::dangerouslyDefaultConstruct();
    std::unique_lock lock{m_writeMutex};

    // Loop over the material tracks and write them out
    for (const auto& materialTrack : *materialTracks) {
        // write & fill
        m_accessor.write(geoContext, ctx.evt(), materialTrack);
        m_outputTree->Fill();
    }

    // return success
    return StatusCode::SUCCESS;
}
}
