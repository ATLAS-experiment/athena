/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ITkStripsByteStreamCnv/IITkStripRawContByteStreamTool.h
 * 
 * Athena Algorithm Tool to fill Collections of SCT RDO Containers.
 */

#ifndef ITKSTRIP_RAWDATABYTESTREAMCNV_IITKSTRIPRAWDATAPROVIDERTOOL_H
#define ITKSTRIP_RAWDATABYTESTREAMCNV_IITKSTRIPRAWDATAPROVIDERTOOL_H

#include "ByteStreamData/RawEvent.h"
#include "InDetByteStreamErrors/IDCInDetBSErrContainer.h"
#include "InDetRawData/SCT_RDO_Container.h"

#include "GaudiKernel/IAlgTool.h"

#include "AthAllocators/DataPool.h"
/** 
 * @class IITkStripRawDataProviderTool
 *
 * @brief Interface for Athena Algorithm Tool to fill Collections of SCT RDO Containers.
 *
 * The class inherits from IAlgTool.
 */
class IITkStripRawDataProviderTool : virtual public IAlgTool
{
 public:

  /** Creates the InterfaceID and interfaceID() method */
  DeclareInterfaceID(IITkStripRawDataProviderTool, 1, 0);

  /** Destructor */
  virtual ~IITkStripRawDataProviderTool() = default;

  /** Main decoding methods */
  virtual StatusCode convert(std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>&,
                             SCT_RDO_Container&,
                             IDCInDetBSErrContainer& errs,
                             DataPool<SCT3_RawData>* dataItemsPool,
                             const EventContext& ctx) const = 0;

};

#endif // ITKSTRIP_BYTESTREAMCNV_ITKSTRIPRAWDATAPROVIDERTOOL_H
