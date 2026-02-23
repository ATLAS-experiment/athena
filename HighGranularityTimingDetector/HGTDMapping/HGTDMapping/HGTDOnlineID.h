/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HGTDOnlineID_H
#define HGTDOnlineID_H
/**
  * @author  Yassine El Ghazali
  * @date  23 January 2026
  * @brief Online Identifier for HGTD
  */

#include<ostream>
#include<cstdint>
#include <typeindex> //provides std::hash

class HGTDOnlineID{
    public:
    friend std::ostream& operator<<(std::ostream & os, const HGTDOnlineID & id);
    
    // Default constructor
    HGTDOnlineID() = default;

    // Construct from uint32
    HGTDOnlineID(const std::uint32_t onlineId);

    // construct from RobID and felix elink ?
    HGTDOnlineID(const std::uint32_t rodId, const std::uint32_t elink);

    // return ROB/ROD ID
    std::uint32_t rod() const;

    // return elink
    std::uint32_t elink() const;
    
    // comparison - spaceship operator
    auto operator<=>(const HGTDOnlineID & other) const = default;

    // check if the Online ID is valid
    bool isValid() const;

    enum {
        INVALID_ELINK=255, 
        INVALID_ROD=16777215, 
        INVALID_ONLINE_ID=0xFFFFFFFF
    };


    private:
    std::uint32_t m_onlineId{INVALID_ONLINE_ID};

};

#endif
