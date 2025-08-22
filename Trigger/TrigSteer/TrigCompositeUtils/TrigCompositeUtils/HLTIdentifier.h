/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigCompositeUtils_HLTIdentifier_h
#define TrigCompositeUtils_HLTIdentifier_h

#include <string>
#include <vector>
#include <set>

#include "AsgMessaging/MsgStream.h"
#include "xAODTrigger/TrigCompositeContainer.h"

/**
 * @brief An trigger identifier class, used to provide mapping fromt the human readable IDs to efficienct unsigned ints
 **/
namespace HLT {
class Identifier {
public:

  static HLT::Identifier fromToolName( const std::string& tname );
  /**
   * @brief constructs identifier from human redable name
   **/  
  explicit Identifier( const std::string& stringID );

  /**
   * @brief Construct wiht numeric ID
   **/
 Identifier( TrigCompositeUtils::DecisionID id ) : m_id( id ) {}

  /**
   * @brief reports human redable name
   **/  
  std::string name() const;
  
  /**
   * @brief numeric ID
   **/    
  inline TrigCompositeUtils::DecisionID numeric() const { return m_id; }
  inline operator TrigCompositeUtils::DecisionID () const { return numeric(); }

  /**
   *  @brief comparisons, for containers of identifiers
   **/      
  bool operator == ( const Identifier& rhs )  const { return numeric() == rhs.numeric(); }
  bool operator == ( TrigCompositeUtils::DecisionID id )  const { return numeric() == id; }
  bool operator < ( const Identifier& rhs )  const { return numeric() < rhs.numeric(); }
  bool operator < ( TrigCompositeUtils::DecisionID id ) const { return numeric() < id; } 
private:
  TrigCompositeUtils::DecisionID m_id;

};
 typedef std::vector<HLT::Identifier> IDVec;
 typedef std::set<HLT::Identifier> IDSet;
}

MsgStream& operator<< ( MsgStream& m, const HLT::Identifier& id );



#endif // HLTIdentifier
