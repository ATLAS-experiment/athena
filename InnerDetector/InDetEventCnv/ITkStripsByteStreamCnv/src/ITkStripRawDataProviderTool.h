/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKSTRIPSBYTETREAMCNV_ITKSTRIPRAWDATAPROVIDERTOOL_H
#define ITKSTRIPSBYTETREAMCNV_ITKSTRIPRAWDATAPROVIDERTOOL_H

#include "ITkStripsByteStreamCnv/IITkStripRawDataProviderTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/ToolHandle.h"

#include <atomic>


class IITkStripsRodDecoder;

/**
 * @class ITkStripRawDataProviderTool
 *
 * @brief Athena Algorithm Tool to fill Collections of ITk Strips RDO Containers.
 *
 * The class inherits from AthAlgTool and IITkStripRawDataProviderTool.
 *
 * Contains a convert method that fills the ITk Strips RDO Collection.
 */
class ITkStripRawDataProviderTool : public extends<AthAlgTool, IITkStripRawDataProviderTool>
{
 public:
   
  using base_class::base_class;
  
  /** Destructor */
  virtual ~ITkStripRawDataProviderTool() = default;

  /** Initialize */
  virtual StatusCode initialize() override;

  /**
   * @brief Main decoding method.
   *
   * Loops over ROB fragments, get ROB/ROD ID, then decode if not allready decoded.
   *
   * @param vecROBFrags Vector containing ROB framgents.
   * @param rdoIDCont RDO ID Container to be filled.
   * @param errs Byte stream error container.
   * @param ctx EventContext of the event
   *  */
  virtual StatusCode convert(std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecROBFrags,
                             SCT_RDO_Container& rdoIDCont,
                             IDCInDetBSErrContainer& errs,
                             DataPool<SCT3_RawData>* dataItemsPool,
                             const EventContext& ctx) const override;

 private: 

  /** Algorithm Tool to decode ROD byte stream into RDO. */
  ToolHandle<IITkStripsRodDecoder> m_decoder{this, "Decoder", "ITkStripsRodDecoder", "Decoder"};

  /** Number of decode errors encountered in decoding. 
      Turning off error message after 100 errors are counted */
  mutable std::atomic_int m_decodeErrCount{0};

};

#endif // ITKSTRIPSBYTETREAMCNV_ITKSTRIPRAWDATAPROVIDERTOOL_H
