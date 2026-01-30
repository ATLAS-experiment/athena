/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_MATERIALTRACKWRITER_H
#define ACTSMATERIAL_MATERIALTRACKWRITER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsMaterial/RecordedMaterialTrackCollection.h"

#include <ActsPlugins/Root/RootMaterialTrackIo.hpp>

#include "TTree.h"
#include "TFile.h"

#include <mutex>


namespace ActsTrk {

    /// @class MaterialTrackWriter
    ///
    /// @brief Writes out RecordedMaterialTrackCollection to a root file
    ///
    /// It writes out a MaterialTrack which is usually generated from
    /// MaterialTrackRecorder G4UA

    class MaterialTrackWriter : public AthReentrantAlgorithm  {
        public:
            MaterialTrackWriter(const std::string &name, ISvcLocator *pSvcLocator);
            virtual StatusCode initialize() override;
            virtual StatusCode execute (const EventContext& ctx) const override;
            virtual StatusCode finalize() override;
            virtual ~MaterialTrackWriter();

        private:
            /// mutex used to protect multi-threaded writes
            mutable std::mutex m_writeMutex;
            /// The output file name
            Gaudi::Property<std::string> m_fileName{this, "FileName", "MaterialTracks.root", "The output file name"};
            /// The output file name
            Gaudi::Property<std::string> m_treeName{this, "TreeName", "material-tracks", "The output tree name"};
            /// Write the surface to which the material step correpond
            Gaudi::Property<bool> m_storeSurface{this, "StoreSurface", false, "Write the surface to which the material step correpond"};
            /// Write the volume to which the material step correpond
            Gaudi::Property<bool> m_storeVolume{this, "StoreVolume", false, "Write the volume to which the material step correpond"};
            /// Write out pre and post step (for G4), otherwise central step position
            Gaudi::Property<bool> m_prePostStep{this, "PrePostStep", false, "Write out pre and post step (for G4), otherwise central step position"};
            /// Re-calculate total values from individual steps (for cross-checks)
            Gaudi::Property<bool> m_recalculateTotals{this, "RecalculateTotals", false, "Re-calculate total values from individual steps (for cross-checks)"};

            /// The RecordedMaterialTrackCollection to read
            SG::ReadHandleKey<RecordedMaterialTrackCollection> m_materialTrackCollectionKey {this, "MaterialTrackCollectionKey", "OutputMaterialTracks", "Name of the RecordedMaterialTrackCollection"};

            /// The tracking geometry service to retrive the geometry context
            ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc","ActsTrackingGeometrySvc", "The ACTS geometry service for the geometry context"};
            /// The read - write payload
            mutable ActsPlugins::RootMaterialTrackIo m_accessor ATLAS_THREAD_SAFE;
            /// The output file and tree
            TFile* m_outputFile{};
            mutable TTree* m_outputTree ATLAS_THREAD_SAFE = {};

        };
}

#endif
