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
#include "MuonDigitContainer/MdtDigit.h"

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
            /** @brief Abrivate the digit pointer */
        using DigitPtr_t = std::unique_ptr<MdtDigit>;
        using TwinDigit_t = std::array<DigitPtr_t, 2>;
        /** @brief Store the digits per chamber to ensure sorting */
        using DigitVec_t = std::vector<DigitPtr_t>;

        /** @brief Helper cache to carry the data containers needed for the
         *         dedcoding around */
        struct HandleCache{
            HandleCache() = default;
            /** @brief Use the fill container backend to have a valid container for filling */
            using PrdCont_t = xAOD::FillContainer<xAOD::MdtDriftCircleContainer,
                                                  xAOD::MdtDriftCircleAuxContainer>;
            using PrdTwinCont_t = xAOD::FillContainer<xAOD::MdtTwinDriftCircleContainer,
                                                      xAOD::MdtTwinDriftCircleAuxContainer>;
            /** @brief The container to hold all 1D tube measurements */
            PrdCont_t prdContainer{};
            /** @brief The container to hold the twin prep data measurements */
            PrdTwinCont_t prdTwinContainer{};
            /** @brief Geometry context to align the tubes within ATLAS */
            const ActsTrk::GeometryContext* gctx{nullptr};
            /** @brief Cabling map to tell which tube is short circuted at the HV
             *         side with another one to form a twin tube pair */
            const Muon::TwinTubeMap* twinTubeMap{};
            /** @brief Flag stating whether the cache is successfully initialized or not */
            bool isValid{false};

            /** @brief Temporary collection to be cleaned afterwards */
            std::vector<DigitVec_t> digitsForCnv{};
            /** @brief Temporary collection  of twin digits for conversion */
            std::map<IdentifierHash, std::vector<TwinDigit_t>> twinDigitsForCnv{};
            
        };

        HandleCache setupHandleCache(const EventContext& ctx) const;
    private:
        /** @brief Process a digit to be converted to a 1D measurement
         *  @param ctx: EventContext to access the calibration constants
         *  @param digit: Pointer to the digit for conversion
         *  @param hCache: Data cache handling the container to which the measurement is appended */
        void processDigit(const EventContext& ctx, DigitPtr_t&& digit, HandleCache& hCache) const;
        
        void processDigit(const EventContext& ctx, TwinDigit_t&& digit, HandleCache& cache) const;
        

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
        // xAOD PRDs
        SG::WriteHandleKey<xAOD::MdtDriftCircleContainer> m_xAODKey{this, "xAODKey", "xMdtDriftCircles", "If empty, do not produce xAOD, otherwise this is the key of the output xAOD MDT PRD container"};
        SG::WriteHandleKey<xAOD::MdtTwinDriftCircleContainer> m_xAODTwinKey{this, "xAODTwinKey", "xMdtTwinDriftCircles", "If empty, do not produce xAOD, otherwise this is the key of the output xAOD MDT PRD container"};
    };
}  // namespace Muon

#endif
