/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#undef DEBUG_SD
#undef DEBUG_HITS

#include "LArG4TBECBeamChSensitiveDetector.h"
#include "LArG4TBECBeamChCalculator.h"
#include "LArG4TBSimEvent/LArG4TBECBeamChHit.h"

#include "G4Version.hh"
#include "G4Step.hh"
#include "G4ios.hh"

LArG4TBECBeamChSensitiveDetector::LArG4TBECBeamChSensitiveDetector(const G4String& a_name)
  : G4VSensitiveDetector(a_name)
  , m_calculator(std::make_unique< LArG4TBECBeamChCalculator>())
  , m_detectorName(a_name)
{
  // Construct a unique name for the collection of hits maintained by
  // a sensitive detector.  Hit collection names can't have any
  // slashes ("/"), because that confuses the name with the sensitive
  // detector names.
#if G4VERSION_NUMBER < 1100
  G4String HCname = m_detectorName.strip(G4String::both,'/');
  if ( HCname.contains('/') )
#else
  G4String HCname = G4StrUtil::strip_copy(m_detectorName,'/');
  if ( G4StrUtil::contains(HCname, '/') )
#endif
    {
      G4int lastSlash = HCname.rfind('/');
      HCname.erase(0,lastSlash+1);
    }

  // This is the name to search for in the hit-collection table.
  // Given a detector whose name is "/something/blah/duh/yada", the
  // hit collection name will be "yadaHits".
  m_HCname = HCname + "::Hits";

  // "collectionName" is defined in G4VSensitiveDetector.hh.
  collectionName.insert(m_HCname);

#ifdef DEBUG_SD
        G4cout << "LArG4TBECBeamChSensitiveDetector: Sensitive Detector <"
                   << m_detectorName << "> has hit collection <" << m_HCname
                   << ">" << G4endl;
#endif
}

void LArG4TBECBeamChSensitiveDetector::Initialize(G4HCofThisEvent* /*m_HCE*/)
{
        m_Hits.clear();
#ifdef DEBUG_SD
        G4cout << "LArG4TBECBeamChSensitiveDetector: initialized " << G4endl;
#endif
}


G4bool LArG4TBECBeamChSensitiveDetector::ProcessHits(G4Step* a_step,
                                                     G4TouchableHistory* /*ROhist*/)
{
        if(!m_calculator->Process(a_step)) return false;
        G4int ident = m_calculator->identifier();
        G4double x = m_calculator->x();
        G4double y = m_calculator->y();

#ifdef DEBUG_HITS
        std::cout << "LArG4TBECBeamChSensitiveDetector                    "
            << " ID=" << ident
            << " x=" << x
            << " y=" << y
            << std::endl;
#endif

        auto hit = std::make_unique<LArG4TBECBeamChHit>(ident, x, y);

#ifdef DEBUG_HITS
        std::cout << "LArG4TBECBeamChSensitiveDetector hit = " << hit.get() << std::endl;
#endif

        G4bool found = false;
        for (const auto & hit : m_Hits) {
                if(hit->getIdentifier() == ident){
                        found = true;
                        break;
                }
        }
        if(!found){
                m_Hits.push_back(std::move(hit));
                return true;
        } else {
#ifdef DEBUG_HITS
        std::cout << "LArG4TBECBeamChSensitiveDetector duplicate hit in"
                  << ident << std::endl;
#endif
                return false;
        }
}

void LArG4TBECBeamChSensitiveDetector::EndOfEvent(G4HCofThisEvent* /*m_HCE*/)
{
#ifdef DEBUG_SD
        G4cout << "LArG4TBECBeamChSensitiveDetector::EndOfEvent: "
               << "collectionName[0]=" << collectionName[0] << G4endl
                   << "LArG4TBECBeamChSensitiveDetector::EndOfEvent: "
                   << " m_Hits.size()=" << m_Hits.size() << G4endl;
#endif

#ifdef DEBUG_HITS
	for(const auto & hit : m_Hits) {
                std::cout << "LArG4TBECBeamChSensitiveDetector::EndOfEvent"
                          << " ID= " << hit->getIdentifier()
                          << " x=" << hit->X()
                                  << " y=" << hit->Y()
                                  << std::endl;
        }
#endif
}
