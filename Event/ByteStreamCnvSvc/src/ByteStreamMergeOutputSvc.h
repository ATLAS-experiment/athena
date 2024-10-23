/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMMERGEOUTPUTSVC_H
#define BYTESTREAMMERGEOUTPUTSVC_H

/** @file ByteStreamMergeOutputSvc.h
 *  @brief This file contains the class definition for the ByteStreamMergeOutputSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "ByteStreamCnvSvc/ByteStreamInputSvc.h"
#include "ByteStreamCnvSvc/IByteStreamOutputSvc.h"
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
   virtual ~ByteStreamMergeOutputSvc();

   virtual StatusCode initialize() override;
   /// Implementation of the IByteStreamOutputSvc interface methods.
   virtual bool putEvent(const RawEvent* re) override;
   virtual bool putEvent(const RawEvent* re, const EventContext& ctx) override;

private:
   uint32_t reducedROBid(uint32_t);

   Gaudi::Property<std::string> m_inSvcName;
   ByteStreamInputSvc*  m_inSvc;
   ServiceHandle<IByteStreamOutputSvc> m_outSvc{this, "ByteStreamOutputSvc", {}};

   Gaudi::Property<std::string> m_bsOutputStreamName; //!< stream name for multiple output
   Gaudi::Property<bool> m_overwriteHeader;
};

#endif
