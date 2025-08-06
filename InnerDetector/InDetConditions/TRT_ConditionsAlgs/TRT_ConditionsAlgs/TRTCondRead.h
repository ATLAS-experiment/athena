/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTCONDITIONSALGS_TRTCONDREAD_H
#define TRTCONDITIONSALGS_TRTCONDREAD_H

/** @file TRTCondRead.h
 * @brief Algorithm to read TRT calibration constants from db and dump them to a text file.
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
#include "TRT_ConditionsServices/ITRT_CalDbTool.h"
#include "StoreGate/DataHandle.h"
#include "InDetIdentifier/TRT_ID.h"
#include "TRT_ConditionsData/RtRelationMultChanContainer.h"
#include "TRT_ConditionsData/StrawT0MultChanContainer.h"

/** @class TRTCondRead
   read TRT calibration constants from db and write text file
**/

class TRTCondRead : public AthAlgorithm
{
public:
    typedef TRTCond::RtRelationMultChanContainer RtRelationContainer;
    typedef TRTCond::StrawT0MultChanContainer StrawT0Container;

    /** constructor **/
    TRTCondRead(const std::string &name, ISvcLocator *pSvcLocator);

    virtual ~TRTCondRead() override = default;

    virtual StatusCode initialize(void) override;
    virtual StatusCode execute(void) override;
    virtual StatusCode finalize(void) override;

    /// create an TRTCond::ExpandedIdentifier from a TRTID identifier
    virtual TRTCond::ExpandedIdentifier trtcondid(const Identifier &id, int level = TRTCond::ExpandedIdentifier::STRAW) const;

    // methods for persistency

    /// write calibration constants or errors to flat text file
    virtual StatusCode writeCalibTextFile(std::ostream &) const;
    virtual StatusCode writeErrorTextFile(std::ostream &) const;

private:
    ToolHandle<ITRT_CalDbTool> m_TRTCalDbTool{this, "TRTCalDbTool", "CalDbTool", "Access to the folder of the calibration constants"};
    Gaudi::Property<std::string> m_par_caloutputfile{this, "CalibOutputFile", "caliboutput.txt", ""};
    const TRT_ID *m_trtid; //!< trt id helper
};

#endif // TRTCONDITIONSALGS_TRTCONDREAD_H
