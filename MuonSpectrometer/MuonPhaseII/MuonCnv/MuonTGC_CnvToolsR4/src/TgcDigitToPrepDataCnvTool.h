/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTGC_CNVTOOLSR4_TGCDIGITPREPDATACNVTOOL_H
#define MUONTGC_CNVTOOLSR4_TGCDIGITPREPDATACNVTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include <ActsGeometryInterfaces/GeometryContext.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

#include <MuonDigitContainer/TgcDigitContainer.h>
#include <xAODMuonPrepData/TgcStripContainer.h>


namespace MuonR4{
    /** @brief Conversion tool from Tgc digits -> Tgc xAOD uncalibrated measurements. The tool
     *         shall be seen as a placeholder for the actual RDO -> prepdata conversion tool used
     *         in Phase-II which does not exist yet. */
    class TgcDigitToPrepDataCnvTool : public extends<AthAlgTool, Muon::IMuonRdoToPrepDataTool> {
        public:
            using base_class::base_class;


            StatusCode initialize() override final;

            StatusCode decode(const EventContext& ctx, 
                              const std::vector<IdentifierHash>& idVect) const override final;
            /** not implemented... */
            StatusCode decode(const EventContext& ctx,
                              const std::vector<uint32_t>& robIds) const override final;

            StatusCode provideEmptyContainer(const EventContext& ctx) const override final;
        private:
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            SG::ReadHandleKey<TgcDigitContainer> m_readKey{this, "ReadKey", "TGC_DIGITS", "Digit container to retrieve"};

            SG::WriteHandleKey<xAOD::TgcStripContainer> m_writeKey{this, "WriteKey", "xTgcStrips", "Output container"};

            const MuonGMR4::MuonDetectorManager* m_detMgr{};
    };
}


#endif