/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMdtRdoToPrepDataToolMT_H
#define MUONMdtRdoToPrepDataToolMT_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "MuonMDT_CnvTools/IMDT_RDO_Decoder.h"

#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonCablingData/TwinTubeMap.h"

#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRDO/MdtCsmContainer.h"

#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/MdtTwinDriftCircleContainer.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

class MdtDigit;

namespace Muon {
    /** @class MdtRdoToPrepDataToolMT
     * Tool to produce MDT PRDs
    */

    class MdtRdoToPrepDataToolMT : public extends<AthAlgTool, IMuonRdoToPrepDataTool> {
    public:
        using base_class::base_class;

        /** default destructor */
        virtual ~MdtRdoToPrepDataToolMT() = default;

        /** standard Athena-Algorithm method */
        virtual StatusCode initialize() override;

        /** Decode method - declared in Muon::IMuonRdoToPrepDataTool*/
        virtual StatusCode decode(const EventContext& ctx, const std::vector<IdentifierHash>& idVect) const override;
        virtual StatusCode provideEmptyContainer(const EventContext& ctx) const override;
        // new decode method for Rob based readout
        virtual StatusCode decode(const EventContext& ctx, const std::vector<uint32_t>& robIds) const override;

    protected:
        void printPrepDataImpl(const Muon::MdtPrepDataContainer* mdtPrepDataContainer) const;

        /// method to get the twin tube 2nd coordinate
        Muon::MdtDriftCircleStatus getMdtTwinPosition(const MdtDigit& prompt_digit, const MdtDigit& twin_digit, double& radius,
                                                      double& errRadius, double& zTwin, double& errZTwin, bool& twinIsPrompt) const;


        /// Helper struct to parse the event data around the tool
        struct ConvCache {
            ConvCache(const Muon::IMuonIdHelperSvc* idHelperSvc);
            /// Creates a new MdtPrepDataCollection, if it's neccessary
            /// and also possible. Nullptr is returned if the collection
            /// cannot be modified
            MdtPrepDataCollection* createCollection(const Identifier& id);
            /// Copy the non-empty collections into the created prd container.
            StatusCode finalize(MsgStream& msg);

            Muon::MdtPrepDataContainer* legacyPrd{nullptr};
            xAOD::MdtDriftCircleContainer* xAODPrd{nullptr};
            xAOD::MdtTwinDriftCircleContainer* xAODTwinPrd{nullptr};

            const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
            /// Detector manager from the conditions store
            const MuonGM::MuonDetectorManager* legacyDetMgr{nullptr};
            /// Detector manger from R4
            const MuonGMR4::MuonDetectorManager* r4DetMgr{nullptr};
            /// Acts Geometry context
            const ActsTrk::GeometryContext* gctx{nullptr};
            /// Pointer to the map having the mapping of twin tube pairs
            const TwinTubeMap* twinTubeMap{nullptr};

            /// Flag set to indicate that the complete validation was successful
            bool isValid{false};
            std::vector<std::unique_ptr<MdtPrepDataCollection>> addedCols{};
        };

        StatusCode processCsm(const EventContext& ctx, ConvCache& mdtPrepDataContainer, 
                              const MdtCsm* rdoColl) const;

        StatusCode processCsmTwin(const EventContext& ctx, 
                                  ConvCache& mdtPrepDataContainer,  
                                  const MdtCsm* rdoColll) const;
        
        /// Creates the PRD object
        std::unique_ptr<MdtPrepData> createPrepData(const MdtCalibInput& calibInput,
                                                    const MdtCalibOutput& calibOutput,
                                                    ConvCache& cache) const;
        /// Creates the prep data container to be written
        ConvCache setupMdtPrepDataContainer(const EventContext& ctx) const;
        
        /// Loads the input RDO container from StoreGate
        const MdtCsmContainer* getRdoContainer(const EventContext& ctx) const;

        void processPRDHashes(const EventContext& ctx, ConvCache& mdtPrepDataContainer,
                                const std::vector<IdentifierHash>& chamberHashInRobs) const;

        bool handlePRDHash(const EventContext& ctx, ConvCache& mdtPrepDataContainer, 
                           IdentifierHash rdoHash) const;

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        /// MDT calibration service
        ToolHandle<IMdtCalibrationTool> m_calibrationTool{this, "CalibrationTool", "MdtCalibrationTool"};

        
        Gaudi::Property<bool> m_useNewGeo{this, "UseR4DetMgr", false,
                                         "Switch between the legacy and the new geometry"};

        const MuonGMR4::MuonDetectorManager* m_detMgrR4{nullptr};
        SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "Stored alignment"};


        /// MdtPrepRawData containers
        SG::WriteHandleKey<Muon::MdtPrepDataContainer> m_mdtPrepDataContainerKey{this, "OutputCollection", "MDT_DriftCircles"};

        SG::ReadHandleKey<MdtCsmContainer> m_rdoContainerKey{this, "RDOContainer", "MDTCSM"};
        

        /** member variables for algorithm properties: */
        Gaudi::Property<int>  m_adcCut{this, "AdcCut", 50, 
                                        "Minimal cut on the adc to convert it into a prepdata object"};
        Gaudi::Property<bool> m_calibratePrepData{this, "CalibratePrepData", true};  //!< toggle on/off calibration of MdtPrepData
        Gaudi::Property<bool> m_decodeData{this, "DecodeData", true};  //!< toggle on/off the decoding of MDT RDO into MdtPrepData
        bool m_sortPrepData = false;                                   //!< Toggle on/off the sorting of the MdtPrepData

        ToolHandle<Muon::IMDT_RDO_Decoder> m_mdtDecoder{this, "Decoder", "Muon::MdtRDO_Decoder/MdtRDO_Decoder"};

        bool m_BMGpresent{false};
        int m_BMGid{-1};

        // + TWIN TUBE
        Gaudi::Property<bool> m_useTwin{this, "UseTwin", true};
        Gaudi::Property<bool> m_discardSecondaryHitTwin{this, "DiscardSecondaryHitTwin", false};

        // - TWIN TUBE
        SG::ReadCondHandleKey<TwinTubeMap> m_twinTubeKey{this, "TwinTubeKey", "MdtTwinTubeMap"};

        std::unordered_set<Identifier> m_DeadChannels{};
        void initDeadChannels(const MuonGM::MdtReadoutElement* mydetEl);

        SG::ReadCondHandleKey<MuonMDT_CablingMap> m_readKey{this, "ReadKey", "MuonMDT_CablingMap", "Key of MuonMDT_CablingMap"};

        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                         "Key of input MuonDetectorManager condition data"};
        /// This is the key for the cache for the MDT PRD containers, can be empty
        SG::UpdateHandleKey<MdtPrepDataCollection_Cache> m_prdContainerCacheKey{this, "MdtPrdContainerCacheKey", "",
                                                                                "Optional external cache for the MDT PRD container"};

        // xAOD PRDs
        SG::WriteHandleKey<xAOD::MdtDriftCircleContainer> m_xAODKey{this, "xAODKey", "", "If empty, do not produce xAOD, otherwise this is the key of the output xAOD MDT PRD container"};
        SG::WriteHandleKey<xAOD::MdtTwinDriftCircleContainer> m_xAODTwinKey{this, "xAODTwinKey", "", "If empty, do not produce xAOD, otherwise this is the key of the output xAOD MDT PRD container"};

    };
}  // namespace Muon

#endif
