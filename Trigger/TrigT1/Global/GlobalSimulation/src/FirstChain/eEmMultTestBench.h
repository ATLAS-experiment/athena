/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EMMULTTESTBENCH_H
#define GLOBALSIM_EMMULTTESTBENCH_H

/*
 * Create and write out a FIFO (vector) of eEMTObs to the event store/
 * This simulates the action of the APP FIFOs, which feed TOBs to the
 * APU Algorithhms
 *
 */
 
#include "AthenaBaseComps/AthAlgorithm.h"

#include "../IGlobalSimAlgTool.h"
#include "../ITIPwriterAlgTool.h"
#include "../IO/IeEmTOBContainer.h" 

#include <string>
#include <memory>
#include <bitset>
#include <vector>
#include <fstream>

namespace GlobalSim {
  namespace IOBitwise {
    class IeEmTOB;
  }
}

namespace GlobalSim {

  
  /**
   * @brief AlgTool to count create inputs and expectations to test the
   * eEmMultAlgTool class.
   *
   */

  

  class eEmMultTestBench: public AthAlgorithm {

  public:
     
    eEmMultTestBench(const std::string& name, ISvcLocator *pSvcLocator);

    virtual StatusCode initialize () override;
    virtual StatusCode execute () override;

  private:

    SG::WriteHandleKey<GlobalSim::IOBitwise::IeEmTOBContainer>
    m_eEmTOBContainer_WriteKey {
      this,
      "eEmTOBs",
      "eEmTOBs",
      "Key for GlobalSim eEmTOB container"};
    

    SG::WriteHandleKey<TIPword>
    m_TIPword_WriteKey {
      this,
      "TIPwordWriteKey",
      "ExpectedTIPwords",
      "key to write out expectations for the TIP word"
    };
    
    Gaudi::Property<std::string>
    m_tobs_fileName{this,
		    "tobs_fileName",
		    {"GlobalSimulation/eEmMultTest_tobs.txt"},
		    "name of file with Global Sim eEmTOB data"};

    Gaudi::Property<std::string>
    m_TIPword_fileName{this,
		       "TIPwords_fileName",
		       {"GlobalSimulation/eEmMultTest_TIPwords.txt"},
		       "name of file with expected TIP words"};


    std::unique_ptr<TIPword> TIPword_from_file() const;

    std::ifstream m_tob_stream;
    std::unique_ptr<std::ifstream> m_TIPword_stream{nullptr};
    

    GlobalSim::IOBitwise::IeEmTOB* make_tob(const std::string& s) const;

  };

}
#endif
