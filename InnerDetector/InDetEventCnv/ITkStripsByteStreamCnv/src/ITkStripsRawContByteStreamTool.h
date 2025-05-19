/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITkStripsRawDataByteStreamCnv_ITkStripsRawContByteStreamTool_h
#define ITkStripsRawDataByteStreamCnv_ITkStripsRawContByteStreamTool_h

#include "AthenaBaseComps/AthAlgTool.h"
//#include "ITkStripsByteStreamCnv/IITkStripsRawContByteStreamTool.h"
#include "SCT_RawDataByteStreamCnv/ISCTRawContByteStreamTool.h"

#include "ByteStreamCnvSvcBase/FullEventAssembler.h"
#include "ByteStreamCnvSvcBase/IByteStreamCnvSvc.h"

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "ITkStripCabling/IITkStripCablingTool.h"

class IITkStripsRodEncoder;

class SCT_ID;

/** 
 * @class ITkStripsRawContByteStreamTool
 * 
 * @brief Athena Algorithm Tool to provide conversion from ITkStrips RDO container to ByteStream.
 *
 * Conversion from ITkStrips RDO container to ByteStream, and fill it in RawEvent.
 *
 * Contains convert method that maps ROD ID's to vectors of RDOs in those RODs, then
 * loops through the map, using RodEncoder to fill data for each ROD in turn.
 */
class 
//ITkStripsRawContByteStreamTool : public extends<AthAlgTool, IITkStripsRawContByteStreamTool> {
ITkStripsRawContByteStreamTool : public extends<AthAlgTool, ISCTRawContByteStreamTool> {
 public:

  /** Constructor */
  ITkStripsRawContByteStreamTool(const std::string& type, const std::string& name, const IInterface* parent);
  
  /** Destructor */
  virtual ~ITkStripsRawContByteStreamTool() = default;

  /** Initialize */
  virtual StatusCode initialize() override;

  /** Finalize */
  virtual StatusCode finalize() override;

  /** 
   * @brief Main Convert method 
   *
   * Maps ROD ID's to vectors of RDOs in those RODs, then loops through the map, 
   * using RodEncoder to fill data for each ROD in turn.
   *
   * @param itkRDOCont ITk RDO Container of Raw Data Collections.
   * */
  virtual StatusCode convert(const SCT_RDO_Container* itkRDOCont) const override;
  
 private:
  ServiceHandle<IByteStreamCnvSvc> m_byteStreamCnvSvc
  { this, "ByteStreamCnvSvc", "ByteStreamCnvSvc" };

  /** Algorithm Tool to decode ROB bytestream data into RDO. */ 
  ToolHandle<IITkStripsRodEncoder> m_encoder{this, "Encoder", "ITkStripsRodEncoder", "ITkStrips ROD Encoder for RDO to BS conversion"};

  /** Providing mappings of online and offline identifiers and also serial numbers. */ 
  ToolHandle<IITkStripCablingTool> m_cabling{this, "ITkStripsCablingTool", "ITkStripCablingTool", "Tool to retrieve ITk Strips Cabling"};

  /** Identifier helper class for the SCT subdetector that creates compact Identifier objects and 
      IdentifierHash or hash IDs. Also allows decoding of these IDs. */ 
  const SCT_ID* m_itkStripsIDHelper{nullptr};

  UnsignedShortProperty m_rodBlockVersion{this, "RodBlockVersion", 0};
};

#endif 
