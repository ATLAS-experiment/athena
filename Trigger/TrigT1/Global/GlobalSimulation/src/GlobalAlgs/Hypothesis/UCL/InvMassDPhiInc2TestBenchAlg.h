/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_INVMASSDPHIINC2TESTBENCHALG_H
#define GLOBALSIM_INVMASSDPHIINC2TESTBENCHALG_H

/*
 * Create and write out vectors of bit strings represented GenericTobs
 # and expectation values for the InvaariantMassDPhiInclusive2 Algorithm
 *
 */
 
#include "AthenaBaseComps/AthAlgorithm.h"

#include <string>
#include <memory>
#include <vector>


namespace GlobalSim {
  
  class InvMassDPhiInc2TestBenchAlg : public AthAlgorithm {
  public:

     
    InvMassDPhiInc2TestBenchAlg(const std::string& name, ISvcLocator *pSvcLocator);
    
    virtual StatusCode initialize () override;
    virtual StatusCode execute () override;

    using TobContainer = std::vector<std::string>;
    using TobContainerPtr = std::unique_ptr<TobContainer>;
    using Result = std::string;
    using ResultPtr = std::unique_ptr<Result>;

  private:

    SG::WriteHandleKey<TobContainer>
    m_tobs1_WriteKey {
      this,
	"genericTobBitContainer1WriteKey",
	"genericTobBitContainer1",
	"key to write out a GenericTob bit string Container (1/2"};


    SG::WriteHandleKey<TobContainer>
    m_tobs2_WriteKey {
      this,
      "genericTobBitContainer2WriteKey",
      "genericTobBitContainer2",
      "key to write out a GenericTob bit string Container (2/2"};

    SG::WriteHandleKey<Result>
    m_expectations_WriteKey {
      this,
      "InvMassDPhiInc2ExpectationsWriteKey",
      "InvMassDPhiInc2Expectations",
      "key to write out expectations for InvariantMassDPhiInclusive2 regression tests"
    };

    std::size_t m_dataIndex{0}; // indexes pointing to the  current data.
 


    // filename used when reading test data from files.
    Gaudi::Property<std::string>
    m_testsFileName1{this,
      "testsFileName1",
      {},
      "name of file with 32 bit data for GenericTobs 1 from HW Sim"};


    
    // filename used when reading test data from files.
    Gaudi::Property<std::string>
    m_testsFileName2{this,
      "testsFileName2",
      {},
      "name of file with 32 bit data for GenericTobs 2 from HW Sim"};

    
    // filename used when expected results  from files.
    Gaudi::Property<std::string>
    m_expectedResults_FileName{this,
      "expectedResultsFileName",
      {},
      "name of file with expected generic tob values from HW Sim"};

    // Manually written test data (used when not reading data from files)
    // GenericTobs represented as bit strings
    Gaudi::Property<std::vector<std::string>>
    m_testTobs1_in {
      this,
      "test_tobs1",
      {},
      "test vectors for manual tests. Binary. Manual."};

     
    // test Tobs after repeat has been applied to testVecs_in
    std::vector<TobContainer> m_testTobs1{};

    
    // Manually written test data (used when not reading data from files)
    // GenericTobs represented as bit strings
    Gaudi::Property<std::vector<std::string>>
    m_testTobs2_in {
      this,
      "test_tobs2",
      {},
      "test vectors for manual tests. Binary. Manual."};

     
    // test Tobs after repeat has been applied to testVecs_in
    std::vector<std::vector<std::string>> m_testTobs2{};

  
    // expected results for manual testing
    Gaudi::Property<std::string>
    m_expResults_in{
      this,
      "expResults",
      {},
      "expected counts for manual tests. Binary"};

    
    // choose int rather than unsigned someting to avoid unpleasantness
    // if initialised with a negative value
    Gaudi::Property<int>
    m_testRepeat {
      this,
      "testRepeat",
      {1},
      "number of times to repeat manual test values"};

    // expected results - one value per event
    std::vector<std::string>  m_expectedResults{};
    

    // m_fifo fillers, called from init()
    StatusCode init_manual();
    StatusCode init_from_file();
    StatusCode init_tobs1_from_file();
    StatusCode init_tobs2_from_file();
    StatusCode init_expected_results_from_file();

  };
}
#endif
