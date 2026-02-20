/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HGTDMappingData_h
#define HGTDMappingData_h
/**
  * @author  Yassine El Ghazali
  * @date  23 January 2026
  * @brief Data object containing the offline-online mapping for HGTD (based on ITkPixelCabling package)
  */

#include "HGTDMapping/HGTDOnlineID.h"

// Athena includes
#include "Identifier/IdentifierHash.h"
#include "Identifier/Identifier.h"
#include "AthenaKernel/CondCont.h"
#include "AthenaKernel/CLASS_DEF.h"
//
#include <unordered_map>



class HGTDMappingData{

    public:
    //stream extraction to read value from stream into HGTDMappingData
    friend std::istream& operator>>(std::istream & is, HGTDMappingData & mapping);
    ///stream insertion for debugging
    friend std::ostream& operator<<(std::ostream & os, const HGTDMappingData & mapping);

    bool empty() const;
    std::size_t size() const;
    HGTDOnlineID onlineId(const Identifier & id) const;

    // HGTD contains 8032 modules, per sensor?
    enum {NUMBER_OF_HASHES=8032}; // In HGTD, we have 16064 chips.


    private:
    std::unordered_map<Identifier, HGTDOnlineID> m_offline2OnlineMap;
    std::set<std::uint32_t> m_rodIdSet; //!< Set of robIds
    std::array<HGTDOnlineID, NUMBER_OF_HASHES> m_hash2OnlineIdArray; //!< Array for hash to onlineId; hash goes from 0-49536

    
};

CLASS_DEF( HGTDMappingData , 43043952, 1 );
CONDCONT_DEF( HGTDMappingData , 64668622);

#endif
