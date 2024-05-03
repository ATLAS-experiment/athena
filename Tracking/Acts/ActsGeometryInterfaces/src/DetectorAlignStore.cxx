
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
    using ReturnedHintArr = DetectorAlignStore::TrackingAlignStore::ReturnedHintArr;
    TicketCounterArr DetectorAlignStore::TrackingAlignStore::s_clientCounter{};
    ReturnedTicketArr DetectorAlignStore::TrackingAlignStore::s_returnedTickets{};
    ReturnedHintArr DetectorAlignStore::TrackingAlignStore::s_returnedHints{};

    DetectorAlignStore::TrackingAlignStore::TrackingAlignStore(const DetectorType type) {
        m_transforms.resize(distributedTickets(type));
    }
    unsigned int DetectorAlignStore::TrackingAlignStore::drawTicket(const DetectorType type) { 
        std::lock_guard guard{s_ticketMutex};
        const unsigned int idx = static_cast<unsigned>(type);
        std::vector<bool>& returnedPool = s_returnedTickets[idx];
        int& returnedHint = s_returnedHints[idx];
        if (returnedPool.size() && returnedHint >= 0) {
            for (size_t i = returnedHint; i < returnedPool.size(); i++) {
              if (returnedPool[i]) {
                returnedPool[i] = false;

                returnedHint = i+1;
                if (static_cast<size_t>(returnedHint) >= returnedPool.size()) {
                  returnedHint = 0;
                }
                return i;
              }
            }

            for (size_t i = 0; i < static_cast<size_t>(returnedHint); i++) {
              if (returnedPool[i]) {
                returnedPool[i] = false;
                returnedHint = i+1;
                return i;
              }
            }

            returnedHint = -1;
        }
        else {
          returnedHint = -1;
        }
        return s_clientCounter[idx]++;
    }            
    unsigned int DetectorAlignStore::TrackingAlignStore::distributedTickets(const DetectorType type) { 
        return s_clientCounter[static_cast<unsigned int>(type)]; 
    }
    void DetectorAlignStore::TrackingAlignStore::giveBackTicket(const DetectorType type, unsigned int ticketNo) {
        std::lock_guard guard{s_ticketMutex};
        const unsigned int idx = static_cast<unsigned int>(type);
        std::vector<bool>& returnedPool = s_returnedTickets[idx];
        int& returnedHint = s_returnedHints[idx];
        /// The ticket which was handed out at the very latest is returned. Remove all returned tickets from before
        if (ticketNo == distributedTickets(type) -1) {

           if (ticketNo > 0 && ticketNo-1 < returnedPool.size()) {
             for (; ticketNo > 0 && returnedPool[ticketNo-1]; --ticketNo)
               ;
             returnedPool.resize (ticketNo);
           }
           /// Remove all trailing ticket numbers
           s_clientCounter[idx] = ticketNo;
           if (returnedHint >= static_cast<int>(ticketNo)) {
             returnedHint = 0;
           }
        } else {
            if (returnedPool.size() <= ticketNo) {
              returnedPool.resize (ticketNo+1);
            }
            returnedPool[ticketNo] = true;
            if (returnedHint < 0 || static_cast<int>(ticketNo) < returnedHint) {
              returnedHint = ticketNo;
            }
        }
    }
   
}
