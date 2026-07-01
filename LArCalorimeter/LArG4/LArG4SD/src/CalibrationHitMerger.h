/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4SD_CALIBRATIONHITMERGER_H
#define LARG4SD_CALIBRATIONHITMERGER_H 1

// STL includes
#include <set>
#include <string>
#include <vector>

// Framework includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"

namespace LArG4 {

  /** @class CalibrationHitMerger

      @brief Athena Algorithm that merges a given set of CaloCalibrationHitContainers into a single CaloCalibrationHitContainers - based on CollectionMerger by Elmar Ritsch.

     */
  class CalibrationHitMerger final : public AthAlgorithm {

  public:
    /** Constructor */
    CalibrationHitMerger( const std::string& name, ISvcLocator* pSvcLocator );

    /** Destructor */
    virtual ~CalibrationHitMerger() = default;

    /** Athena algorithm's interface methods */
    virtual StatusCode  initialize() override final;
    virtual StatusCode  execute(const EventContext& ctx)    override final;

  private:
    class LessHit {
    public:
      // Allow a lookup in a calibrationHits_t using a const pointer.
      using is_transparent = std::true_type;
      bool operator() ( const CaloCalibrationHit* p, const CaloCalibrationHit* q ) const
      {
        return p->Less(q);
      }
    };
    typedef std::set< CaloCalibrationHit*, LessHit >  calibrationHits_t;

    /** Initialize the given VarHandleKey */
    StatusCode initializeVarHandleKey( SG::VarHandleKey& varHandleKey ) const;

    SG::ReadHandleKeyArray<CaloCalibrationHitContainer> m_inputHits{this, "InputHits", {}, "Input collection ReadHandleKeys"};

    SG::WriteHandleKey<CaloCalibrationHitContainer> m_outputHits{this, "OutputHits", "", "Output collection WriteHandleKeys"};
  };

}

#endif //> !LARG4SD_CALIBRATIONHITMERGER_H
