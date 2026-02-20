/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DECISIONHANDLING_VIEWCREATORMUONSUPERROITOOL_H
#define DECISIONHANDLING_VIEWCREATORMUONSUPERROITOOL_H

#include "GaudiKernel/SystemOfUnits.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/WriteHandleKey.h"
#include "DecisionHandling/IViewCreatorROITool.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

/**
 * @class ViewCreatorMuonSuperROITool
 * Creates a new (super)-ROI combining windows centred on muons passing eta/pt selection
 * criteria, extracted from a single decision object (which would normally link the FSRoI).
 *
 * Stores this new ROI in the output container, and links it to the Decision Object
 *
 * The new EventView spawned by the parent EventViewCreatorAlgorithm of this tool will process in this new ROI.
 *
 * This tool is mainly intended to create a view for bmumux tracking operations, beginning from muons.
 **/
  class ViewCreatorMuonSuperROITool : public extends<AthAlgTool, IViewCreatorROITool>
{
public:
  ViewCreatorMuonSuperROITool(const std::string& type, const std::string& name, const IInterface* parent);

  virtual ~ViewCreatorMuonSuperROITool() = default;

   virtual StatusCode initialize() override;

  /**
   * @brief Tool interface method.
   **/
  virtual StatusCode attachROILinks(TrigCompositeUtils::DecisionContainer& decisions, const EventContext& ctx) const override;


 public:
  SG::WriteHandleKey< TrigRoiDescriptorCollection > m_roisWriteHandleKey {this,"RoisWriteHandleKey","",
    "Name of the ROI collection produced by this tool."};

  // JK Need to change this
  //  Gaudi::Property< std::string > m_featureLinkName{this,"FeatureLinkName","PreselMuons",
  //  "The name of the decoration holding the list of muons used for bmumux"};

  Gaudi::Property< std::string > m_iParticleLinkName{this,"IParticleLinkName","feature",
    "Name of linked IParticle object to centre the new ROI on. Normally the 'feature' from the previous Step."};


//JK  Gaudi::Property< double > m_jetMinPt{this,"JetMinPt",20*Gaudi::Units::GeV,
//JK    "Minimum jet pT used for RoI creation"};

  // Use rapidity?
  //JK   Gaudi::Property< double > m_jetMaxAbsEta{this,"JetMaxAbsEta",2.5,
  //JK    "Maximum absolute jet pseudorapidity used for RoI creation"};

  Gaudi::Property< double > m_roiEtaWidth{this,"RoIEtaWidth",0.5,
    "Extent of the ROI in eta from its centre"};

  Gaudi::Property< double > m_roiPhiWidth{this,"RoIPhiWidth",0.5,
    "Extent of the ROI in phi from its centre"};

  // Need to get beamspot position from somewhere to recentre?
  Gaudi::Property< double > m_roiZedWidth {this,"RoIZedWidth",50.0,
      "Z Half Width in mm"};

  Gaudi::Property< bool > m_isBhh {this,"isBhh",false,
      "true for HLT_muX_bBhh chains"};

};

#endif //> !DECISIONHANDLING_VIEWCREATORMUONSUPERROITOOL_H
