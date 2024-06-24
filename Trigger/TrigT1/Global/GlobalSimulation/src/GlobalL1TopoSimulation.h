/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_GLOBALL1TOPOSIMULATION_H
#define GLOBALSIM_GLOBALL1TOPOSIMULATION_H

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

#include "IL1TopoAlgTool.h"

namespace GlobalSim {
  
  class GlobalL1TopoSimulation : public AthReentrantAlgorithm {
  public:
     
    GlobalL1TopoSimulation(const std::string& name, ISvcLocator *pSvcLocator);
    
    virtual StatusCode initialize () override;
    virtual StatusCode execute (const EventContext& ctx) const override;

  private:

    Gaudi::Property<bool>
    m_useTestInputEvent {this, "useTestInputEvent", {false},
      "use a test input event"};

    ToolHandleArray<IL1TopoAlgTool> m_topoAlgs{
      this,
      "topo_algs",
      {},
      "ordered sequence of L1 TopoAlgs"};
                
    Gaudi::Property<bool>
    m_enableDumps {this, "enableDumps", {false},
      "flag to control writing debug files"};

  };

}
#endif
