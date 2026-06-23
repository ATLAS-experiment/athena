/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONG4R4_MUONSENSITIVEDETECTORTOOL_H
#define MUONG4R4_MUONSENSITIVEDETECTORTOOL_H


#include <GeoPrimitives/GeoPrimitives.h>
///
#include <G4AtlasTools/SensitiveDetectorBase.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>


namespace MuonG4R4 {
    /** @brief Baseline MuonSensitive detector tool to store the sim hits. 
     *         The individual implementations need to inherit from this tool.
     *         The tool takes care to write the SimHit container into the 
     *         G4 HitCollection and to retrieve the detector manager and 
     *         to initialize the alignment key pointing to the alignment
     *         store with all local -> global transformations of each sensitive
     *         volume */
    class MuonSensitiveDetectorTool : public SensitiveDetectorBase {

        public:
            /** @brief Use the standard AthAlgTool constructors */
            using SensitiveDetectorBase::SensitiveDetectorBase;
            /** @brief Default the destructor */
            ~MuonSensitiveDetectorTool()=default;
            /** @copydoc SensitiveDetectorBase::initialize */
            virtual StatusCode initialize() override final;
            /** @copydoc SensitiveDetectorBase::SetupEvent */
            virtual StatusCode SetupEvent(HitCollectionMap& hitCollections) override final;
            /** @copydoc SensitiveDetectorBase::Gather */
            virtual StatusCode Gather(HitCollectionMap& hitCollections) override final;
        protected:
            /** @brief The muon detector manager to retrieve the sensitive elements */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            /** @brief: Key to the alignment / transform store per event. */
            Gaudi::Property<std::string> m_alignStoreKey{this, "AlignStoreKey", ""};
    };
}
#endif