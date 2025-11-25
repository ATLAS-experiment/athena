/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDC_EscapedEnergyProcessing.h"
#include "ZDC_G4CalibSD.h"


#include "G4MultiSensitiveDetector.hh"
#include "G4TouchableHandle.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4VSensitiveDetector.hh"

#undef DEBUG_PROCESS


ZDC_EscapedEnergyProcessing::ZDC_EscapedEnergyProcessing(ZDC_G4CalibSD* SD)
  : m_defaultSD(SD)
{}

ZDC_EscapedEnergyProcessing::~ZDC_EscapedEnergyProcessing()
{;}

G4bool ZDC_EscapedEnergyProcessing::Process( G4Step* fakeStep ) {
  // Escaped energy requires special processing.  The energy was not
  // deposited in the current G4Step, but at the beginning of the
  // particle's track.  For example, if a neutrino escapes the
  // detector, the energy should be recorded at the point where the
  // neutrino was created, not at the point where it escaped the
  // detector.

  // The calibration sensitive detector has a special routine to
  // handle this: CalibrationSensitiveDetector::SpecialHit().  We
  // have to locate the correct sensitive detector in order to
  // deposit the escaped energy.

  // Escaped energies from non-sensitive volumes requires a special
  // sensitive detector.

  G4StepPoint*        fakePreStepPoint = fakeStep->GetPreStepPoint();
  G4StepPoint*        fakePostStepPoint = fakeStep->GetPostStepPoint();
  G4TouchableHandle a_touchableHandle = fakePreStepPoint->GetTouchableHandle();

  // Here is the actual location of the escaped energy, as the
  // fourth component of a vector.  (Why not just pass the energy in
  // the G4Step?  Because CalibrationSensitiveDetector::SpecialHit()
  // is not designed just for escaped energy, but any unusual cases
  // in which we have to deposit any of the calibration energies in
  // a place other than the current step.)

  std::vector<G4double> energies;
  energies.resize(4,0.);
  energies[3] = fakeStep->GetTotalEnergyDeposit();//a_energy;

  // If we find a calibration sensitive detector in the original
  // volume, use that sensitive detector.  If we don't, use a
  // "default" sensitive detector that's designed to accumulate
  // these "dead material" energies.

  // Find out in which physical volume our hit is located.
  G4VPhysicalVolume* physicalVolume = a_touchableHandle->GetVolume();

  // If the volume is valid...
  if (ATH_LIKELY(physicalVolume))
    {
#ifdef DEBUG_PROCESS
      G4cout << "ZdcG4::ZDC_EscapedEnergyProcessing::Process - "
             << " particle created in volume '"
             << physicalVolume->GetName()
             << "'" << G4endl;
#endif

      G4LogicalVolume* logicalVolume = physicalVolume->GetLogicalVolume();

      // To prevent some potential problems with uninitialized values,
      // set the material of the step points.
      fakePreStepPoint->SetMaterial( logicalVolume->GetMaterial()  );
      fakePostStepPoint->SetMaterial( logicalVolume->GetMaterial()  );

      // Is this volume associated with a sensitive detector?
      G4VSensitiveDetector* sensitiveDetector =
        logicalVolume->GetSensitiveDetector();

      if (ATH_LIKELY(sensitiveDetector)){
#ifdef DEBUG_PROCESS
          G4cout << "   ... which has sensitive detector '" << sensitiveDetector->GetName() << "'" << G4endl;
#endif
          ZDC_G4CalibSD* zdcG4CalibSD(nullptr);
          G4MultiSensitiveDetector* zdcG4MultSD = dynamic_cast<G4MultiSensitiveDetector*>(sensitiveDetector);
          // The multiSensitiveDetector is needed for the quartz rods and RPD fibers that have FiberSD and G4CalibSD associated with it
          if(zdcG4MultSD !=0){
              G4bool found = false;
              for(unsigned int i=0; i<zdcG4MultSD->GetSize(); i++){
                  zdcG4CalibSD = dynamic_cast<ZDC_G4CalibSD*>(zdcG4MultSD->GetSD(i));
                  if (zdcG4CalibSD){
                      // We found one.  Process the energy.
                      found = true;
#ifdef DEBUG_PROCESS
                      G4cout << "   ... which contains calibration sensitive detector '" << zdcG4CalibSD->GetName() << "'" << G4endl;
#endif
                      zdcG4CalibSD->SpecialHit(fakeStep,energies);
                    }
                }// -- for (1)
              if(ATH_UNLIKELY(!found)){
#ifdef DEBUG_PROCESS
                  G4cout << "ZdcG4::ZDC_EscapedEnergyProcessing::Process - "
                         << " particle (x,y,z)=("
                         << a_point.x() << ","
                         << a_point.y() << ","
                         << a_point.z() << ") with energy="
                         << a_energy
                         << " was created in volume '"
                         << physicalVolume->GetName()
                         << "'; could not find a CalibrationSensitiveDetector"
                         << " within  ZdcG4MultSD'"
                         << zdcG4MultSD->GetName() << "'"
                         << G4endl;
                  G4cout << "   Using default sensitive detector." << G4endl;
#endif
                  m_defaultSD->SpecialHit( fakeStep, energies );
                  return false; // error
                }// - !found (1)

            } else { // zdcG4MultSD == 0
              // Next possibility: ZDC_G4CalibSD
              zdcG4CalibSD = dynamic_cast<ZDC_G4CalibSD*>(sensitiveDetector);
              if(ATH_UNLIKELY(zdcG4CalibSD)){
#ifdef DEBUG_PROCESS
                  G4cout << "   ... which is a ZDC_G4CalibSD " << G4endl;
#endif
                  zdcG4CalibSD->SpecialHit( fakeStep, energies );
              }
              else // zdcG4CalibSD==0
                {
                  // --- backwards compatibility ---

                  // We've found a sensitive detector.  Is it a CalibrationSensitiveDetector?
                  ZDC_G4CalibSD* calibSD = dynamic_cast<ZDC_G4CalibSD*>(sensitiveDetector);
                  if ( calibSD != 0 ){
                      // It is!  Process the energy.
#ifdef DEBUG_PROCESS
                      G4cout << "   ... which is a calibration sensitive detector " << G4endl;
#endif
                      calibSD->SpecialHit( fakeStep, energies );
                  } else {
#ifdef DEBUG_PROCESS
                      G4cout << "ZdcG4::ZDC_EscapedEnergyProcessing::Process - "
                             << " particle (x,y,z)=("
                             << a_point.x() << ","
                             << a_point.y() << ","
                             << a_point.z() << ") with energy="
                             << a_energy
                             << " was created in volume '"
                             << physicalVolume->GetName()
                             << "' with sensitive detector '"
                             << sensitiveDetector->GetName()
                             << "' which is not a CalibrationSensitiveDetector."
                             << G4endl;
                      G4cout << "   Using default sensitive detector." << G4endl;
#endif
                      // of type ZDC_G4CalibSD
                      m_defaultSD->SpecialHit( fakeStep, energies );
                      return false; // error
                    }
                }
            }
        } else {
          // If we reach this point, then the particle whose energy
          // has escaped was created outside of a sensitive region.
#ifdef DEBUG_PROCESS
          G4cout << "ZdcG4::ZDC_EscapedEnergyProcessing::Process - "
                 << " particle (x,y,z)=("
                 << a_point.x() << ","
                 << a_point.y() << ","
                 << a_point.z() << ") with energy="
                 << a_energy
                 << " was created in volume '"
                 << physicalVolume->GetName()
                 << "' which is not sensitive."
                 << G4endl;
          G4cout << "   Using default sensitive detector." << G4endl;
#endif
          m_defaultSD->SpecialHit( fakeStep, energies );
          return false; // error
        }
    }

  // Processing was OK.
  return true;
}
