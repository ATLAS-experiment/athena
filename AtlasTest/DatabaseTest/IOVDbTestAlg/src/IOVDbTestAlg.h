/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 IOVDB test package
 -----------------------------------------
 ***************************************************************************/

//<version>	$Name: not supported by cvs2svn $

#ifndef IOVDBTESTALG_IOVDBTESTALG_H
# define IOVDBTESTALG_IOVDBTESTALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "IOVDbTestConditions/IOVDbTestMDTEleMap.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"

#include "RegistrationServices/IIOVRegistrationSvc.h"
#include "StoreGate/DataHandle.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaKernel/IOVSvcDefs.h"

class EventInfo;
class IIOVRegistrationSvc;
class IAthenaOutputStreamTool;

/**
 ** Algorithm to test writing conditions data and reading them back.
 **/

class IOVDbTestAlg: public AthReentrantAlgorithm 
{
public:
    IOVDbTestAlg (const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~IOVDbTestAlg();

    virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE() override;
    virtual StatusCode execute (const EventContext& ctx) const override;
    virtual StatusCode finalize() override;

private:

    StatusCode createCondObjects (const EventContext& ctx) const;
    StatusCode printCondObjects() const;
    StatusCode streamOutCondObjects();
    StatusCode registerCondObjects();
    StatusCode readWithBeginRun();
    void       waitForSecond() const;
    StatusCode testCallBack(  IOVSVC_CALLBACK_ARGS  );
    StatusCode registerIOV(const CLID& clid);

    BooleanProperty           m_writeCondObjs{this, "WriteCondObjs", false};
    BooleanProperty           m_regIOV{this, "RegisterIOV", false};
    BooleanProperty           m_readWriteCool{this, "ReadWriteCool", false};
    BooleanProperty           m_twoStepWriteReg{this, "TwoStepWriteReg", false};
    BooleanProperty           m_createExtraChans{this, "CreateExtraChanns", false};
    BooleanProperty           m_nameChans{this, "NameChanns", false};
    BooleanProperty           m_readInInit{this, "ReadInInit", false};
    BooleanProperty           m_writeOnlyCool{this, "WriteOnlyCool", false};
    BooleanProperty           m_fancylist{this, "FancyList", false};
    BooleanProperty           m_printLB{this, "PrintLB", false};
    BooleanProperty           m_writeNewTag{this, "WriteNewTag", false};
    BooleanProperty           m_readNewTag{this, "ReadNewTag", false};
    BooleanProperty           m_noStream{this, "NoStream", false};
    IntegerProperty           m_regTime{this, "RegTime", 0, "Register time in sec"};
    StringProperty            m_streamName{this, "StreamName", "CondStream1"};
	IntegerProperty           m_run{this, "run", 0};
    BooleanProperty           m_online{this, "online", false};
    StringProperty            m_tagID{this, "TagID", ""};
    
    ServiceHandle<IIOVRegistrationSvc>      m_regSvc;
    ToolHandle<IAthenaOutputStreamTool>     m_streamer;
};


#endif // IOVDBTESTALG_IOVDBTESTALG_H
