/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PunchThroughG4Classifier.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef G4ATLASTOOLS_PUNCHTHROUGHG4CLASSIFIER_H
#define G4ATLASTOOLS_PUNCHTHROUGHG4CLASSIFIER_H

//
#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPunchThroughG4Classifier.h"

#include <map>
#include <memory> //for unique_ptr

//LWTNN
#include "lwtnn/LightweightGraph.hh"

class PunchThroughG4Classifier : virtual public extends<AthAlgTool, IPunchThroughG4Classifier>{
public:

    /** Constructor */
    PunchThroughG4Classifier(const std::string&,const std::string&,const IInterface*);

    /** Destructor */
    virtual ~PunchThroughG4Classifier() = default;

    /** AlgTool initialize method */
    virtual StatusCode initialize() override;
    /** AlgTool finalize method */
    virtual StatusCode finalize() override;

    /** input variable MinMaxScaler initialize method */
    StatusCode initializeScaler(const std::string & scalerConfigFile);

    /** neural network initialize method */
    StatusCode initializeNetwork(const std::string & networkConfigFile);

    /** isotonic regressor calibrator initialize method */
    StatusCode initializeCalibrator(const std::string & calibratorConfigFile);

    /** interface method to return probability prediction of punch through */
    virtual double computePunchThroughProbability(const G4FastTrack& fastTrack, const double simE, std::vector<double> simEfrac) const override;

    /** calcalate NN inputs based on G4FastTrack and simulstate */
    static std::map<std::string, std::map<std::string, double> > computeInputs(const G4FastTrack& fastTrack, const double simE, std::vector<double> simEfrac);

    /** scale NN inputs using MinMaxScaler */
    std::map<std::string, std::map<std::string, double> > scaleInputs(std::map<std::string, std::map<std::string, double> >& inputs) const;

    /** calibrate NN output using isotonic regressor */
    double calibrateOutput(double& networkOutput) const;

private:
    /** NN graph */
    std::unique_ptr<lwt::LightweightGraph> m_graph{};

    /** input variable MinMaxScaler members */
    double m_scalerMin{};
    double m_scalerMax{};
    std::map<std::string, double> m_scalerMinMap;
    std::map<std::string, double> m_scalerMaxMap;

    /** isotonic regressor calibrator members */
    std::string m_calibratorConfigFile;
    double m_calibrationMin{};
    double m_calibrationMax{};
    std::map<double, double> m_calibrationMap;

    /*---------------------------------------------------------------------
     *  Properties
     *---------------------------------------------------------------------*/
    StringProperty m_scalerConfigFileName{this,     "ScalerConfigFileName",     "", ""};
    StringProperty m_networkConfigFileName{this,    "NetworkConfigFileName",    "", ""};
    StringProperty m_calibratorConfigFileName{this, "CalibratorConfigFileName", "", ""};

}; // class PunchThroughG4Classifier

#endif //G4ATLASTOOLS_PUNCHTHROUGHG4CLASSIFIER_H
