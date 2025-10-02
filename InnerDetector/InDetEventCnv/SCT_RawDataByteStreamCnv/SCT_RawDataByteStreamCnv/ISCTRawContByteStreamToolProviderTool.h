#ifndef ISCTRAWCONTBYTESTREAMTOOLPROVIDERTOOL_H
#define ISCTRAWCONTBYTESTREAMTOOLPROVIDERTOOL_H

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IAlgTool.h"

class ISCTRawContByteStreamTool;

class ISCTRawContByteStreamToolProviderTool: virtual public IAlgTool
{
public:
   DeclareInterfaceID(ISCTRawContByteStreamToolProviderTool, 1, 0);

   virtual const ISCTRawContByteStreamTool &getTool() const = 0;
};

#endif
