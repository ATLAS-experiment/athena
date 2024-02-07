/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTCONDITIONSALGS_TRTCONDSTORETEXT_H
#define TRTCONDITIONSALGS_TRTCONDSTORETEXT_H

/** @file TRTCondStoreText.h
 * @brief Algorithm to read TRT Conditions objects
 * from text file and stream them to db.
 * @author Peter Hansen <phansen@nbi.dk>
 **/

//
#include <string>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ICondSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/DataHandle.h"
#include "InDetIdentifier/TRT_ID.h"
#include "TRT_ConditionsData/RtRelationMultChanContainer.h"
#include "TRT_ConditionsData/StrawT0MultChanContainer.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"

/** @class TRTCondStoreText
   read calibration constants from text file and store them in a pool and cool file.
**/ 

class TRTCondStoreText:public AthAlgorithm {
public:
  typedef TRTCond::RtRelationMultChanContainer RtRelationContainer ;
  typedef TRTCond::StrawT0MultChanContainer StrawT0Container ;


  /** constructor **/
  TRTCondStoreText(const std::string& name, ISvcLocator* pSvcLocator);
  /** destructor **/
  ~TRTCondStoreText();

  virtual StatusCode  initialize(void) override;    
  virtual StatusCode  execute(void) override;
  virtual StatusCode  finalize(void) override;

  /// create an TRTCond::ExpandedIdentifier from a TRTID identifier
  virtual TRTCond::ExpandedIdentifier trtcondid( const Identifier& id, int level = TRTCond::ExpandedIdentifier::STRAW) const;

  /// read calibration from text file into TDS
  virtual StatusCode checkTextFile(const std::string& file, int& format);
  virtual StatusCode readTextFile(const std::string& file, int& format);
  virtual StatusCode readTextFile_Format1(std::istream&);
  virtual StatusCode readTextFile_Format2(std::istream&);
  virtual StatusCode readTextFile_Format3(std::istream&);


 private:

  std::string m_par_rtcontainerkey;        //"/TRT/Calib/RT"
  std::string m_par_errcontainerkey;       //"/TRT/Calib/errors2d"
  std::string m_par_slopecontainerkey;     //"/TRT/Calib/slopes"
  std::string m_par_t0containerkey;        //"/TRT/Calib/T0"
  std::string m_par_caltextfile;           //!< calibration text file specified in jobOptions
  const TRT_ID* m_trtid;                   //!< trt id helper
  std::string m_streamer;                  //"AthenaOutputStreamTool/CondStream1"
  ServiceHandle<StoreGateSvc> m_detstore;
 
};
 
#endif // TRTCONDITIONSALGS_TRTCONDSTORETEXT_H

