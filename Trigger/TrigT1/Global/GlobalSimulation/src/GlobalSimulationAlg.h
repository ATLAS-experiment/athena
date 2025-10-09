/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_GLOBALSIMULATIONALG_H
#define GLOBALSIM_GLOBALSIMULATIONALG_H

/*
 *  GlobalL1TopoSimulation runs L1Topo Algorithms.
 * It is heavily influenced by the L1TopoSimmulation packages.
 *
 * This Athena Algorithm is configuired with an ordered Sequence of AlgTools.
 * which represent the nodes of a call graph of GlobalSim Algorithms.
 *
 * It is the responsaility of the python configuration code to order the
 * AlgTools such that the ToolArray can be traversed sequentially.
 *
 */
 
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "IGlobalSimAlgTool.h"
#include "IO/TipWord_clid.h"

namespace GlobalSim {
  
  class GlobalSimulationAlg : public AthReentrantAlgorithm {
  public:
     
    GlobalSimulationAlg(const std::string& name, ISvcLocator *pSvcLocator);
    
    virtual StatusCode initialize () override;
    virtual StatusCode execute (const EventContext& ctx) const override;

  private:

    SG::WriteHandleKey<std::bitset<1024>> m_tipWordKey {
      this,
	"tipWord",
	"GlobalSimTIP",
	"Key to ewrite out TIP bitset"}; 
    

    Gaudi::Property<bool>
    m_useTestInputEvent {this, "useTestInputEvent", {false},
      "use a test input event"};

    ToolHandleArray<IGlobalSimAlgTool> m_algTools{
      this,
      "globalsim_algs",
      {},
      "ordered sequence of GlobalSim AlgTools"};
                
    Gaudi::Property<bool>
    m_enableDumps {this, "enableDumps", {false},
      "flag to control writing debug files"};

  };

}
#endif
