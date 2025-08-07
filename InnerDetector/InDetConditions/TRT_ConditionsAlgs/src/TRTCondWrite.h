/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTCONDITIONSALGS_TRTCONDWRITE_H
#define TRTCONDITIONSALGS_TRTCONDWRITE_H

/** @file TRTCondWrite.h
 * @brief CondAlg to read TRT calibration constants in from text file and load them in ConditionsStore
 * @author Peter Hansen <phansen@nbi.dk>
 **/

//
#include <string>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ICondSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "StoreGate/DataHandle.h"
#include "InDetIdentifier/TRT_ID.h"
#include "TRT_ConditionsData/RtRelationMultChanContainer.h"
#include "TRT_ConditionsData/StrawT0MultChanContainer.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "GaudiKernel/EventIDRange.h"

/** @class TRTCondWrite
   read calibration constants from text file and publish them in ConditionsStore
**/

class TRTCondWrite : public AthAlgorithm
{
public:
    typedef TRTCond::RtRelationMultChanContainer RtRelationContainer;
    typedef TRTCond::StrawT0MultChanContainer StrawT0Container;

    /** constructor **/
    TRTCondWrite(const std::string &name, ISvcLocator *pSvcLocator);
    /** destructor **/
    virtual ~TRTCondWrite() override = default;

    virtual StatusCode initialize(void) override;
    virtual StatusCode execute(void) override;
    virtual StatusCode finalize(void) override;

    /// create an TRTCond::ExpandedIdentifier from a TRTID identifier
    virtual TRTCond::ExpandedIdentifier trtcondid(const Identifier &id, int level = TRTCond::ExpandedIdentifier::STRAW) const;

    /// read calibration from text file into TDS
    virtual StatusCode checkTextFile(const std::string &file, int &format);
    virtual StatusCode readTextFile(const std::string &file, int &format);
    virtual StatusCode readTextFile_Format1(std::istream &);

    virtual EventIDRange IOVInfRange() const;

private:
    const TRT_ID *m_trtid; //!< trt id helper
    ServiceHandle<ICondSvc> m_condSvc{this, "CondSvc", "CondSvc"};
    Gaudi::Property<std::string> m_par_caltextfile{this, "CalibInputFile", "dummy_TRTCondWrite.txt", ""};

    // WriteHandle Keys
    SG::WriteCondHandleKey<RtRelationContainer> m_rtWriteKey{this, "RtWriteKeyName", "/TRT/Calib/RT", "r-t relation out-key"};
    SG::WriteCondHandleKey<StrawT0Container> m_t0WriteKey{this, "T0WriteKeyName", "/TRT/Calib/T0", "t0 out-key"};
};

#endif // TRTCONDITIONSALGS_TRTCONDWRITE_H
