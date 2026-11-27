/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_RAWDATABYTESTREAMCNV_TRTRAWCONTRAWEVENTTOOL_H
#define TRT_RAWDATABYTESTREAMCNV_TRTRAWCONTRAWEVENTTOOL_H

#include "TRT_RawDataByteStreamCnv/ITRTRawContByteStreamTool.h"

#include "GaudiKernel/ToolHandle.h"


#include "TRT_Cabling/ITRT_CablingSvc.h"


#include "ByteStreamCnvSvcBase/FullEventAssembler.h" 
#include "ByteStreamCnvSvcBase/SrcIdMap.h" 
#include "ByteStreamCnvSvcBase/IByteStreamCnvSvc.h"


class TRT_ID;



/** An AlgTool class to provide conversion from TRT RDO container
  *  to ByteStream, and fill it in RawEvent
  *  created:  Oct 25, 2002, by Hong Ma 
  *  requirements:   typedef for CONTAINER class 
		     method 
               StatusCode convert(CONTAINER* cont, RawEvent* re, MsgStream& log ); 
  */

class TRTRawContByteStreamTool: public extends<AthAlgTool, ITRTRawContByteStreamTool>
{
 public:

  // RawData type
  typedef TRT_RDORawData          RDO ;

  // Collection type 
  typedef InDetRawDataCollection< RDO > TRTRawCollection; 


  //! constructor
  using base_class::base_class;
  
  //! destructor 
  virtual ~TRTRawContByteStreamTool() ;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  //! New convert method which makes use of the encoder class (as done for other detectors)
  virtual StatusCode convert(TRT_RDO_Container* cont) const override; 
  
private: 
   ServiceHandle<IByteStreamCnvSvc> m_byteStreamCnvSvc
   { this, "ByteStreamCnvSvc", "ByteStreamCnvSvc" };

  ServiceHandle<ITRT_CablingSvc>  m_trt_CablingSvc{"TRT_CablingSvc", name()};
  
   const TRT_ID*                   m_trt_idHelper{nullptr};
   Gaudi::Property<unsigned short> m_RodBlockVersion{this, "RodBlockVersion", 3};

};
#endif



