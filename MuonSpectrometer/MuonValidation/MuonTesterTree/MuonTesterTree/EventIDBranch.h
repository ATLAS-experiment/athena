/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTERTREE_EVENTIDBRANCH_H
#define MUONTESTERTREE_EVENTIDBRANCH_H

#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/MuonTesterBranch.h"
#include "GaudiKernel/EventIDBase.h"

namespace MuonVal{
    /** @brief Branch class to dump a EventIDBase object into a MuonTesterTree format. The event ID object 
     *         is characterized by the following 4 attributes
     *          runNumber: Number of the taken run
     *          lumbiBlock: number: Number of the lumi-block inside the run
     *          eventNumber: Start of a particular event
     *          timeStamp: Time using the linux time convention
     * 
     *      If added to the output, the branch has to be set in each fill iteration. 
     *      Otherwise, the tree filling will fail.  */
    class EventIDBranch: public MuonTesterBranch {
        public:
            /** @brief Standard constructor
             *  @param objName: Name of the eventID object in the tree
             *  @param parent: Reference to the MuonTesterTree to which the information is added */
            EventIDBranch(MuonTesterTree& parent, const std::string& objName);

            /** @brief Fill the eventID information */
            void operator=(const EventIDBase& eventID);
            void set(const EventIDBase& eventID);

            /** @brief Interface function from the IMuonTesterBranch */
            bool fill(const EventContext&) override final;
            /** @brief Interface function from the IMuonTesterBranch  */
            bool init() override final;

        private:
            ScalarBranch<unsigned>& m_lumiBlock{parent().newScalar<unsigned>(name() + "_LB")};
            ScalarBranch<unsigned>& m_runNumber{parent().newScalar<unsigned>(name()+"_runNumber")};
            ScalarBranch<Long64_t>& m_eventNumber{parent().newScalar<Long64_t>(name()+"_eventNumber")};
            ScalarBranch<unsigned>& m_timeStamp{parent().newScalar<unsigned>(name()+"_timeStamp")};
    };

}
#endif