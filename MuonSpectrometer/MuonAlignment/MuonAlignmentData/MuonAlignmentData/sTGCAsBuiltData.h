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
 * @class sTGCAsBuiltData
 *
 * @brief Class holding the sTGC as built conditions data and applying it. The model consists of four parameters:
 *         - offset: shift of the strip layer in the precission coordinate
 *         - rotation: rotation of the strip plane around the origin of the local coordinate system
 *         - scale: correction to the pitch of the strips.
 *         - nonPara: correction to the non parallelism of the strips
 * 
 *        The as built parameters are applied in athenas local coordinate system. The parameters are expected in units of um and mrad
 *
 */

class sTGCAsBuiltData: public AthMessaging {
    public:
        sTGCAsBuiltData(const Muon::IMuonIdHelperSvc* idHelperSvc);
        // returns the local positon corrected for the as built parameters
        Amg::Vector2D correctPosition(const Identifier& channelId, const Amg::Vector2D& pos) const;
        // Set the parameters of the as build model (shift, rotation, scale)


        struct Parameters{
          double offset {0.};
          double rotation{0.};
          double scale{0.};
          double nonPara {0.};
        };
        
        StatusCode setParameters(const Identifier& gasGapId, const Parameters& pars);

    private:
        const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
        using ParMap = std::unordered_map<Identifier, Parameters>; 
        ParMap m_asBuiltData{};
};

std::ostream& operator<<(std::ostream& ostr, const sTGCAsBuiltData::Parameters& par);

CLASS_DEF( sTGCAsBuiltData  , 154485394 , 1 );
CONDCONT_DEF( sTGCAsBuiltData , 66070092 );
#endif
