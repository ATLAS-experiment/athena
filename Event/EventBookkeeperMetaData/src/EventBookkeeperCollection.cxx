/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// Implementation file for class EventBookkeeperCollection
// Author: David Cote, September 2008. <david.cote@cern.ch>
/////////////////////////////////////////////////////////////////// 
#include "EventBookkeeperMetaData/EventBookkeeperCollection.h"

////////////////
/// Constructors
////////////////

EventBookkeeperCollection::EventBookkeeperCollection(): DataVector<EventBookkeeper> (SG::OWN_ELEMENTS) {}

EventBookkeeperCollection::EventBookkeeperCollection( const EventBookkeeperCollection& rhs ) : 
   DataVector<EventBookkeeper> (rhs) {}
 
EventBookkeeperCollection& EventBookkeeperCollection::operator=( const EventBookkeeperCollection& rhs ) 
   {
     if ( this != &rhs ) {
       DataVector<EventBookkeeper>::operator=(rhs);
     }
   return *this;
   }

EventBookkeeperCollection::EventBookkeeperCollection( const SG::OwnershipPolicy own ) :
  DataVector<EventBookkeeper>(own)
{}

EventBookkeeper* EventBookkeeperCollection::GetElement( const EventBookkeeper* book ){
  for(unsigned int i=0; i<this->size(); i++){
    if( this->at(i)->isEqualTo( book ) ){ return this->at(i); }
  }  
  //if book not found, return null pointer (0x0)
  EventBookkeeper* nullElement=0;
  return nullElement;
}

void EventBookkeeperCollection::UpdateFromColl( const EventBookkeeperCollection* coll ){
  //Loop through coll, if element already in self update event count, otherwise create new element
  for(unsigned int i=0; i<coll->size(); i++){
    const EventBookkeeper* book_i=coll->at(i);
    EventBookkeeper* tmpBook = this->GetElement( book_i );
    if( tmpBook ){
      tmpBook->updateAcceptedEventsIncludingFamily(book_i);
    }
    else{
      EventBookkeeper* newBook = new EventBookkeeper( *book_i );
      this->push_back( newBook );
    }
  }
  return;
}

EventBookkeeperCollection* EventBookkeeperCollection::GetCopyWithFlatStructure(){
  //Just a simple vector of pointers, nothing new is constructed, but now with a flat structure
  EventBookkeeperCollection PtrFLAT (SG::VIEW_ELEMENTS);
  for (EventBookkeeper* bk : *this) {
    PtrFLAT.push_back(bk);
    bk->fillWithWholeFamily( &PtrFLAT ); //family does not include topParent
  }

  //Now a make a flat-structure collection with new objects
  auto CopyFLAT = std::make_unique<EventBookkeeperCollection>();
  for (EventBookkeeper* bk : PtrFLAT) {
    EventBookkeeper* tmp = bk->DeepCopyForFlatStructure( &PtrFLAT );
    CopyFLAT->push_back( tmp );
  }

  //Set parentIndex of the CopyFLAT elements
  for(unsigned int i=0; i<CopyFLAT->size(); i++){ 
    if(CopyFLAT->at(i)->m_childrenIndices->size()>0){
      //parentIndex=i for children of element i
      std::vector<unsigned int>* childrenIndices=CopyFLAT->at(i)->m_childrenIndices;
      for(unsigned int j=0; j<childrenIndices->size(); j++){
	CopyFLAT->at(childrenIndices->at(j))->m_parentIndex=i;
      }
    }
  }
  //The caller now owns CopyFLAT and its content
  return CopyFLAT.release();
}

