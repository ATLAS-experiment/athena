/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JFEXDRIVER_H
#define JFEXDRIVER_H

// STL
#include <string>


// Athena/Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "L1CaloFEXToolInterfaces/IjFEXSysSim.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "L1CaloFEXSim/jFEXOutputCollection.h"

class CaloIdManager;

namespace LVL1 {

class jFEXDriver : public AthAlgorithm
{
    public:
        jFEXDriver(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~jFEXDriver();

        virtual StatusCode initialize();
        virtual StatusCode execute();
        StatusCode finalize();

    private:

        SG::WriteHandleKey<jFEXOutputCollection> m_jFEXOutputCollectionSGKey {this, "MyOutputs", "jFEXOutputCollection", "MyOutputs"};

        ToolHandle<IjFEXSysSim> m_jFEXSysSimTool {this, "jFEXSysSimTool", "LVL1::jFEXSysSim", "Tool that creates the jFEX System Simulation"};

};

} // end of LVL1 namespace
#endif
