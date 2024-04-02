

#include "ActsGeometryInterfaces/DetectorAlignStore.h"

#include <stdlib.h>

int main() {
    using TicketCounter = ActsTrk::DetectorAlignStore::TrackingAlignStore;
    
    const ActsTrk::DetectorType refType = ActsTrk::DetectorType::Csc;
    constexpr unsigned int nTicketsToTest = 1000;

    for (unsigned int customer = 0; customer < nTicketsToTest ; ++customer) {
        unsigned int recieved = TicketCounter::drawTicket(refType);
        if (recieved != customer){
            std::cerr<<"TicketAlignStoreTest() "<<__LINE__<<": Expected for customer "<<customer
                     <<" to draw the same ticket number but recieved: "<<recieved<<std::endl;
            return EXIT_FAILURE;         
        }       
    }
    if (nTicketsToTest != TicketCounter::distributedTickets(refType)) {
        std::cerr<<"TicketAlignStoreTest() "<<__LINE__<<": "<<nTicketsToTest<<" tickets should have been distributed. "
                 <<"But in fact, "<<TicketCounter::distributedTickets(refType)<<" were handed out"<<std::endl;
        return EXIT_FAILURE;
    }
    /// Let's test the return mechanism by returning 5 ticket numbers in random order
    std::vector<unsigned int> toReturn{989, 666, 10, 42, 66};
    for (unsigned int sendBack : toReturn) {
        TicketCounter::giveBackTicket(refType, sendBack);
    }
    /// The next five tickets drawn should be the previously given back ones in ascending order
    std::sort(toReturn.begin(), toReturn.end());
    for (unsigned int customer : toReturn) {
        unsigned int recieved = TicketCounter::drawTicket(refType);
        if (recieved != customer){
            std::cerr<<"TicketAlignStoreTest() "<<__LINE__<<": Expected for customer "<<customer
                     <<" to draw the same ticket number but recieved: "<<recieved<<std::endl;
            return EXIT_FAILURE;         
        }       
    }
    /// Check that the overall counter of distributed tickets has not increased in the meantime
    if (nTicketsToTest != TicketCounter::distributedTickets(refType)) {
        std::cerr<<"TicketAlignStoreTest() "<<__LINE__<<": "<<nTicketsToTest<<" tickets should have been distributed. "
                 <<"But in fact, "<<TicketCounter::distributedTickets(refType)<<" were handed out"<<std::endl;
        return EXIT_FAILURE;
    }
    constexpr unsigned int lastOneHundred = nTicketsToTest - 100;
    for (unsigned int sendBack = lastOneHundred; sendBack != nTicketsToTest; ++sendBack) {
        TicketCounter::giveBackTicket(refType, sendBack);
    }
    if (lastOneHundred != TicketCounter::distributedTickets(refType)) {
        std::cerr<<"TicketAlignStoreTest() "<<__LINE__<<": "<<lastOneHundred<<" tickets should have been finally distributed. "
                 <<"But in fact, "<<TicketCounter::distributedTickets(refType)<<" were handed out"<<std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}