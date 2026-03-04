/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_MATERIALTRACKWRITER_H
#define ACTSMATERIAL_MATERIALTRACKWRITER_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometry/RecordedMaterialTrackCollection.h"

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

    class MaterialTrackWriter : public AthHistogramAlgorithm  {
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;
            virtual StatusCode initialize() override;
            virtual StatusCode execute () override;           
            virtual ~MaterialTrackWriter();

        private:
            /// mutex used to protect multi-threaded writes
            mutable std::mutex m_writeMutex;
            /// The output file name
            Gaudi::Property<std::string> m_outStream{this, "OutStream", "ACTSMATERIALWRITER", "The output file name"};
            /// The output file name
            Gaudi::Property<std::string> m_treeName{this, "TreeName", "materialTracks", "The output tree name"};
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

            Gaudi::Property<bool> m_useTrackingGeo{this, "useTrackingGeometry", false, 
                                                    "Use the tracking geometry to retrieve the geometry context"};
            /// The tracking geometry service to retrive the geometry context
            PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool",
                                                                                    "", "The ACTS geometry service for the geometry context"};
            using Config_t = ActsPlugins::RootMaterialTrackIo::Config;
            /// The read - write payload
            ActsPlugins::RootMaterialTrackIo m_accessor{Config_t{}};
            /// The output file and tree
           
            TTree* m_outputTree{};

        };
}

#endif
