/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TGC_RodDecoderReadout.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef MUONTGC_CNVTOOLS_TGC_RODDECODERREADOUT_H
#define MUONTGC_CNVTOOLS_TGC_RODDECODERREADOUT_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonTGC_CnvTools/ITGC_RodDecoder.h"

class TgcRdo;
class TgcIdHelper;

namespace Muon {

/** @class TGC_RodDecoderReadout
 *  A tool to decode a TGC ROB fragment written in the readout format into TGC
 * RDO.
 *  @author Zvi Tarem <zvi@caliper.co.il>
 *
 *  The previous class name was TGC_RodDecoder.
 */

class TGC_RodDecoderReadout : public extends<AthAlgTool, ITGC_RodDecoder> {
   public:
    /** Default constructor */
    using base_class::base_class;
    /** default destructor */
    virtual ~TGC_RodDecoderReadout();

    /** Standard AlgTool method */
    virtual StatusCode initialize() override;
    /** Standard AlgTool method */
    virtual StatusCode finalize() override;
    /** Convert ROBFragment to RDO */
    virtual StatusCode fillCollection(
        const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
        TgcRdoContainer& rdoIdc, const EventContext& ctx) const override;

   private:
    /** TGC ID helper */
    const TgcIdHelper* m_tgcIdHelper{nullptr};

    /** Retrieve header of ROBFragment */
    void getCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                       TgcRdo* rdo) const;
    /** Convert data contents of ROBFragment to RDO */
    void byteStream2Rdo(OFFLINE_FRAGMENTS_NAMESPACE::PointerType bs,
                        TgcRdo* rdo, uint32_t source_id) const;
    void byteStreamSrod2Rdo(OFFLINE_FRAGMENTS_NAMESPACE::PointerType bs,
                            TgcRdo* rdo, uint32_t source_id,
                            uint32_t ndata) const;
    /** Show status words */
    void showStatusWords(const uint32_t source_id, const uint16_t rdoId,
                         const int idHash, const uint32_t nstatus,
                         const uint32_t* status) const;

    /** Flag for showStatusWords */
    Gaudi::Property<bool> m_showStatusWords{this, "ShowStatusWords", false};
    /** Flag for skipping coincidence objects (for TrigT1TGC running on data) */
    Gaudi::Property<bool> m_skipCoincidence{this, "SkipCoincidence", false};

    // counters to see how often we use the cache for the raw data collections
    mutable std::atomic<unsigned int> m_nCache = 0;
    mutable std::atomic<unsigned int> m_nNotCache = 0;
};
}  // namespace Muon

#endif  // MUONTGC_CNVTOOLS_TGC_RODDECODERREADOUT_H
