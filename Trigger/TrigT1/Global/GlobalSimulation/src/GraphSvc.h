/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIMULATION_GRAPHSVC_H
#define GLOBALSIMULATION_GRAPHSVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/ServiceHandle.h"

namespace GlobalSim {

    /**
     * The GraphSvc can be used to create an execution graph of the global simulation part of the job
     */
    class GraphSvc : public extends<AthService,IService> {
    public:
        /// Constructor
        using extends::extends;

        /// Initialise
        virtual StatusCode initialize() override;

        /// Start - do graph calculation here, to ensure EventLoopMgr initialized already
        virtual StatusCode start() override;

        /// Finalise
        virtual StatusCode finalize() override;

    private:
        StringProperty m_fileName{this,"FileName","graph.dot","Output file"};
        StringProperty m_topSequence{this,"SequenceNameFilter","GlobalSim","Regex of sequence names to include"};

        ServiceHandle<IAlgResourcePool> m_algResourcePool{this,"AlgResourcePool","AlgResourcePool",
                                                          "Algorithm resource pool service."};

    };

}

#endif