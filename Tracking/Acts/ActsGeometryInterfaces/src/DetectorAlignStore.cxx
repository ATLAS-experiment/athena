
#include "ActsGeometryInterfaces/DetectorAlignStore.h"
#include "CxxUtils/ArrayHelper.h"

namespace{
    std::mutex s_ticketMutex{};
}

namespace ActsTrk{
    
    DetectorAlignStore::DetectorAlignStore(const DetectorType type) :
        detType{type} {}
    
    using TicketCounterArr = DetectorAlignStore::TrackingAlignStore::TicketCounterArr; 
    using ReturnedTicketArr = DetectorAlignStore::TrackingAlignStore::ReturnedTicketArr;
    TicketCounterArr DetectorAlignStore::TrackingAlignStore::s_clientCounter{};
    ReturnedTicketArr DetectorAlignStore::TrackingAlignStore::s_returnedTickets{};

    DetectorAlignStore::TrackingAlignStore::TrackingAlignStore(const DetectorType type) {
        m_transforms.resize(distributedTickets(type));
    }
    unsigned int DetectorAlignStore::TrackingAlignStore::drawTicket(const DetectorType type) { 
        std::lock_guard guard{s_ticketMutex};
        const unsigned int idx = static_cast<unsigned>(type);
        if (s_returnedTickets[idx].size()) {
            unsigned int distTicket = (*s_returnedTickets[idx].begin());
            s_returnedTickets[idx].erase(s_returnedTickets[idx].begin());
            return distTicket;
        }
        return s_clientCounter[idx]++;
    }            
    unsigned int DetectorAlignStore::TrackingAlignStore::distributedTickets(const DetectorType type) { 
        return s_clientCounter[static_cast<unsigned int>(type)]; 
    }
    void DetectorAlignStore::TrackingAlignStore::giveBackTicket(const DetectorType type, unsigned int ticketNo) {
        std::lock_guard guard{s_ticketMutex};
        s_returnedTickets[static_cast<unsigned int>(type)].push_back(ticketNo);
    }
   
}