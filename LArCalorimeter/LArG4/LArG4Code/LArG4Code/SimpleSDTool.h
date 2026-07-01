/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4CODE_SIMPLESDTOOL_H
#define LARG4CODE_SIMPLESDTOOL_H

/**
 * @file SimpleSDTool.h
 * @brief Defines the SimpleSDTool class
 * @author Steve Farrell <Steven.Farrell@cern.ch>
 * @date 2016-03-26
 */

// System includes
#include <string>
#include <vector>

// G4Atlas includes
#include "G4AtlasTools/SensitiveDetectorBase.h"

#include "HitManagement/HitCollectionMap.h"

// Local includes
#include "LArG4SimpleSD.h"

// External includes
#include "CLHEP/Units/SystemOfUnits.h"

// Forward declarations
class ILArCalculatorSvc;
class LArEM_ID;
class LArFCAL_ID;
class LArHEC_ID;
class LArG4SimpleSD;

namespace LArG4
{

  /// @class SimpleSDTool
  /// @brief A base class for tools that manage LArG4SimpleSDs.
  ///
  /// The tool owns only the Athena-event lifecycle of the output collection.
  /// Geant4 SD instances write through `LArHitContainerBuilder`, which carries
  /// all per-event merge state and the ordered regular-SD partitions.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class SimpleSDTool : public SensitiveDetectorBase
  {

    public:

      /// Constructor
      SimpleSDTool(const std::string& type, const std::string& name,
                   const IInterface* parent);

      /// Initialize the tool
      StatusCode initialize() override final;

      /// Create and register all SDs for the current thread.
      StatusCode initializeSD() override final;

      /// Create event-owned hit collections.
      StatusCode SetupEvent(HitCollectionMap&) override final;

      /// Finalize and record event-owned hit collections.
      StatusCode Gather(HitCollectionMap&) override final;

    protected:

      /// Initialize Calculator Services
      virtual StatusCode initializeCalculators() { return StatusCode::SUCCESS; }

      /// Helper method to create one SD
      LArG4SimpleSD* 
      makeOneSD(const std::string& name, ILArCalculatorSvc* calc,
                const std::vector<std::string>& volumes) const;

      /// Are we handling frozen shower fast sim?
      bool useFrozenShowers() const { return m_useFrozenShowers; }

      /// Collection receiving the hits for this tool.
      virtual std::string hitCollectionName() const;

    protected:

      /// @name configuration properties
      /// @{

      /// What time binning type for regular hits?
      Gaudi::Property<std::string> m_timeBinType{this, "TimeBinType", "Default"};
      /// What time bin width for regular hits?
      Gaudi::Property<float> m_timeBinWidth{this, "TimeBinWidth", 2.5*CLHEP::ns};
      /// Is there going to be a fast simulation coming into this SD?
      Gaudi::Property<G4bool> m_useFrozenShowers{this, "UseFrozenShowers", false};

      /// @}

      /// @name Calo identifier helpers
      /// @{
      const LArEM_ID*       m_larEmID{nullptr};
      const LArFCAL_ID*     m_larFcalID{nullptr};
      const LArHEC_ID*      m_larHecID{nullptr};
      /// @}

  }; // class SimpleSDTool

} // namespace LArG4

#endif
