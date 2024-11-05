/* Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration */
#ifndef BYTESTREAMCNVSVC_IBYTESTREAMOUTPUTSVC_H
#define BYTESTREAMCNVSVC_IBYTESTREAMOUTPUTSVC_H

/** @file IByteStreamOutputSvc.h
 *  @brief This file contains the interface for the ByteStreamOutputSvc classes.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/EventContext.h"

#include <string>


/** @class IByteStreamOutputSvc
 *  @brief This class provides the interface to services to write bytestream
 *         data.
 *  The concrete class can provide RAW event to a file, transient store, or
 *  through network.
 **/
class IByteStreamOutputSvc : virtual public IInterface {
 public:
  DeclareInterfaceID(IByteStreamOutputSvc, 1, 0);

  /// virtual method for writing the event
  virtual bool putEvent(const RawEvent* re) = 0;

  /// context-aware method for writing the event
  virtual bool putEvent(const RawEvent* re, const EventContext& ctx) = 0;
};

#endif  // BYTESTREAMCNVSVC_BYTESTREAMOUTPUTSVC_H
