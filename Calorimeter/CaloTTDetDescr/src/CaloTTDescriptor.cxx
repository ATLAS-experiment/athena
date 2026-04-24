/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Liquid Argon detector description package
 -----------------------------------------
 ***************************************************************************/

//<doc><file>	$Id: CaloTTDescriptor.cxx,v 1.5 2006-02-15 09:05:19 fledroit Exp $
//<version>	$Name: not supported by cvs2svn $

//<<<<<< INCLUDES                                                       >>>>>>

#include "CaloTTDetDescr/CaloTTDescriptor.h"

//FLG#include "CaloIdentifier/CaloLVL1_ID.h"

#include <iostream>
#include <iomanip>

CaloTTDescriptor::CaloTTDescriptor(float eta_min,   float eta_max,   float deta, 
				   float phi_min,   float phi_max,   float dphi,
				   int  sign_eta,   short n_lay)
    : 
    m_sign_eta	(sign_eta),
    m_eta_min	(eta_min),
    m_eta_max	(eta_max),
    m_deta	(deta),
    m_phi_min	(phi_min),
    m_phi_max	(phi_max),
    m_dphi	(dphi),
    m_nEta      ((short) ((eta_max - eta_min)/deta + 0.501)),
    m_nPhi      ((short) ((phi_max - phi_min)/dphi + 0.501)),
    m_nLay      (n_lay)
{
}



void	
CaloTTDescriptor::print	() const
{
    std::cout << "\n CaloTTDescriptor print: \n\n" << m_id;
    std::cout << " Calo LVL1 Trigger Towers: \n";
    std::cout << "  eta min  eta max     deta   phi min   phi max      dphi   nLay\n";
    std::cout << std::setiosflags(std::ios::fixed);
    std::cout << std::setw(9) << std::setprecision(4) << m_eta_min << " " 
	      << std::setw(9) << std::setprecision(4) << m_eta_max << " " 
	      << std::setw(9) << std::setprecision(4) << m_deta    << " " 
	      << std::setw(9) << std::setprecision(4) << m_phi_min << " " 
	      << std::setw(9) << std::setprecision(4) << m_phi_max << " " 
	      << std::setw(9) << std::setprecision(4) << m_dphi    << " "
	      << std::setw(9) << std::setprecision(4) << m_nLay    << " "
	      << std::endl;	    
}

    
void 
CaloTTDescriptor::set (float               eta_min,        
		       float               eta_max,
		       float               deta,
		       float               phi_min,
		       float               phi_max,
		       float               dphi,
		       int                 sign_eta,
		       short               n_lay)
{
  m_eta_min  = eta_min;
  m_eta_max  = eta_max;
  m_deta     = deta;
  m_phi_min  = phi_min;
  m_phi_max  = phi_max;
  m_dphi     = dphi;
  m_nEta     = (short) ((eta_max - eta_min)/deta + 0.501);
  // Dummy conditional to prevent these two statements from being
  // evaluated using a vectorized instruction.  Otherwise, we can
  // get a FPE in the clang build from the division because there
  // are two unused vector lanes.
  if (m_nEta > 0) {
    m_nPhi     = (short) ((phi_max - phi_min)/dphi + 0.501);
  }
  else {
    m_nPhi = 0;
  }
  m_sign_eta = sign_eta;
  m_nLay = n_lay;
}


    








