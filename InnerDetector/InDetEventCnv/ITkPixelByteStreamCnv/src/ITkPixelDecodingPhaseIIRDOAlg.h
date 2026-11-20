/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_DECODINGPHASEIIRDOALG_H
#define ITKPIXEL_DECODINGPHASEIIRDOALG_H

/**
  * @file ITkPixelByteStreamCnv/ITkPixelDecodingPhaseIIRDOAlg.h
  * @author Ondra Kovanda, Goetz Gaycken, Fabrice Balli
  * @date June 2026
  * @brief Bytestream decoding class for phase-2 pixel RDO EDM
  */

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "InDetIdentifier/PixelID.h"
#include "Identifier/Identifier.h"
#include "InDetRawData/PhaseIIPixelRawDataContainerMT.h"
#include "InDetRawData/PixelRDO_Collection.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"
#include <chrono>
#include <cstdint> //for uint8_t etc.

/**
 * @class ITkPixelDecodingPhaseIIRDOAlg
 * This algorithm translates the event-formatted
 * bytestream into PhaseIIPixelRawDataContainer RDOs.
 * This is the main pixel decoding algorithm intended
 * for Run4, and the parallel using the old RDO will
 * be phased out.
 */


class ITkPixelDecodingPhaseIIRDOAlg : public AthReentrantAlgorithm 
{
  public:

    ITkPixelDecodingPhaseIIRDOAlg(const std::string &name, ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
  
    virtual StatusCode execute (const EventContext& ctx) const override;
  
    virtual StatusCode finalize() override;

  
  private:

    typedef std::vector< std::vector<uint32_t >> ITkPacketCollection;

    ServiceHandle<IROBDataProviderSvc> m_robDataProviderSvc
        { this, "ROBDataProvider", "ROBDataProviderSvc" };

    SG::ReadCondHandleKey<ITkPixelCablingData> m_pixelCablingKey
        {this, "PixelCablingKey", "ITkPixelCablingData", "Cond Key of Pixel Cabling"};

    SG::WriteHandleKey<PhaseIIPixelRawDataContainer> m_pixelRDOKey
        {this,    "pixelRDOKey", "PixelRDOs", "StoreGate Key of Pixel RDOs"};

    Gaudi::Property<bool> m_FailOnDecodingError{this, "FailOnDecodingError",
        false, "Whether the algortihm should fail on decoding error"};
    
    const PixelID* m_idHelper{};

    const Gaudi::Property<uint32_t> m_n_rdos_est{this, "nRDOs", 1200000,
        "Estimated number of RDOs per event"};

};

namespace ITkPixelDecodingPhaseIIRDO{
    struct ScopedTimer {
        std::string m_label;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
        MsgStream& m_msg_source;

        ScopedTimer(std::string lbl, MsgStream& src) : m_label(std::move(lbl)), m_start(std::chrono::high_resolution_clock::now()), m_msg_source(src) {}
        ~ScopedTimer() {
            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();
            m_msg_source << MSG::DEBUG << m_label << " took " << ms << " ms" << endmsg;
        }

    };
}

#endif
