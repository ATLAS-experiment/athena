/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_HYPOTESTBENCHALG_H
#define GLOBALSIM_HYPOTESTBENCHALG_H

/*
 * Create and write out a FIFO (vector) of eEMTObs to the event store/
 * This simulates the action of the APP FIFOs, which feed TOBs to the
 * APU Algorithhms
 *
 */
 
#include "AthenaBaseComps/AthAlgorithm.h"

#include "GepAlgoHypothesisPortsIn.h"
#include "eEmSortSelectCountExpectations.h"


#include <string>
#include <memory>
#include <bitset>


namespace GlobalSim {
  
  class HypoTestBenchAlg : public AthAlgorithm {
  public:

     
    HypoTestBenchAlg(const std::string& name, ISvcLocator *pSvcLocator);
    
    virtual StatusCode initialize () override;
    virtual StatusCode execute () override;

  private:

    SG::WriteHandleKey<GepAlgoHypothesisFIFO>
    m_hypothesisFIFO_WriteKey {
      this,
	"hypothesisFIFOWriteKey",
	"hypoFIFO",
	"key to write out Fifo containing ports data"};

    SG::WriteHandleKey<eEmSortSelectCountExpectations>
    m_eEmSortSelectCountExpectations_WriteKey {
      this,
	"eEmSortSelectCountExpectationsWriteKey",
	"eEmSortSelectCountExpectations",
	"key to write out expectations for eEmSortSelecCount regression tests"
    };
    
    Gaudi::Property<std::string>
    m_testsFileName{this,
      "testsFileName",
      {},
      "name of file with APP FIFO data"};

    
    Gaudi::Property<std::string>
    m_expectedMults_FileName{this,
      "expectedMultsFileName",
      {},
      "name of file with the expected multiplicity values from HW Sim"};
    
    Gaudi::Property<std::string>
    m_expectedTobs_FileName{this,
      "expectedTobsFileName",
      {},
      "name of file with the expected Generic TOB values from HW Sim"};
    
    Gaudi::Property<std::vector<std::string>>
    m_testVecs_in{
      this,
      "testVecs",
      {},
      "test vectors for manual tests. Hex"};

    Gaudi::Property<std::string>
    m_expMults_in{
      this,
      "expMults",
      {},
      "expected counts for manual tests. Hex"};

    
    Gaudi::Property<std::string>
    m_expTobs_in {
      this,
      "expTobs",
      {},
      "expected output generic TOBs for manual tests. Hex"};

    // choose int rather than unsigned someting to avoid unpleasantness
    // if initialised with a negative value
    Gaudi::Property<int>
    m_testRepeat {
      this,
      "testRepeat",
      {1},
      "number of times to repeat manual test values"};

    // test vectors after repeat has been applied to testVecs_in
    std::vector<std::string> m_testVecs{};
    
    // One elenement of the following vector is written out each event.
    std::vector<std::unique_ptr<GepAlgoHypothesisFIFO>>  m_fifos;
    std::size_t m_fifo_ptr{0};

    // expected values for counts coorespoinding to the  FIFO data.
    std::vector<std::string>  m_expected_mults{};
    std::vector<std::string>  m_expected_tobs{};


    // m_fifo fillers, called from init()
    StatusCode init_manual();
    StatusCode init_from_file();
    StatusCode init_tests_from_file();
    StatusCode init_expected_mults_from_file();
    StatusCode init_expected_tobs_from_file();
    StatusCode init_expected_from_file(std::vector<std::string>&,
				       const std::string&);


    StatusCode
    hexTOB2bitsetTOB(std::string, std::bitset<72>&) const;
    
  };
}
#endif
