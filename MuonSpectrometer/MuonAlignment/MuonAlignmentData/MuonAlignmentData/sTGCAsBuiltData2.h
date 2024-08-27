/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDDATA_sTGCAsBuiltData2_H
#define MUONCONDDATA_sTGCAsBuiltData2_H

//Athena includes
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "GeoPrimitives/GeoPrimitives.h"

/**
 * Container class that stores the as built parameters  of the sTgcs. 
 * Each gasGap has its own set of corrections. 
*/



class sTGCAsBuiltData2: public AthMessaging {
    public:
        sTGCAsBuiltData2(const Muon::IMuonIdHelperSvc* idHelperSvc);
        // returns the local positon corrected for the as built parameters
        Amg::Vector2D correctPosition(const Identifier& channelId, const Amg::Vector2D& pos) const;
        // Set the parameters of the as build model (shift, rotation, scale)
        
        struct Parameters{
          double offset {0.};
          double rotation{0.};
          double scale{0.};
        };
        
        StatusCode setParameters(const Identifier& gasGapId, const Parameters& pars);

    private:
        const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
        using ParMap = std::unordered_map<Identifier, Parameters>; 
        ParMap m_asBuiltData{};
};

CLASS_DEF( sTGCAsBuiltData2  , 189786421 , 1 );
CONDCONT_DEF( sTGCAsBuiltData2 , 15989615 );
#endif
