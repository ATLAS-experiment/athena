/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "L1TopoSimulationUtils/Helpers.h"



bool TSU::isAmbiguousAt( TCS::TOBArray const* tobs, size_t pos, unsigned minEt) {
    // checks if the sorting of two TOBs at positions 'pos' and 'pos+1'
    // is ambiguous, i.e., if they have the same ET values
        // ambiguities due to TOBs with ET <= minEt are ignored

    // if there is no other TOB after the one in question 
    // then there is no ambiguity (or we cannot tell anymore 
    // in case the array was previously truncated already
    if (tobs->size() <= pos+1) { return false; }
    if ((*tobs)[pos].Et() <= minEt) { return false; }
    if ((*tobs)[pos].Et() == (*tobs)[pos+1].Et()) { return true; }
    return false;
}

bool TSU::isAmbiguousTruncation( TCS::TOBArray const* list, size_t nMax, unsigned minEt ) {
    // checks if the truncation of the given, sorted list after nMax elements
    // results in an ambiguous set of TOBs due to the last element of the list
    // having the same ET value as the first one to be dropped
    // also accounts for ambiguity flags previously set on the list 
    // (due to earlier truncations) if the value of nMax implies that all list 
    // elements are to be used/retained
    // ambiguities due to TOBs with ET <= minEt are ignored
    if ((*list).size() <= nMax && ( (list->size() > 0) && ( (*(--list->end()))->Et() > minEt ) ) && (*list).ambiguityFlag()) {
      return true;
    }
    return TSU::isAmbiguousAt(list, nMax-1);
}

bool TSU::isAmbiguousAnywhere( TCS::TOBArray const* list, size_t nMax, unsigned minEt ) {
    // checks if anywhere in a sorted list of TOBs there is a consecutive 
    // pair of TOBs for which the purely ET based sorting is ambiguous.
    // An ambiguity is also indicated if the list is already flagged as
    // being ambiguous (due to an earlier truncation with the last TOB 
    // in the list having the same ET as the first dropped TOB) and all
    // elements of the presented list are to be used as indicated by 'nMax'
    // ambiguities due to TOBs with ET <= minEt are ignored
    
    // forward the ambiguity flag if the list has been truncated upstream
    // such that it has the same (or smaller) length as we would trim it 
    // down to and thereby flagged as being ambiguous (for its last element)
    if ((*list).size() <= nMax && ( (list->size() > 0) && ( (*(--list->end()))->Et() > minEt ) ) && (*list).ambiguityFlag()) {
      return true;
    }
    // comparing each TOB with the subsequent one
    if ((*list).size() < 1 || nMax < 1) { return false; }
    for (size_t i=0; i < std::min((*list).size(), nMax) - 1; ++i) {
        if (TSU::isAmbiguousAt(list, i, minEt)) {
          return true;
        }
    }
    return false;
}
