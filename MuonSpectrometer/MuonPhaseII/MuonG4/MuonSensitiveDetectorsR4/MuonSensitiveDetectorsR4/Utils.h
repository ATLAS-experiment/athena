/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSENSITIVEDETECTORSR4_UTILS_H
#define MUONSENSITIVEDETECTORSR4_UTILS_H
/// Include the common definitions from the MuonReadoutGeometry
#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>


#include <G4VTouchable.hh>
#include <G4Track.hh>


namespace MuonG4R4 {


    /** 
     * @brief Extracts the local -> global transformation from a TouchableHistory at a given level
     * @param history: Pointer to the history of the G4 track
     * @param level: Level inside the history - should not exceed GetNDepth() of the history
     * @return: The transformation to go from the local into the global coordinate frame
     * 
    */
    inline Amg::Transform3D getTransform(const G4VTouchable* history, unsigned int level) {
        	return Amg::Translation3D{Amg::Hep3VectorToEigen(history->GetTranslation(level))}*
                                      Amg::CLHEPRotationToEigen(*history->GetRotation(level)).inverse();
    }
}
    inline std::ostream& operator<<(std::ostream& ostr, const G4Track& step) {
        ostr<<"position: "<<Amg::toString(Amg::Hep3VectorToEigen(step.GetPosition()),2)<<", ";
        ostr<<"momentum: "<<Amg::toString(Amg::Hep3VectorToEigen(step.GetMomentum()),2)<<", ";
        ostr<<"velocity: "<<step.GetVelocity()<<", ";
        ostr<<"time: "<<step.GetGlobalTime()<<", ";
        ostr<<"mass: "<<step.GetDefinition()->GetPDGMass()<<", ";
        ostr<<"kinetic energy: "<<step.GetKineticEnergy()<<", ";
        ostr<<"charge: "<<step.GetDefinition()->GetPDGCharge()<<", ";
        ostr<<"G4Id: "<<step.GetTrackID();
        return ostr;
    }
    inline std::ostream& operator<<(std::ostream& ostr, const G4StepPoint& step) {
        ostr<<"position: "<<Amg::toString(Amg::Hep3VectorToEigen(step.GetPosition()),2)<<", ";
        ostr<<"momentum: "<<Amg::toString(Amg::Hep3VectorToEigen(step.GetMomentum()),2)<<", ";
        ostr<<"time: "<<step.GetGlobalTime()<<", ";
        ostr<<"mass: "<<step.GetMass()<<", ";
        ostr<<"kinetic energy: "<<step.GetKineticEnergy()<<", ";
        ostr<<"charge: "<<step.GetCharge();
        return ostr;
    }


#endif