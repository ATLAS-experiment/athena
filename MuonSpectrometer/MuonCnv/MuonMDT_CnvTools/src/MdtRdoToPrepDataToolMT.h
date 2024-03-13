/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMdtRdoToPrepDataToolMT_H
#define MUONMdtRdoToPrepDataToolMT_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonMDT_CnvTools/IMDT_RDO_Decoder.h"
#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRDO/MdtCsmContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"

class MdtDigit;
class MdtCalibHit;

namespace MuonGM {
    class MdtReadoutElement;
}

namespace Muon {
    /** @class MdtRdoToPrepDataToolMT
     * Tool to produce MDT PRDs
    */

    class MdtRdoToPrepDataToolMT : public extends<AthAlgTool, IMuonRdoToPrepDataTool> {
    public:
        MdtRdoToPrepDataToolMT(const std::string&, const std::string&, const IInterface*);

        /** default destructor */
        virtual ~MdtRdoToPrepDataToolMT() = default;

        /** standard Athena-Algorithm method */
        virtual StatusCode initialize() override;

        /** Decode method - declared in Muon::IMuonRdoToPrepDataTool*/
        virtual StatusCode decode(const EventContext& ctx, std::vector<IdentifierHash>& idVect, std::vector<IdentifierHash>& selectedIdVect) const override;
        virtual StatusCode provideEmptyContainer(const EventContext& ctx) const override;
        // new decode method for Rob based readout
        virtual StatusCode decode(const EventContext& ctx, const std::vector<uint32_t>& robIds) const override;

        // dump methods for debugging
        virtual void printInputRdo(const EventContext& ctx) const override;

        virtual void printPrepData(const EventContext& ctx) const override;
 
    protected:
        void printPrepDataImpl(const Muon::MdtPrepDataContainer* mdtPrepDataContainer) const;

        /// method to get the twin tube 2nd coordinate
        Muon::MdtDriftCircleStatus getMdtTwinPosition(const MdtDigit& prompt_digit, const MdtDigit& twin_digit, double& radius,
                                                      double& errRadius, double& zTwin, double& errZTwin, bool& twinIsPrompt,
                                                      const MuonGM::MuonDetectorManager* muDetMgr) const;

        // decode method for Rob based readout
        StatusCode decode(const EventContext& ctx, const std::vector<IdentifierHash>& multiLayerHashInRobs) const;

        /// Helper struct to steer which collections were added by
        /// this tool and which already existed before hand
        struct ModfiablePrdColl {
            ModfiablePrdColl() = default;
            ModfiablePrdColl(Muon::MdtPrepDataContainer* cont) : prd_cont{cont} {}
            /// Creates a new MdtPrepDataCollection, if it's neccessary
            /// and also possible. Nullptr is returned if the collection
            /// cannot be modified
            MdtPrepDataCollection* createCollection(const Identifier& id, const MdtIdHelper& id_helper, MsgStream& msg);
            /// Copy the non-empty collections into the created prd container. Fill the id_hash vector with
            /// the corresponding hashes
            StatusCode finalize(std::vector<IdentifierHash>& id_hash, MsgStream& msg);

            Muon::MdtPrepDataContainer* prd_cont{nullptr};

            using PrdCollMap = std::map<IdentifierHash, std::unique_ptr<MdtPrepDataCollection>>;
            PrdCollMap addedCols{};
        };

        StatusCode processCsm(const EventContext& ctx, ModfiablePrdColl& mdtPrepDataContainer, xAOD::MdtDriftCircleContainer* xAODMdtPrepDataContainer, const MdtCsm* rdoColl,
                              const MuonGM::MuonDetectorManager* muDetMgr) const;

        StatusCode processCsmTwin(const EventContext& ctx, ModfiablePrdColl& mdtPrepDataContainer, xAOD::MdtDriftCircleContainer* xAODMdtPrepDataContainer, const MdtCsm* rdoColll,
                                  const MuonGM::MuonDetectorManager* muDetMgr) const;
        
        /// Creates the PRD object
        std::unique_ptr<MdtPrepData> createPrepData(const MdtCalibInput& calibInput,
                                                    const MdtCalibOutput& calibOutput) const;
        
        /// Creates the xAOD PRD object
        void createxAODPrepData(const MdtCalibInput& calibInput,
                                                    const MdtCalibOutput& calibOutput, 
                                                    xAOD::MdtDriftCircleContainer* xAODMdtPrepDataContainer) const;

        /// Creates the prep data container to be written
        ModfiablePrdColl setupMdtPrepDataContainer(const EventContext& ctx) const;
        
        /// Creates the xAOD PRD container to be written
        xAOD::MdtDriftCircleContainer* setupxAODMdtPrepDataContainer(SG::WriteHandle<xAOD::MdtDriftCircleContainer>& outputContainer) const;
        
        /// Is the identifier disabled due to BMG cut outs
        bool deadBMGChannel(const Identifier& channelId) const;

        /// Loads the input RDO container from StoreGate
        const MdtCsmContainer* getRdoContainer(const EventContext& ctx) const;

        void processPRDHashes(const EventContext& ctx, ModfiablePrdColl& mdtPrepDataContainer,
                                xAOD::MdtDriftCircleContainer* xAODMdtPrepDataContainer,
                                const std::vector<IdentifierHash>& chamberHashInRobs) const;

        bool handlePRDHash(const EventContext& ctx, ModfiablePrdColl& mdtPrepDataContainer, 
                                xAOD::MdtDriftCircleContainer* xAODMdtPrepDataContainer,
                                IdentifierHash rdoHash) const;

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        /// MDT calibration service
        ToolHandle<IMdtCalibrationTool> m_calibrationTool{this, "CalibrationTool", "MdtCalibrationTool"};

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
        Gaudi::Property<bool> m_useAllBOLTwin{this, "UseAllBOLTwin", false};
        Gaudi::Property<bool> m_twinCorrectSlewing{this, "TwinCorrectSlewing", false};
        Gaudi::Property<bool> m_discardSecondaryHitTwin{this, "DiscardSecondaryHitTwin", false};
        int m_twin_chamber[2][3][36]{};
        int m_secondaryHit_twin_chamber[2][3][36]{};
        // - TWIN TUBE

        std::map<Identifier, std::set<Identifier>> m_DeadChannels{};
        void initDeadChannels(const MuonGM::MdtReadoutElement* mydetEl);

        SG::ReadCondHandleKey<MuonMDT_CablingMap> m_readKey{this, "ReadKey", "MuonMDT_CablingMap", "Key of MuonMDT_CablingMap"};

        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                         "Key of input MuonDetectorManager condition data"};
        /// This is the key for the cache for the MDT PRD containers, can be empty
        SG::UpdateHandleKey<MdtPrepDataCollection_Cache> m_prdContainerCacheKey{this, "MdtPrdContainerCacheKey", "",
                                                                                "Optional external cache for the MDT PRD container"};

        // xAOD PRDs
        SG::WriteHandleKey<xAOD::MdtDriftCircleContainer>   m_mdtxAODKey  {this, "MdtxAODKey", "", "If empty, do not produce xAOD, otherwise this is the key of the output xAOD MDT PRD container"};
    };
}  // namespace Muon

#endif
