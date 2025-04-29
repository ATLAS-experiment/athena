/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*EXTERNAL CODE PORTED FROM YARR MINIMALLY ADAPTED FOR ATHENA*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 02/2024
* Description: ITkPix* encoding
*/

#include "ITkPixV2Encoder.h"

void ITkPixV2Encoder::endStream() const{
    //only called from mutex-protected function
    m_currBlock |= (0x1ULL << 63);
    pushWords32();
}

void ITkPixV2Encoder::addToStream(const HitMap& hitMap, bool last) const {

    //All operation on the class mutables (except m_words()) is done here
    //in a serialized sequence ensuring thread safety. The individual functions
    //called here are not accessible from the outside.

    //This is a high-level interface function that can take care of
    //adding an event into the current stream, this can be called
    //easily from the outside, and automatically tags/ends streams
    //and events based on internal vars only

    //If this is the first event, start a new stream. Otherwise, add an
    //internal tag
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_currEvent == 0){
        streamTag(m_currStream);
        m_currStream++;
    }
    else {
        intTag(m_currEvent);
    }
    
    //Then add the actual encoded event information
    setHitMap(hitMap);
    encodeEvent();
    m_currEvent++;

    //If this is the last event in the stream or if we explicitly
    //want to end the stream (i. e. total number of generated events
    //is not a multiple of nEventsPerStream), end the stream
    if (m_currEvent == m_nEventsPerStream || last){
        endStream();
        m_currEvent = 0;
    }

}