// -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONEVENT_TRIGMUONEF_H
#define TRIGMUONEVENT_TRIGMUONEF_H



// Gaudi/Athena include(s):
#include "EventKernel/INavigable4Momentum.h"
#include "FourMom/P4IPtCotThPhiM.h"
#include "Navigation/NavigableTerminalNode.h"
#include "AthenaKernel/CLASS_DEF.h"
// STL include(s):
#include <string>
#include <map>

// Forward declaration(s):
class MsgStream;

class TrigMuonEF : public P4IPtCotThPhiM,
                   public NavigableTerminalNode,
                   virtual public INavigable4Momentum {

public: 

   /** Default constructor */
   TrigMuonEF( ) = default;

   //  /** Default constructor */
   TrigMuonEF( double theIPt, double theCotTh, double thePhi, double theM );

   /** Copy constructor */
   TrigMuonEF( const TrigMuonEF& rhs ) = delete;

   /** Assignment operator */
   TrigMuonEF& operator=( const TrigMuonEF& rhs ) = delete;

   /** Destructor */
   virtual ~TrigMuonEF();

   /* set methods */
   /** Set MuonCode */
   void set_muonCode( int theMuonCode );

   /** Set RoINum */
   void set_RoINum( const std::string& theRoINum );

   /** Set Charge */
   void set_Charge( double theCharge );

   /* get methods */
   /** Code for type of muon */
   int  MuonCode() const { return m_muonCode; }

   /** Seeding algorithm RoI number */
   std::string  RoINum() const { return m_roi; }
   
   /** Charge */
   double Charge() const { return m_charge; }
   double charge() const { return this->Charge(); }

private:
   /** what kind of track we are dealing with (Moore, StandAlone, Combined) */
   int m_muonCode{0};
   std::string m_roi;
   double m_charge{0};

};

/// Helper function for printing the object
std::string str( const TrigMuonEF& d );
/// Helper operator for printing the object
MsgStream& operator<< ( MsgStream& m, const TrigMuonEF& d );

/// Operator comparing two TrigMuonEF objects for equality
bool operator== ( const TrigMuonEF& a, const TrigMuonEF& b );
/// Operator comparing two TrigMuonEF objects for inequality
inline bool operator!= ( const TrigMuonEF& a, const TrigMuonEF& b ) {
   return !( a == b );
}

/// Comparison with feedback
void diff( const TrigMuonEF& a, const TrigMuonEF& b,
           std::map< std::string, double >& variableChange );

CLASS_DEF( TrigMuonEF , 60689128 , 1 )


#endif // TRIGMUONEVENT_TRIGMUONEF_H
