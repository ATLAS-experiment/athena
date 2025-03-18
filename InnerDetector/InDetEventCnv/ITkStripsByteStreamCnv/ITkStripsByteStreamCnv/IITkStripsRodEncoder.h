/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ITkStripsRawDataByteStreamCnv/IITkStripsRodEncoder.h
 * @author Shaun Roe
 * @date 10 April 2024
 * 
 * @brief This class provides conversion from ITk Strips RDO to ROD format ByteStream.
 */
 
#ifndef ITkStripsRawDataByteStreamCnv_IITkStripsRodEncoder_h
#define ITkStripsRawDataByteStreamCnv_IITkStripsRodEncoder_h

#include "GaudiKernel/IAlgTool.h"

#include <vector>
#include <cstdint>

class SCT_RDORawData;

/**
 * @class IITkStripsRodEncoder
 *
 * @brief Interface for Athena Algorithm Tool to convert from Strips RDO to ROD format BysteStream.
 *
 */
class IITkStripsRodEncoder : virtual public IAlgTool 
{
 public:

  /** Creates the InterfaceID and interfaceID() method */
  DeclareInterfaceID(IITkStripsRodEncoder, 1, 0);

  /** Destructor */
  virtual ~IITkStripsRodEncoder() = default;

  /** Convert method */
  virtual void fillROD(std::vector<uint32_t>& vec32Data, const uint32_t& robID, 
                       const std::vector<const SCT_RDORawData*>& vecRDOs) const = 0;
}; 

#endif 
