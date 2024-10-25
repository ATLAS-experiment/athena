/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVCBASE_IBYTESTREAMEVENTACCESS_H
#define BYTESTREAMCNVSVCBASE_IBYTESTREAMEVENTACCESS_H

#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/IInterface.h"

/** @class IByteStreamEventAccess
  * @brief Interface for accessing raw data.
  */
class IByteStreamEventAccess: virtual public IInterface {
public:
   /// Gaudi interface id
   DeclareInterfaceID(IByteStreamEventAccess, 2, 0);

   /// pure virtual method for accessing RawEventWrite
   virtual RawEventWrite* getRawEvent() = 0;

};

#endif
