/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_FIBER_SIMHIT
#define ZDC_FIBER_SIMHIT

#include "Identifier/Identifier.h"

class ZDC_SimFiberHit
{
 public:
  
  /**
   * @brief Default constructor. Should never be used
   */
  ZDC_SimFiberHit()
  : m_ID(Identifier()),
    m_Nphotons(-1),
    m_Edep(-1)
  {}
  
  /**
   * @brief Standard constructor 
   * @param id Volume identifier
   * @param nphot Number of photons generated in this volume
   * @param edep Energy deposited as light in this volume
  */
  ZDC_SimFiberHit(Identifier id, int nphot, double edep) 
  : m_ID(id),
    m_Nphotons(nphot),
    m_Edep(edep)
  {}

  /**@brief Copy constructor **/
  ZDC_SimFiberHit(const ZDC_SimFiberHit &right)
    : m_ID (right.m_ID),
      m_Nphotons (right.m_Nphotons),
      m_Edep (right.m_Edep)
  {
  }

  /**
   * @brief Copy constructor 
   */
  ZDC_SimFiberHit(const ZDC_SimFiberHit *right) : ZDC_SimFiberHit(*right){};
  
  Identifier getID      () const { return m_ID; }
  double     getEdep    () const { return m_Edep; }
  int        getNPhotons() const { return m_Nphotons; }

  /** 
   * @brief Assignment operator 
   */
  ZDC_SimFiberHit& operator=(const ZDC_SimFiberHit &right)
  {
    m_ID = right.m_ID;
    m_Nphotons = right.m_Nphotons;
    m_Edep = right.m_Edep;
    return *this;
  }

  /** 
   * @brief Just for checking if the ID is the same
   */
  bool operator == (const ZDC_SimFiberHit& h) const {
    return (m_ID == h.m_ID); 
  }

  /**
   * @brief Check if the ID is less than the compared hit. If they are the same, check if Nphotons is less than the compared hit
   * @param h Hit being compared against
   * @return True if the ID or Nphotons is less than the compared hit. False otherwise
   */
  bool  operator < (const ZDC_SimFiberHit& h) const {
    if(m_ID != h.m_ID){
      return m_ID < h.m_ID; 
    }else{
      return m_Nphotons < h.m_Nphotons; 
    }
  }

  /**
   * @brief Addition operator
   * @param h 
   * @return 
   */
  ZDC_SimFiberHit operator +(const ZDC_SimFiberHit& h){
    return ZDC_SimFiberHit(m_ID, m_Nphotons + h.m_Nphotons, m_Edep + h.m_Edep);
  }

  /**
   * @brief 
   * @param h 
   * @return 
   */
  ZDC_SimFiberHit operator +=(const ZDC_SimFiberHit& h){
    m_Nphotons += h.m_Nphotons;
    m_Edep += h.m_Edep;
    return *this;
  }

  void Add(int nphot, double edep){
    m_Nphotons += nphot;
    m_Edep += edep;
  }

  void Add(ZDC_SimFiberHit &h){
    m_Nphotons += h.m_Nphotons;
    m_Edep += h.m_Edep;
  }

  void Add(ZDC_SimFiberHit *h){
    Add(*h);
  }

  bool Equals(const ZDC_SimFiberHit& h) const { 
    return (m_ID == h.m_ID); 
  };
  bool Equals(const ZDC_SimFiberHit* h) const {
    return Equals (*h);
  }


 private:
  
  Identifier m_ID;
  int        m_Nphotons;
  double     m_Edep;
};

#endif


