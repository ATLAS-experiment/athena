/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialTrackWriter.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "GaudiKernel/IInterface.h"

#include "TTree.h"
#include "TFile.h"


ActsTrk::MaterialTrackWriter::MaterialTrackWriter(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator),
    m_accessor({m_prePostStep, m_storeSurface, m_storeVolume, m_recalculateTotals})
{}

ActsTrk::MaterialTrackWriter::~MaterialTrackWriter()
{
    if (m_outputFile != nullptr)
        m_outputFile->Close();
}

StatusCode ActsTrk::MaterialTrackWriter::initialize()
{
    // Check the tracking geometry
    ATH_CHECK(m_trackingGeometrySvc.retrieve());

    // Check the input collection key
    ATH_CHECK(m_materialTrackCollectionKey.initialize());

    // Setup ROOT I/O
    m_outputFile = TFile::Open(m_fileName.value().c_str(), "RECREATE");
    if (m_outputFile == nullptr) {
        ATH_MSG_ERROR("Could not open '" + m_fileName + "'");
        return StatusCode::FAILURE;
    }

    m_outputFile->cd();
    m_outputTree =
      new TTree(m_treeName.value().c_str(), "TTree from MaterialTrackWriter");
    if (m_outputTree == nullptr) {
        ATH_MSG_ERROR("Could not create TTree '" + m_treeName + "'");
        return StatusCode::FAILURE;
    }

    // Connect the branches
    m_accessor.connectForWrite(*m_outputTree);

    return StatusCode::SUCCESS;
}

StatusCode ActsTrk::MaterialTrackWriter::finalize()
{
    // write the tree and close the file
    ATH_MSG_INFO("Writing ROOT output File : " << m_outputFile);

    m_outputFile->cd();
    m_outputTree->Write();
    m_outputFile->Close();

    return StatusCode::SUCCESS;
}

StatusCode
ActsTrk::MaterialTrackWriter::execute (const EventContext& ctx) const
{
    // Get the collection from storegate
    SG::ReadHandle<ActsTrk::RecordedMaterialTrackCollection> materialTracks(m_materialTrackCollectionKey, ctx);
    if (!materialTracks.isValid()) {
        ATH_MSG_ERROR("Failed to read " << m_materialTrackCollectionKey.key());
        return StatusCode::FAILURE;
    }

    const ActsTrk::GeometryContext& geoContext{m_trackingGeometrySvc->getNominalContext()};

    std::lock_guard<std::mutex> lock(m_writeMutex);

    // Loop over the material tracks and write them out
    for (const auto& materialTrack : *materialTracks) {
        // write & fill
        m_accessor.write(geoContext.context(), ctx.evt(), materialTrack);
        m_outputTree->Fill();
    }

    // return success
    return StatusCode::SUCCESS;
}
