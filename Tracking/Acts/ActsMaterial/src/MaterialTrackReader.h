/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_MATERIALTRACKREADER_H
#define ACTSMATERIAL_MATERIALTRACKREADER_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometry/RecordedMaterialTrackCollection.h"
///
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

    class MaterialTrackReader : public AthAlgorithm  {
        public:

            using AthAlgorithm::AthAlgorithm;
            virtual StatusCode initialize() override;
            virtual StatusCode execute () override;
            virtual StatusCode finalize() override;
            virtual unsigned cardinality() const override { return 1; }
            virtual ~MaterialTrackReader();

        private:
            /// The number of events in the processed
            Gaudi::Property<std::size_t> m_maxEvents{this, "maxEvents", std::numeric_limits<std::size_t>::max()};
            /// The number of events in the file to skip
            Gaudi::Property<std::size_t> m_skipEvents{this, "skipEvents", 0ul};
            /// The batch size (number of track per events)
            Gaudi::Property<std::size_t>  m_batchSize{this, "batchSize", 1000ul};
            /// The number of entries in the tree to read
            std::size_t m_nTreeEntries{0ul};
            /// The current processed tree entry
            std::size_t m_currEntry{0ul};
            /// The number of processed tree events
            std::size_t m_procEvents{0ul};
            /// The list of input filenames
            Gaudi::Property<std::vector<std::string>> m_fileNames{this, "FileNames", {}, "The list of input file names"};
            /// The name of the input tree
            Gaudi::Property<std::string> m_treeName{this, "TreeName", "material-tracks", "The input tree name"};
            /// Read surface information for the root file
            Gaudi::Property<bool> m_readCachedSurfaceInformation{this, "ReadCachedSurface", false, "Read surface information for the root file"};
            /// The read - write payload
            ActsPlugins::RootMaterialTrackIo m_accessor{{false, m_readCachedSurfaceInformation}};
            /// The input chain of entries
            std::unique_ptr<TChain> m_inputChain{};
            /// The RecordedMaterialTrackCollection to write
            SG::WriteHandleKey<RecordedMaterialTrackCollection> m_materialTrackCollectionKey {this, "MaterialTrackCollectionKey", "InputMaterialTracks", "Name of the RecordedMaterialTrackCollection"};
        };
}

#endif

