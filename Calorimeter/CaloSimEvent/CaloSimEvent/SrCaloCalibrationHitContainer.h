/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// SrCaloCalibrationHitContainer
// Based on CaloCalibrationHitContainer 26-Jan-2004 William Seligman
// 23/06/25 Frederic DEJEAN

// This class exists to provides two features that an
// AthenaHitsVector<CaloCalibrationHit> does not provide on its own:

// - a CLID for StoreGate

// - a std::string method that can be used to examine the contents of
// the container.

#ifndef CaloSimEvent_SrCaloCalibrationHitContainer_h
#define CaloSimEvent_SrCaloCalibrationHitContainer_h

#include "AthenaKernel/CLASS_DEF.h"
#include "CaloSimEvent/CaloCalibrationHit.h"
#include "HitManagement/AthenaHitsVector.h"

class SrCaloCalibrationHitContainer
    : public AthenaHitsVector<CaloCalibrationHit> {
 public:
  /** Constructor of SrCaloCalibrationHitContainer */
  SrCaloCalibrationHitContainer(
      const std::string& collectionName = "DefaultCollectionName");

  /** Destructor */
  virtual ~SrCaloCalibrationHitContainer();

  /**
     Returns a string containing the description of this <br>
     SrCaloCalibrationHitContainer with a dump of all the hits
     that it contains<br>
     Can be used in printouts <br>
  */
  virtual operator std::string() const;
};

CLASS_DEF(SrCaloCalibrationHitContainer, 1098120221, 1)

class StoredSrLArCalibHitContainers
/** @brief store pointers to the different hit collections */
{
 public:
  /** Constructor */
  StoredSrLArCalibHitContainers()
      : activeHitCollection(0),
        inactiveHitCollection(0),
        deadHitCollection(0) {}

  /** Active calibration Hits */
  SrCaloCalibrationHitContainer* activeHitCollection;

  /** Inactive calibration Hits */
  SrCaloCalibrationHitContainer* inactiveHitCollection;

  /** Dead calibration Hits */
  SrCaloCalibrationHitContainer* deadHitCollection;
};

CLASS_DEF(StoredSrLArCalibHitContainers, 1229549986, 1)

#endif // CaloSimEvent_SrCaloCalibrationHitContainer_h
