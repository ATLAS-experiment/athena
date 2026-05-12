/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMDT_CONVTOOLSR4_MDTRDOTOMDTPREPDATATOOL_H
#define MUONMDT_CONVTOOLSR4_MDTRDOTOMDTPREPDATATOOL_H

#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "MuonMDT_CnvTools/IMDT_RDO_Decoder.h"

#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonCablingData/TwinTubeMap.h"

#include "MuonRDO/MdtCsmContainer.h"

#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/MdtTwinDriftCircleContainer.h"

#include "xAODMuonPrepData/MdtDriftCircleAuxContainer.h"
#include "xAODMuonPrepData/MdtTwinDriftCircleAuxContainer.h"

#include "xAODMuonViews/FillContainer.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

namespace MuonR4 {
    /** @class MdtRdoToMdtPrepDataTool
     * Tool to produce MDT PRDs for Run 4
    */
    class MdtRdoToMdtPrepDataTool : public extends<AthAlgTool, Muon::IMuonRdoToPrepDataTool> {
    public:
        using base_class::base_class;

        /** default destructor */
        ~MdtRdoToMdtPrepDataTool() = default;
        /** standard Athena-Algorithm method */
        virtual StatusCode initialize() override;
        /** Decode method - declared in Muon::IMuonRdoToPrepDataTool*/
        virtual StatusCode decode(const EventContext& ctx, const std::vector<IdentifierHash>& idVect) const override;
        // new decode method for Rob based readout
        virtual StatusCode decode(const EventContext& ctx, const std::vector<uint32_t>& robIds) const override;
        // method to return empty container 
        virtual StatusCode provideEmptyContainer(const EventContext& ctx) const override;

    protected: 
    
        // helper struct to pass write handle across the tol=ol
        struct HandleCache{
            HandleCache() = default;
            using PrdCont_t = xAOD::FillContainer<xAOD::MdtDriftCircleContainer,
                                                  xAOD::MdtDriftCircleAuxContainer>;
            using PrdTwinCont_t = xAOD::FillContainer<xAOD::MdtTwinDriftCircleContainer,
                                                      xAOD::MdtTwinDriftCircleAuxContainer>;
            PrdCont_t prdContainer{};
            PrdTwinCont_t prdTwinContainer{};
            const ActsTrk::GeometryContext* gctx{nullptr};

            bool isValid{false};
        };

        HandleCache setupHandleCache(const EventContext& ctx) const;
    private:
        // processes CSM for tube and twin tube 
        void processDigit(const EventContext& ctx, std::unique_ptr<MdtDigit> digit, HandleCache& hCache) const;
        StatusCode processCsm(const EventContext& ctx, const MdtCsm* rdoColl, HandleCache& hCache) const;
        StatusCode processCsmTwin(const EventContext& ctx, const MdtCsm* rdoColl, HandleCache& hCache) const;

        StatusCode processPRDHashes(const EventContext& ctx, const std::vector<IdentifierHash>& chamberHashInRobs, HandleCache& hCache) const;

        /// tools and services 
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        ToolHandle<IMdtCalibrationTool> m_calibrationTool{this, "CalibrationTool", "MdtCalibrationTool"};
        ToolHandle<Muon::IMDT_RDO_Decoder> m_mdtDecoder{this, "Decoder", "Muon::MdtRDO_Decoder/MdtRDO_Decoder"};
        const MuonGMR4::MuonDetectorManager* m_detMgrR4{nullptr};

        // read handles
        SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "Stored alignment"};
        SG::ReadHandleKey<MdtCsmContainer> m_rdoContainerKey{this, "RDOContainer", "MDTCSM"};
        
        // read condition handle keys
        SG::ReadCondHandleKey<Muon::TwinTubeMap> m_twinTubeKey{this, "TwinTubeKey", "MdtTwinTubeMap"};
        SG::ReadCondHandleKey<MuonMDT_CablingMap> m_cablingKey{this, "ReadKey", "MuonMDT_CablingMap", "Key of MuonMDT_CablingMap"};

        // properties
        Gaudi::Property<int>  m_adcCut{this, "AdcCut", 50, "Minimal cut on the adc to convert it into a prepdata object"};
        Gaudi::Property<bool> m_calibratePrepData{this, "CalibratePrepData", true};  //!< toggle on/off calibration of MdtPrepData
        Gaudi::Property<bool> m_useTwin{this, "UseTwin", true};
        Gaudi::Property<bool> m_discardSecondaryHitTwin{this, "DiscardSecondaryHitTwin", false};

        // xAOD PRDs
        SG::WriteHandleKey<xAOD::MdtDriftCircleContainer> m_xAODKey{this, "xAODKey", "xMdtDriftCircles", "If empty, do not produce xAOD, otherwise this is the key of the output xAOD MDT PRD container"};
        SG::WriteHandleKey<xAOD::MdtTwinDriftCircleContainer> m_xAODTwinKey{this, "xAODTwinKey", "xMdtTwinDriftCircles", "If empty, do not produce xAOD, otherwise this is the key of the output xAOD MDT PRD container"};
    };
}  // namespace Muon

#endif
