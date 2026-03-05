/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOLS_TGC_RAWDATAPROVIDERTOOLMT_H
#define MUONTGC_CNVTOOLS_TGC_RAWDATAPROVIDERTOOLMT_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "CxxUtils/CachedPointer.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/TgcRdoContainer.h"
#include "MuonRDO/TgcRdo_Cache.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"
#include "MuonTGC_CnvTools/ITGC_RodDecoder.h"
#include "TGC_Hid2RESrcID.h"

namespace Muon {

/** @class TGC_RawDataProviderTool
 *  A tool to decode TGC ROB fragments into TGC RDO.
 *  This version is for athenaMT.
 *
 *  @author Zvi Tarem <zvi@caliper.co.il>
 *  @author Mark Owen <markowen@cern.ch>
 */

class TGC_RawDataProviderTool
    : public extends<AthAlgTool, IMuonRawDataProviderTool> {
   public:
    /** Default constructor */
    using base_class::base_class;
    /** Default destructor */
    virtual ~TGC_RawDataProviderTool() = default;

    /** Standard AlgTool method */
    virtual StatusCode initialize() override;
    /** EventContext ones **/
    using IMuonRawDataProviderTool::convert;
    virtual StatusCode convert(const EventContext&) const override;
    virtual StatusCode convert(const std::vector<IdentifierHash>&,
                               const EventContext&) const override;

   private:
    /** Method that converts the ROBFragments into the passed container */
    StatusCode convertIntoContainer(const ROBFragmentList& vecRobs,
                                    const EventContext& ctx) const;

    /** Function to get the ROB data from a vector of IdentifierHash **/
    ROBFragmentList getROBData(const std::vector<IdentifierHash>& rdoIdhVect,
                               const EventContext& ctx) const;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    /** Decoder for ROB fragment RDO conversion */
    ToolHandle<ITGC_RodDecoder> m_decoder{
        this, "Decoder", "Muon::TGC_RodDecoderReadout/TGC_RodDecoderReadout"};
    /** RDO container key */
    SG::WriteHandleKey<TgcRdoContainer> m_rdoContainerKey{
        this, "RdoLocation", "TGCRDO",
        "Name of the TGCRDO produced by RawDataProvider"};  // MT

    unsigned int m_maxhashtoUse = 0U;  // MT

    /** ID converter */
    TGC_Hid2RESrcID m_hid2re;
    /** Rob Data Provider handle */
    ServiceHandle<IROBDataProviderSvc> m_robDataProvider{
        this, "ROBDataProviderSvc", "ROBDataProviderSvc"};

    SG::ReadCondHandleKey<Muon::TgcCablingMap> m_cablingKey{
        this, "CablingKey", "MuonTgc_CablingMap"};
    // TGC container cache key
    SG::UpdateHandleKey<TgcRdo_Cache> m_rdoContainerCacheKey{
        this, "TgcContainerCacheKey", ""};
    /** EventContext ones **/
    virtual StatusCode convert(const std::vector<uint32_t>&,
                               const EventContext&) const override {
        return StatusCode::FAILURE;
    }
};
}  // namespace Muon

#endif  // MUONTGC_CNVTOOLS_TGC_RAWDATAPROVIDERTOOLMT_H
