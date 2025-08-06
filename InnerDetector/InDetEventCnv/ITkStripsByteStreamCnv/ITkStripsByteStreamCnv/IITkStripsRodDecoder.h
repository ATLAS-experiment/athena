/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ITkStripsByteStreamCnv/IITkStripsRodDecoder.h
 * @author Daniel.Ignacio.Torres.Arza@cern.ch based in SCT_RawDataByteStreamCnv/ISCT_RodDecoder.h
 *
 * AlgTool class to decode ROB bytestream data into RDO
 */

#ifndef ITKSTRIPSBYTESTREAMCNV_IITKSTRIPS_RODDECODER_H 
#define ITKSTRIPSBYTESTREAMCNV_IITKSTRIPS_RODDECODER_H

#include "GaudiKernel/IAlgTool.h"

#include "InDetRawData/SCT_RDO_Container.h"
#include "ByteStreamData/RawEvent.h"
#include "InDetByteStreamErrors/IDCInDetBSErrContainer.h"

#include "AthAllocators/DataPool.h"

#include <vector>

class StatusCode;
class IdentifierHash;

class IITkStripsRodDecoder : virtual public IAlgTool 
{
 public: 

  /** Creates the InterfaceID and interfaceID() method */
  DeclareInterfaceID(IITkStripsRodDecoder, 1, 0);

  /** Destructor */
  virtual ~IITkStripsRodDecoder() = default;

  /** Fill Collection method */
  virtual StatusCode fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment&,
                                    SCT_RDO_Container&,
                                    IDCInDetBSErrContainer& errs,
                                    DataPool<SCT3_RawData>* dataItemsPool,
                                    const EventContext& ctx,
                                    const std::vector<IdentifierHash>* vecHash = nullptr) const = 0;
};

#endif //ITK_STRIPS_BYTESTREAMCNV_IITK_RODDECODER_H
