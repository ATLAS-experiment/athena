// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1RESULTBYTESTREAM_CTPRESULTBYTESTREAMTOOL_H
#define TRIGT1RESULTBYTESTREAM_CTPRESULTBYTESTREAMTOOL_H

// Trigger includes
#include "TrigT1ResultByteStream/IL1TriggerByteStreamTool.h"
#include "xAODTrigger/CTPResult.h"

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthAlgTool.h"

/** @class CTPResultByteStreamTool
*  @brief Tool for converting CTP ROB from BS to xAOD::CTPResult and from xAOD::CTPResult to BS
*  (IL1TriggerByteStreamTool interface)
**/
class CTPResultByteStreamTool : public extends<AthAlgTool, IL1TriggerByteStreamTool> {

public:

    // To use base class functions (e.g. constructor)
    using base_class::base_class;

    // ------------------------- IAlgTool methods --------------------------------
    virtual StatusCode initialize() override;

    // ------------------------- IL1TriggerByteStreamTool methods ----------------

    // BS->xAOD conversion
    virtual StatusCode convertFromBS(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vrobf, const EventContext& eventContext) const override;

    // xAOD->BS conversion
    virtual StatusCode convertToBS(std::vector<OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment*>& vrobf, const EventContext& eventContext) override;

    // Declare ROB IDs for conversion
    virtual const std::vector<uint32_t>& robIds() const override {return m_robIds.value();}

private:

    // ------------------------- Private types -----------------------------------
    // Struct holding the status words and rob/rod error flags
    struct DataStatus {
      bool rob_error {false};
      bool rod_error {false};
      uint32_t status_word {0};
      uint32_t status_info {0};
    };

    // ------------------------- Data handles ------------------------------------
    SG::ReadHandleKey<xAOD::CTPResult>  m_inKeyCTPResult  {this, "CTPResultReadKey", "", "Read handle key to CTPResult for conversion to ByteStream"};
    SG::WriteHandleKey<xAOD::CTPResult>  m_outKeyCTPResult  {this, "CTPResultWriteKey", "", "Write handle key to CTPResult for conversion from ByteStream"};

    // ------------------------- Other properties --------------------------------
    Gaudi::Property<std::vector<uint32_t>> m_robIds {this, "ROBIDs", {}, "List of ROB IDs required for conversion to/from xAOD"};
    Gaudi::Property<uint16_t> m_detEvType {this, "DetEvType", 1, "Detector event type to write when converting to ByteStream"};

}; // class CTPResultByteStreamTool

#endif // TRIGT1RESULTBYTESTREAM_CTPRESULTBYTESTREAMTOOL_H
