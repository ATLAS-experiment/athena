/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUCALIBRATELC_H
#define TAURECTOOLS_TAUCALIBRATELC_H

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgTools/PropertyWrapper.h"
#include "tauRecTools/TauRecToolBase.h"
#include "xAODEventInfo/EventInfo.h"

class TH1;
class TF1;

/**
 * @brief Implementation of tau energy scale (TES) with eta and pile-up correction.
 * 
 * @author Margar Simonyan
 * @author Felix Friedrich
 *                                                                              
 */

class TauCalibrateLC : public TauRecToolBase {
  
  public:

    ASG_TOOL_CLASS2( TauCalibrateLC, TauRecToolBase, ITauToolBase )

    TauCalibrateLC(const std::string& name="TauCalibrateLC");
    ~TauCalibrateLC();

    virtual StatusCode initialize() override;
    virtual StatusCode execute(xAOD::TauJet& tau) const override;


  private:
    static const int s_nProngBins = 2;

    std::vector<std::vector<std::unique_ptr<TF1>>> m_calibFunc;
    std::vector<std::unique_ptr<TH1>> m_slopeNPVHist; 
    std::unique_ptr<TH1> m_etaBinHist = {};

    int    m_nEtaBins=0;
    double m_averageNPV=0;

    Gaudi::Property<std::string> m_calibrationFile{this, "calibrationFile", ""};
    Gaudi::Property<bool> m_doVertexCorrection{this, "VertexCorrection", true};

    SG::ReadDecorHandleKey<xAOD::EventInfo> m_aveIntPerXKey {this, 
        "averageInteractionsPerCrossingKey", 
        "EventInfo.averageInteractionsPerCrossing",
        "Decoration for Average Interaction Per Crossing"};
  
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexInputContainer {this,
        "Key_vertexInputContainer",
        "PrimaryVertices",
        "input vertex container key"};
};

#endif // TAURECTOOLS_TAUCALIBRATELC_H
