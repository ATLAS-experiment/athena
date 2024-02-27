/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONMDT_CABLING_NSW_CABLINGMAP_H
#define MUONMDT_CABLING_NSW_CABLINGMAP_H

#include <MuonCablingData/NswZebraData.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

#include <optional>

/**********************************************
 *
 * @brief MM cabling map data object
 *
 **********************************************/

class Nsw_CablingMap {
   public:
    Nsw_CablingMap(const Muon::IMuonIdHelperSvc* svc);

    // The following function corrects the MM cabling. It takes an identifier as
    // input (nominal identifer from the decoder) and returns
    //      The same identifier if there is no shift needed for this channel
    //      The new identifier containing the strip shift if a shift is needed
    //      A nullopt if the correction would move a channel outside the defined
    //      range of channel to be shifted (e.g. the first channel of a zebra
    //      connector which would be shifted into the channel range of the
    //      previous connector wich is physically impossible.)
    std::optional<Identifier> correctChannel(const Identifier& id,
                                             MsgStream& msg) const;

    // Function to add a range of channels to be shifted to the MM cabling map.
    bool addConnector(const Identifier& gapID,
                      const NswZebraData& connector, MsgStream& msg);

   private:
    const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};

    // Map holding the MM cabling correction map. The key is the identifier of
    // the gas gap and the value is a  NswZebraSet object containing the
    // information on which channel range needs to be moved
    using LookUpMap = std::map<Identifier, NswZebraSet>;
    LookUpMap m_cablingMap{};
};
std::ostream& operator<<(std::ostream& ostr,
                         const NswZebraData& connector);
CLASS_DEF(Nsw_CablingMap , 219609437 , 1 );
#include "AthenaKernel/CondCont.h"
CONDCONT_DEF( Nsw_CablingMap , 110668435 );

#endif