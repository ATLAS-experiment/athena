/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
   MdtCalibDbAlgR4 reads raw condition data and writes derived condition data to the condition store
*/

#ifndef MUONCALIBR4_MDTANALYTICCALIBALG_H
#define MUONCALIBR4_MDTANALYTICCALIBALG_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MdtCalibData/MdtCalibDataContainer.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

class TGraph;
namespace MuonCalib{
  class SamplePoint;
}
namespace MuonCalibR4{
    class MdtAnalyticRtCalibAlg : public AthHistogramAlgorithm {
      public:
        using AthHistogramAlgorithm::AthHistogramAlgorithm;
        virtual ~MdtAnalyticRtCalibAlg() = default;

        virtual StatusCode initialize() override final;
        virtual StatusCode execute() override final;

        enum class PolyType{
            ChebyChev,
            Legendre,
            Simple
        };
        using RtRelationPtr = MuonCalib::MdtFullCalibData::RtRelationPtr;

      private:

          void drawRt(const EventContext& ctx, 
                      const Identifier& detId,
                      const std::vector<MuonCalib::SamplePoint>& rtPoints,
                      const MuonCalib::MdtRtRelation& inRel) const;

          void drawResoFunc(const EventContext& ctx,
                            const Identifier& detId,
                            const std::vector<MuonCalib::SamplePoint>& resoPoints,
                            const MuonCalib::IRtResolution& inReso) const;

          /** @brief Translates the rt / tr & resolution relation from a look-up table 
           *         into the requested polynomial type. If translation fails, a nullptr is returned
           *  @param ctx: EventContext mainly used for visualization
           *  @param detId: Identifier of the associated Mdt multilayer (visualization)
           *  @param inRel: Pointer to the input rt relation to translate */
          RtRelationPtr translateRt(const EventContext& ctx,
                                    const Identifier& detId,
                                    const MuonCalib::MdtRtRelation& inRel) const;
          
          
          std::vector<Identifier> tubeIds(const Identifier& chId) const;
          
          void saveGraph(const std::string& path, std::unique_ptr<TGraph>&& graph) const;
          ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
          SG::ReadCondHandleKey<MuonCalib::MdtCalibDataContainer> m_readKey{this, "ReadKey", "MdtCalibConstantsR4"};
          SG::WriteCondHandleKey<MuonCalib::MdtCalibDataContainer> m_writeKey{this, "WriteKey", "MdtCalibConstants",
                                                                                            "Conditions object containing the calibrations"};   

          /** @brief Maximum order of the polynomial in use */
          Gaudi::Property<unsigned> m_maxOrder{this, "maxOrder", 12};
          /** @brief Toggle the polynomial for the Rt-relation: ChebyChev or Legendre */
          Gaudi::Property<int> m_polyTypeRt{this, "PolyTypeRt", static_cast<int>(PolyType::ChebyChev)};
          /** @brief Toggle the polynomial for the Rt-relation: ChebyChev or Legendre */
          Gaudi::Property<int> m_polyTypeTr{this, "PolyTypeTr", static_cast<int>(PolyType::Legendre)};
          /** @brief Toggle whether the resolution shall be also converted into a polynomial */
          Gaudi::Property<bool> m_fitRtReso{this, "FitRtReso", true};
          /** @brief Assignment of the relative uncertainty on each resolution data point */
          Gaudi::Property<double> m_relUncReso{this, "RelUncertOnReso", 0.01};
          /** @brief Maximal order to use for the resolution  */
          Gaudi::Property<unsigned> m_maxOrderReso{this, "maxOrderReso", 25};
          /** @brief Stop incrementing the order once the chi2CutOff is reached  */
          Gaudi::Property<float> m_chiCutOff{this,"chi2CutOff", 0.05};

          /** @brief Save diagnostic histograms */
          Gaudi::Property<bool> m_saveDiagnostic{this, "saveDiagnosticHist", true};
          /** @brief StreamName of the diagnostic histograms */
          Gaudi::Property<std::string> m_outStream{this, "OutStream", "MDTANALYTICRTS"};
          /** @brief Precision cut-off to treat two incoming rt-relations as equivalent */
          Gaudi::Property<unsigned> m_precCutOff{this,"precCutOff", 6};
          /** @brief At the end of the translation, it's checked whether all channels have been assigned. */
          Gaudi::Property<bool> m_fillMissingCh{this, "fillMissingCh", true};
          /** @brief Default t0 constant to use, in case there's  */
          Gaudi::Property<float> m_missingT0{this, "missingT0", 5.4597301};
    };
}



#endif
