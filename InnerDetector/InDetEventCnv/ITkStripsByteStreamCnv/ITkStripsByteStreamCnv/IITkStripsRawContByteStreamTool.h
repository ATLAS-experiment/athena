/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ITkStripsByteStreamCnv/IITkStripsRawContByteStreamTool.h
 * @author Shaun Roe based on code by Susumu Oda
 * @date April 2024
 */

#ifndef  ITkStripsByteStreamCnv_IITkStripsRawContByteStreamTool_h
#define  ITkStripsByteStreamCnv_IITkStripsRawContByteStreamTool_h

#include "GaudiKernel/IAlgTool.h"
#include "InDetRawData/SCT_RDO_Container.h" //typedef

/** 
 * @class IITkStripsRawContByteStreamTool
 * @brief Interface for Athena Algorithm Tool to provide conversion from ITk Strips RDO container to ByteStream.
 *
 */
class IITkStripsRawContByteStreamTool : virtual public IAlgTool 
{
 public:

  /** Creates the InterfaceID and interfaceID() method */
  DeclareInterfaceID(IITkStripsRawContByteStreamTool, 1, 0);

  /** Destructor */
  virtual ~IITkStripsRawContByteStreamTool() = default;

  /** Convert method: container type is same as for SCT */
  virtual StatusCode convert(const SCT_RDO_Container* itkStripsRDOCont) const = 0;
};

#endif 