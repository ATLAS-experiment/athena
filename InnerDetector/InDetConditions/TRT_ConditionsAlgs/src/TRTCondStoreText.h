/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTCONDITIONSALGS_TRTCONDSTORETEXT_H
#define TRTCONDITIONSALGS_TRTCONDSTORETEXT_H

/** @file TRTCondStoreText.h
 * @brief Algorithm to read TRT Conditions objects
 * from text file and stream them to db.
 * @author Peter Hansen <phansen@nbi.dk>
 **/

//

#include "AthenaBaseComps/AthAlgorithm.h"

#include "TRT_ConditionsData/RtRelationMultChanContainer.h"
#include "TRT_ConditionsData/StrawT0MultChanContainer.h"
#include "TRT_ConditionsData/ExpandedIdentifier.h" //for TRTCond::ExpandedIdentifier::STRAW

#include <string>
#include <iosfwd> //for std::istream fwd declaration

class Identifier;
class TRT_ID;

/** @class TRTCondStoreText
   read calibration constants from text file and store them in a pool and cool file.
**/ 

class TRTCondStoreText:public AthAlgorithm {
public:
  typedef TRTCond::RtRelationMultChanContainer RtRelationContainer ;
  typedef TRTCond::StrawT0MultChanContainer StrawT0Container ;


  /** constructor **/
  TRTCondStoreText(const std::string& name, ISvcLocator* pSvcLocator);

  virtual ~TRTCondStoreText() override = default;

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
    
    Gaudi::Property<std::string> m_par_errcontainerkey   {this,"ErrorFolderName"     ,"/TRT/Calib/errors2d",""};
    Gaudi::Property<std::string> m_par_slopecontainerkey {this,"ErrorSlopeFolderName","/TRT/Calib/slopes",""};
    Gaudi::Property<std::string> m_par_rtcontainerkey    {this,"RtFolderName"        ,"/TRT/Calib/RT",""};
    Gaudi::Property<std::string> m_par_t0containerkey    {this,"T0FolderName"        ,"/TRT/Calib/T0",""};
    Gaudi::Property<std::string> m_par_caltextfile       {this,"CalibInputFile"      ,"dbconst.txt",""};
    Gaudi::Property<std::string> m_streamer              {this,"StreamTool"          ,"AthenaOutputStreamTool/CondStream1",""};

    const TRT_ID* m_trtid{};                   //!< trt id helper
 
};
 
#endif // TRTCONDITIONSALGS_TRTCONDSTORETEXT_H

