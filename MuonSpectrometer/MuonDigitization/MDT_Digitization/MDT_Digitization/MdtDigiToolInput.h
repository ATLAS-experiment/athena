/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDT_DIGITIZATION_MDTDIGITOOLINPUT_H
#define MDT_DIGITIZATION_MDTDIGITOOLINPUT_H
#include "Identifier/Identifier.h"
/*-----------------------------------------------

   Created 7-5-2004 by Niels van Eldik
   Modified by Dinos Bachas

   20-08-2011 Modified by Oleg Bulekov. The electric charge has been added
   06-10-2011 Modified by Oleg Bulekov. The gamma factor has been added
Class to store input needed for the MDT_Digitization tools:
 - G4 driftradius
 - position along tube
 - magnetic field strength at hit position
 - local temperature
 - electric charge
 - gamma factor
 - hit Identifier

-----------------------------------------------*/

class MdtDigiToolInput {
public:

    MdtDigiToolInput(double radius, double posx, double field, double temp, double electrcharge, double gammafact, Identifier hitID) :
        m_radius(radius),
        m_xpos(posx),
        m_field(field),
        m_temperature(temp),
        m_electriccharge(electrcharge),
        m_gamma(gammafact),
        m_hitID(hitID) {}
    
    ~MdtDigiToolInput() = default;

    double radius() const { return m_radius; }
    double positionAlongWire() const { return m_xpos; }
    double magneticField() const { return m_field; }
    double temperature() const { return m_temperature; }
    double electriccharge() const { return m_electriccharge; }
    double gamma() const { return m_gamma; }
    Identifier getHitID() const { return m_hitID; }

private:
    double m_radius{0.};
    double m_xpos{0.};
    double m_field{0.};
    double m_temperature{0.};
    double m_electriccharge{0.};
    double m_gamma{0.};
    Identifier m_hitID{};
};

#endif
