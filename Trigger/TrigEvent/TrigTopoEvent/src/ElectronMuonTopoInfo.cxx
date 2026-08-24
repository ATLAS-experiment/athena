/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**************************************************************************
 **
 **   File: Trigger/TrigEvent/TrigTopoEvent/ElectronMuonTopoInfo.cxx
 **
 **   Description: -Class for description of combined electron-muon object for 
 ** 		algorithms in TrigEgammaMuonCombHypo package
 **
 **   Author: Pavel Jez <pavel.jez@cern.ch>
 **
 **   Created:   Apr 9, 2011
 **
 **
 **************************************************************************/ 

#include "TrigTopoEvent/ElectronMuonTopoInfo.h"
#include <sstream>
#include <iostream>

// constructors
ElectronMuonTopoInfo::ElectronMuonTopoInfo():   m_roiWord(-1),
						m_DeltaPhi(-1),
						m_DeltaR(-1),
						m_InvMass(-1),
						m_electronValid(false),
						m_oppositeCharge(false),
						m_vertexState(0)
  
{}

ElectronMuonTopoInfo::ElectronMuonTopoInfo(int roiWord, float deltaPhi, float deltaR, float invMass, bool el_valid, 
					   bool oppositeCharge, unsigned short vertexState): m_roiWord(roiWord), 
											     m_DeltaPhi(deltaPhi),
											     m_DeltaR(deltaR),
											     m_InvMass(invMass),
											     m_electronValid(el_valid),
											     m_oppositeCharge(oppositeCharge),
											     m_vertexState(vertexState)
  
{}

// destructor    
    ElectronMuonTopoInfo::~ElectronMuonTopoInfo() {}
    

// set methods 
  void 	ElectronMuonTopoInfo::SetRoiWord(int RoiWord){m_roiWord = RoiWord;}
  void 	ElectronMuonTopoInfo::SetDeltaPhi(float DeltaPhi){m_DeltaPhi= DeltaPhi;}
  void 	ElectronMuonTopoInfo::SetDeltaR(float DeltaR){m_DeltaR= DeltaR;}
  void 	ElectronMuonTopoInfo::SetInvMass(float InvMass){m_InvMass = InvMass;}
  void  ElectronMuonTopoInfo::SetElecValid(bool ElecValid){m_electronValid = ElecValid;}
  void  ElectronMuonTopoInfo::SetOppositeCharge(bool OppositeCharge){m_oppositeCharge = OppositeCharge;} 
  void 	ElectronMuonTopoInfo::SetVertexState(unsigned short vertexState){m_vertexState = vertexState;}

// text output  
   std::string str( const ElectronMuonTopoInfo& d )
   {
		std::stringstream ss;
		ss 	<< "ElectronMuonTopoInfo at address: " << &d
			<< " RoIWord: " 	<< d.RoiWord()
			<< " Delta Phi: " 	<< d.DeltaPhi()
			<< " Delta R: "		<< d.DeltaR()
			<< " Invariant mass: " 	<< d.InvMass()
			<< " electron valid: " 	<< d.ElecValid()
			<< " opposite charge: " << d.OppositeCharge()
			<< " vertex state: " 	<< d.VertexState();
		return ss.str();
	}
	  
	MsgStream& operator<< ( MsgStream& m, const ElectronMuonTopoInfo& d )
	{
		return( m << str ( d ) );
	}
	
	bool operator==( const ElectronMuonTopoInfo& d1,  const ElectronMuonTopoInfo& d2 )
	{
		return(d1.DeltaPhi()==d2.DeltaPhi() && d1.DeltaR()==d2.DeltaR() && d1.InvMass()==d2.InvMass() 
		&& d1.OppositeCharge()==d2.OppositeCharge() && d1.VertexState() == d2.VertexState() );
	}
