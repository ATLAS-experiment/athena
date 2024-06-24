/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDDATA_DigitEffiData_H
#define MUONCONDDATA_DigitEffiData_H

//Athena includes
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h"
#include "AthenaBaseComps/AthMessaging.h"

/**
 * @brief: Container class storing the efficiency values for a subdetector. Depending on the detector technology, 
 *         effiiciencies are defined with different channel granularity
 *       Mdt: Tube-level efficiency
 *       Rpc/Tgc/Csc: Individual gasGap
 *       MM: Individual PCBs
 *     sTGC: Individual FEB
 *      If a certain module is not found in the map, a default efficiency is returned
*/
namespace Muon{
    class DigitEffiData: public AthMessaging {
        public:
            /// Constructor taking the pointer to the IdHelperSvc & defining a default 
            /// efficiency for cases wher the map doesn't know about the channel
            DigitEffiData(const Muon::IMuonIdHelperSvc* idHelperSvc, double defaultEffi);
            /// Returns the signal generation efficiency of the sTgc channel
            double getEfficiency(const Identifier& channelId, bool isInnerQ1 = false /* needed for the sTGCs*/) const;
            /// Sets the efficiency for a given minimal section of the dector
            StatusCode setEfficiency(const Identifier& sectionId, const double effi);
        private:
            Identifier getLookUpId(const Identifier& channelId, bool isInnerQ1 = false /*needed for the sTGCs*/) const;
            const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
            using EffiMap = std::unordered_map<Identifier, double>; 
            EffiMap m_effiData{};
            double m_defaultEffi{1.};
    };
}
CLASS_DEF( Muon::DigitEffiData , 248247637 , 1 );
CONDCONT_DEF( Muon::DigitEffiData , 17123805 );
#endif
