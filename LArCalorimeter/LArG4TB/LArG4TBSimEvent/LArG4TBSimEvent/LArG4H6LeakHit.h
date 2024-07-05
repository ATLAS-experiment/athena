/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//=================================================
// LArG4H6LeakHit class
//
// information stored
//  - energy deposition in the leakege detector
//  - hit position
//  - store the particle types, kinetic and total energies
//  - leakage directionx
//=================================================

#ifndef LArG4H6LeakHit_h
#define LArG4H6LeakHit_h 1

#include "CLHEP/Vector/ThreeVector.h"
#include <vector>
class LArG4H6LeakHit 
{
 public:
     LArG4H6LeakHit() = default;
     LArG4H6LeakHit(int d){m_dir = d;};
     LArG4H6LeakHit(int dir, int pcode, float ekin, float etot);
     LArG4H6LeakHit(const LArG4H6LeakHit &right);

     LArG4H6LeakHit& operator=(const LArG4H6LeakHit &right);
     int operator==(const LArG4H6LeakHit &right);

//Set- Get- methods
 // energy deposit
     inline void SetEkin(double ed) { m_ekin = ed; }
     inline void AddEkin(double ed) { m_ekin += ed; }
     inline double GetEkin() const { return m_ekin; }
     inline void SetEtot(double ed) { m_etot = ed; }
     inline void AddEtot(double ed) { m_etot += ed; }
     inline double GetEtot() const { return m_etot; }

 // particle code
     inline void SetCode(int cod) { m_pcode = cod; }
     inline int GetCode() const { return m_pcode; }

 // position
     inline void SetPos(CLHEP::Hep3Vector p) { m_pos = p; }
     inline CLHEP::Hep3Vector GetPos() const { return m_pos; }

 // leakage direction
     inline void SetDir(int d) { m_dir = d; }
     inline int GetDir() const { return m_dir; }

 private:
     double m_ekin{0};   // kinetic energy of stopped particle
     double m_etot{0};   // total energy       -"-
     int    m_pcode{0};  // particle code, special codes for: deuterium 9001
                        //                                   tritium   9002
                        //                                   alpha     9003
                        //                                   He3       9004
     CLHEP::Hep3Vector m_pos; // absolute position of hit
     int    m_dir{0};   // leakage direction encoding
                        // 1,2     EMEC     x<0, x>0
                        // 3     EMEC down
                        // 4,5   EMEC back    -"-
                        // 6,7   HEC1         -"-
                        // 8     HEC1 down
                        // 9,10  HEC1 back    -"-
                        // 11,12  HEC2        -"-
                        // 13    HEC2 down
                        // 14    HEC2 back
};

typedef std::vector<LArG4H6LeakHit> LArG4H6LeakHitsCollection;

#endif
