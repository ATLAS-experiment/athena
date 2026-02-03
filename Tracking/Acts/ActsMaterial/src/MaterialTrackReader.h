/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_MATERIALTRACKREADER_H
#define ACTSMATERIAL_MATERIALTRACKREADER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsMaterial/RecordedMaterialTrackCollection.h"

#include <ActsPlugins/Root/RootMaterialTrackIo.hpp>

#include "TChain.h"

#include <mutex>


namespace ActsTrk {

    /// @class MaterialTrackReader
    ///
    /// @brief Writes out RecordedMaterialTrackCollection to a root file
    ///
    /// It writes out a MaterialTrack which is usually generated from
    /// MaterialTrackRecorder G4UA

    class MaterialTrackReader : public AthReentrantAlgorithm  {
        public:
            MaterialTrackReader(const std::string &name, ISvcLocator *pSvcLocator);
            virtual StatusCode initialize() override;
            virtual StatusCode execute (const EventContext& ctx) const override;
            virtual StatusCode finalize() override;
            virtual ~MaterialTrackReader();

        private:
            /// mutex used to protect multi-threaded reads
            mutable std::mutex m_readMutex;
            /// The number of events
            std::size_t m_events = 0;
            /// The batch size (number of track per events)
            std::size_t m_batchSize = 0;
            /// The list of input filenames
            Gaudi::Property<std::vector<std::string>> m_fileNames{this, "FileNames", {}, "The list of input file names"};
            /// The name of the input tree
            Gaudi::Property<std::string> m_treeName{this, "TreeName", "material-tracks", "The input tree name"};
            /// Read surface information for the root file
            Gaudi::Property<bool> m_readCachedSurfaceInformation{this, "ReadCachedSurface", false, "Read surface information for the root file"};
            /// The read - write payload
            ActsPlugins::RootMaterialTrackIo m_accessor;
            /// The input chain of entries
            std::unique_ptr<TChain> m_inputChain ATLAS_THREAD_SAFE = {};
            /// The RecordedMaterialTrackCollection to write
            SG::WriteHandleKey<RecordedMaterialTrackCollection> m_materialTrackCollectionKey {this, "MaterialTrackCollectionKey", "InputMaterialTracks", "Name of the RecordedMaterialTrackCollection"};
        };
}

#endif

