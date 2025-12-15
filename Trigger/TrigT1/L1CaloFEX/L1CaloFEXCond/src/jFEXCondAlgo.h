/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//             Interface for jFEXCondAlgo - Tool to read the COOL DB for jFEX
//                              -------------------
//     begin                : 01 08 2023
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************

#ifndef jFEXCondAlgo_H
#define jFEXCondAlgo_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "L1CaloFEXCond/jFEXDBCondData.h"


namespace LVL1 {

class jFEXCondAlgo : public AthCondAlgorithm {

    public:
        /** Constructors **/
        jFEXCondAlgo(const std::string& name, ISvcLocator* svc);

        /// Function initialising the algorithm
        virtual StatusCode initialize() override;
        /// Function executing the algorithm
        virtual StatusCode execute( const EventContext& ) const override;

    private:

        // Key for writing CondHandles
        SG::WriteCondHandleKey<jFEXDBCondData> m_jFEXDBParamsKey {this, "jFEXDBParamsKey", "jFEXDBParams", "Output for jFEX DB parameters"};

        // Key for reading CondHandles
        SG::ReadCondHandleKey<CondAttrListCollection> m_JfexModuleSettingsKey{this,"JfexModuleSettings", "", "Key to store JfexModuleSettings DB path"};
        SG::ReadCondHandleKey<CondAttrListCollection> m_JfexNoiseCutsKey     {this,"JfexNoiseCuts"     , "", "Key to store JfexNoiseCuts DB path"     };
        SG::ReadCondHandleKey<CondAttrListCollection> m_JfexSystemSettingsKey{this,"JfexSystemSettings", "", "Key to store JfexSystemSettings DB path"};

        UnsignedIntegerProperty m_dbBeginTimestamp {this,"BeginTimestamp", 1698527690,"Earliest timestamp that db parameters will be loaded. Default is start of 2023-10-27"};

	Gaudi::Property<bool> m_isMC {this, "IsMC", false, "For MC, always access the DB"};

};



}//end of namespace

#endif
