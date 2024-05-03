/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/


///This test demonstrates how to write an algorithm
///And then use it with POOL::TEvent

#include "AthAnalysisBaseComps/AthAnalysisAlgorithm.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODRootAccess/tools/TFileAccessTracer.h"

#include "AsgMessaging/MessageCheck.h"

class MyTestAlg : public AthAnalysisAlgorithm {
public:
  MyTestAlg( const std::string& name ) : AthAnalysisAlgorithm( name ) {
    declareProperty("MyProperty",m_value,"Example property");

  }

  virtual StatusCode initialize() {
    ATH_MSG_INFO(" initializing with MyProperty = " << m_value );
    return StatusCode::SUCCESS;
  }

  virtual StatusCode execute() {
    const xAOD::EventInfo* ei = 0;
    CHECK( evtStore()->retrieve( ei ) );
    ATH_MSG_INFO(" event num = " << ei->eventNumber());
    return StatusCode::SUCCESS;
  }

private:
  int m_value = 0;

};

//-------
// here's the example program

#include "POOLRootAccess/TEvent.h"

int main ATLAS_NOT_THREAD_SAFE () {

  xAOD::TFileAccessTracer::enableDataSubmission(false); // disable file reporting in unittest

  ANA_CHECK_SET_TYPE (int); // because we are running in a method that returns int
  using namespace asg::msgUserCode;
  
  POOL::TEvent evt(POOL::TEvent::kClassAccess);
  ANA_CHECK( evt.readFrom("$ASG_TEST_FILE_MC") );

  MyTestAlg* alg = new MyTestAlg("MyAlg");
  ANA_CHECK( alg->setProperty("MyProperty",4) );

  ANA_CHECK( alg->sysInitialize() ); //calling sysInitialize means incident listening set up  

  for(int i=0;i<10;i++) {
    evt.getEntry(i);
    ANA_CHECK(alg->execute());
  }

  return 0;

}
