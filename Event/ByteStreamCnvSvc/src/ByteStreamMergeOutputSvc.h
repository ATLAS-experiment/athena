/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVC_BYTESTREAMMERGEOUTPUTSVC_H
#define BYTESTREAMCNVSVC_BYTESTREAMMERGEOUTPUTSVC_H

/** @file ByteStreamMergeOutputSvc.h
 *  @brief This file contains the class definition for the ByteStreamMergeOutputSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaBaseComps/AthService.h"
#include "ByteStreamCnvSvcBase/IByteStreamInputSvc.h"
#include "ByteStreamCnvSvcBase/IByteStreamOutputSvc.h"
#include "GaudiKernel/ServiceHandle.h"

/** @class ByteStreamMergeOutputSvc
 *  @brief This class provides the services for merging FullEventFragment with existing bytestream input.
 *  Mostly meant for adding new L2+EF results
 **/
class ByteStreamMergeOutputSvc : public extends<AthService, IByteStreamOutputSvc> {
public:
   /// Constructors:
   ByteStreamMergeOutputSvc(const std::string& name, ISvcLocator* svcloc);

   /// Destructor.
   virtual ~ByteStreamMergeOutputSvc() = default;

   virtual StatusCode initialize() override;
   /// Implementation of the IByteStreamOutputSvc interface methods.
   virtual bool putEvent(const RawEvent* re) override;
   virtual bool putEvent(const RawEvent* re, const EventContext& ctx) override;

private:
   uint32_t reducedROBid(uint32_t);

   ServiceHandle<IByteStreamInputSvc> m_inSvc{this, "ByteStreamInputSvc", {}};
   ServiceHandle<IByteStreamOutputSvc> m_outSvc{this, "ByteStreamOutputSvc", {}};

   Gaudi::Property<std::string> m_bsOutputStreamName{this, "BSOutputStreamName", {}, "stream name for multiple output"};
   Gaudi::Property<bool> m_overwriteHeader{this, "overWriteHeader", false};
};

#endif // BYTESTREAMCNVSVC_BYTESTREAMMERGEOUTPUTSVC_H
