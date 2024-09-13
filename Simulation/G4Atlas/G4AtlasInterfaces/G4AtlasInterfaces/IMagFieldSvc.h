/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IMagFieldSvc.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef G4ATLASINTERFACES_IMAGFIELDSVC_H
#define G4ATLASINTERFACES_IMAGFIELDSVC_H

// Framework includes
#include "GaudiKernel/IInterface.h"

namespace MagField {

/** @ class IMagFieldSvc

 @ author Elmar.Ritsch -at- cern.ch
 */
    class IMagFieldSvc: virtual public IInterface {

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
    public:

        /** Creates the InterfaceID and interfaceID() method */
        DeclareInterfaceID(IMagFieldSvc, 1, 0);

        /** constructor */
        IMagFieldSvc() : m_solenoidCurrent(0.0), m_toroidCurrent(0.0) {;}

        /** get B field value at given position */
        /** xyz[3] is in mm, bxyz[3] is in kT */
        /** if deriv[9] is given, field derivatives are returned in kT/mm */
        virtual void getField(const double *xyz, double *bxyz, double *deriv = nullptr) const = 0;

        /** get B field value on the z-r plane at given position */
        /** works only inside the solenoid; otherwise calls getField() above */
        /** xyz[3] is in mm, bxyz[3] is in kT */
        /** if deriv[9] is given, field derivatives are returned in kT/mm */
        virtual void getFieldZR(const double *xyz, double *bxyz, double *deriv = nullptr) const = 0;

        /** status of the magnets */
        bool solenoidOn() const { return solenoidCurrent() > 0.0; }
        bool toroidOn() const   { return toroidCurrent()   > 0.0; }
        float solenoidCurrent() const { return m_solenoidCurrent; }
        float toroidCurrent() const   { return m_toroidCurrent;   }

    protected:
        void setSolenoidCurrent(float current) { m_solenoidCurrent = current; }
        void setToroidCurrent  (float current) { m_toroidCurrent   = current; }

    private:
        float m_solenoidCurrent; // solenoid current in ampere
        float m_toroidCurrent;   // toroid current in ampere
    };
}

#endif //> !MAGFIELDINTERFACES_IMAGFIELDSVC_H
