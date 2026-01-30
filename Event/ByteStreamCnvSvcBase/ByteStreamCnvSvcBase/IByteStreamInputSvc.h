/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVC_BYTESTREAMINPUTSVC_H
#define BYTESTREAMCNVSVC_BYTESTREAMINPUTSVC_H

/** @file IByteStreamInputSvc.h
 *  @brief This file contains the interface for the ByteStreamInputSvc classes.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/IInterface.h"


/** @class IByteStreamInputSvc
 *  @brief This class provides the interface to services to read bytestream data.
 *  The concrete class can provide Raw event from a file, transient store, or through network.
 **/
class IByteStreamInputSvc : virtual public IInterface {
public:
  DeclareInterfaceID(IByteStreamInputSvc, 1, 0);

  /// virtual method for advance to the next event
  virtual const RawEvent* nextEvent() = 0;
  virtual const RawEvent* previousEvent() = 0;
  virtual void setEvent(void* /*data*/, unsigned int /*status*/) {}
  /// virtual method for accessing the current event
  virtual const RawEvent* currentEvent() const = 0;
  /// virtual method for accessing the current event status
  virtual unsigned int currentEventStatus() const;
  virtual std::pair<long,std::string> getBlockIterator(const std::string& /* file */);
  virtual void closeBlockIterator(bool);
  virtual bool ready();
  virtual StatusCode generateDataHeader(); 
  virtual long positionInBlock();
  virtual void validateEvent();
};

inline unsigned int IByteStreamInputSvc::currentEventStatus() const {
  return 0;
}

// Virtual methods needed for file input
inline std::pair<long,std::string> IByteStreamInputSvc::getBlockIterator(const std::string& /* file */) {return std::make_pair(-1,"GUID");}
inline void IByteStreamInputSvc::closeBlockIterator(bool) {}
inline bool IByteStreamInputSvc::ready() {return false;}
inline StatusCode IByteStreamInputSvc::generateDataHeader() {return StatusCode::SUCCESS;}
inline long IByteStreamInputSvc::positionInBlock() {return -1;}
inline void IByteStreamInputSvc::validateEvent() {}
#endif
