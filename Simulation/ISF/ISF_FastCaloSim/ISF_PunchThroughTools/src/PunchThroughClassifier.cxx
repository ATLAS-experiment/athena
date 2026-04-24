/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PunchThroughClassifier.h"

#include <fstream>
#include <memory>

// PathResolver
#include "PathResolver/PathResolver.h"

//LWTNN
#include "lwtnn/parse_json.hh"

#include "XMLCoreParser/XMLCoreParser.h"
#include "XMLCoreParser/XMLCoreNode.h"


ISF::PunchThroughClassifier::PunchThroughClassifier(const std::string& type, const std::string& name, const IInterface*  parent)
    : base_class(type, name, parent) {

    declareProperty( "ScalerConfigFileName",          m_scalerConfigFileName   );
    declareProperty( "NetworkConfigFileName",          m_networkConfigFileName   );
    declareProperty( "CalibratorConfigFileName",          m_calibratorConfigFileName   );
}

double ISF::PunchThroughClassifier::computePunchThroughProbability(const ISF::ISFParticle &isfp, const TFCSSimulationState& simulstate) const {

    std::map<std::string, std::map<std::string, double> > networkInputs = computeInputs(isfp, simulstate); //compute inputs

    networkInputs = scaleInputs(networkInputs); //scale inputs

    std::map<std::string, double> networkOutputs = m_graph->compute(networkInputs); //call neural network on inputs

    double calibratedOutput = calibrateOutput(networkOutputs["out_0"]); //calibrate neural network output

    return calibratedOutput;
}


StatusCode ISF::PunchThroughClassifier::initialize(){

    ATH_MSG_VERBOSE( "[ punchthroughclassifier ] initialize()" );

    std::string resolvedScalerFileName = PathResolverFindCalibFile (m_scalerConfigFileName);
    if ( initializeScaler(resolvedScalerFileName) != StatusCode::SUCCESS)
    {
        ATH_MSG_ERROR("[ punchthroughclassifier ] unable to load punchthroughclassifier input scaler");
    }

    std::string resolvedNetworkFileName = PathResolverFindCalibFile (m_networkConfigFileName);
    if ( initializeNetwork(resolvedNetworkFileName) != StatusCode::SUCCESS)
    {
        ATH_MSG_ERROR("[ punchthroughclassifier ] unable to load punchthroughclassifier network");
    }

    std::string resolvedCalibratorFileName = PathResolverFindCalibFile (m_calibratorConfigFileName);
    if ( initializeCalibrator(resolvedCalibratorFileName) != StatusCode::SUCCESS)
    {
        ATH_MSG_ERROR("[ punchthroughclassifier ] unable to load punchthroughclassifier calibrator");
    }

    return StatusCode::SUCCESS;
}

StatusCode ISF::PunchThroughClassifier::initializeScaler(const std::string & scalerConfigFile){

    //parse xml that contains config for MinMaxScaler for each of the network inputs
    XMLCoreParser p;
    std::unique_ptr<XMLCoreNode> doc = p.parse (scalerConfigFile);

    ATH_MSG_DEBUG( "[ punchthroughclassifier ] Loading scaler: " << scalerConfigFile);

    for (const XMLCoreNode* node : doc->get_children ("Transformations/*"))
    {
      if (node->get_name() == "ScalerValues") {
        m_scalerMin = node->get_double_attrib ("min");
        m_scalerMax = node->get_double_attrib ("max");
      }
      else if (node->get_name() == "VarScales") {
        std::string name = node->get_attrib ("name");
        m_scalerMinMap[name] = node->get_double_attrib ("min");
        m_scalerMaxMap[name] = node->get_double_attrib ("max");
      }
    }

    return StatusCode::SUCCESS;
}

StatusCode ISF::PunchThroughClassifier::initializeNetwork(const std::string & networkConfigFile){

    ATH_MSG_INFO( "[ punchthroughclassifier ] Loading classifier: " << networkConfigFile);

    std::ifstream input(networkConfigFile);
    if(!input){
        ATH_MSG_ERROR("Could not find json file " << networkConfigFile );
        return StatusCode::FAILURE;
    }

    m_graph  = std::make_unique<lwt::LightweightGraph>(lwt::parse_json_graph(input));
    if(!m_graph){
        ATH_MSG_ERROR("Could not parse graph json file " << networkConfigFile );
        return StatusCode::FAILURE;
    }


    return StatusCode::SUCCESS;
}


StatusCode ISF::PunchThroughClassifier::initializeCalibrator(const std::string & calibratorConfigFile){

    XMLCoreParser p;
    std::unique_ptr<XMLCoreNode> doc = p.parse (calibratorConfigFile);

    //parse xml that contains config for isotonic regressor used to calibrate the network output
    ATH_MSG_DEBUG( "[ punchthroughclassifier ] Loading calibrator: " << calibratorConfigFile);

    for (const XMLCoreNode* node : doc->get_children ("Transformations/*"))
    {
      if (node->get_name() == "LimitValues") {
        m_calibrationMin = node->get_double_attrib ("min");
        m_calibrationMax = node->get_double_attrib ("max");
      }
      else if (node->get_name() == "LinearNorm") {
        double orig = node->get_double_attrib ("orig");
        double norm = node->get_double_attrib ("norm");
        m_calibrationMap[orig] = norm;
      }
    }

    return StatusCode::SUCCESS;
}

std::map<std::string, std::map<std::string, double> > ISF::PunchThroughClassifier::computeInputs(const ISF::ISFParticle &isfp, const TFCSSimulationState& simulstate) {

    //calculate inputs for NN

    std::map<std::string, std::map<std::string, double> > networkInputs;

    //add initial particle and total energy variables
    networkInputs["node_0"] = {
        {"variable_0", isfp.momentum().mag() },
        {"variable_1", std::abs(isfp.position().eta()) },
        {"variable_2", isfp.position().phi() },
        {"variable_3", simulstate.E()},
    };

    //add energy fraction variables
    for (unsigned int i = 0; i < 24; i++){
        networkInputs["node_0"].insert({"variable_" + std::to_string(i + 4), simulstate.Efrac(i)});
    }

    return networkInputs;
}

std::map<std::string, std::map<std::string, double> > ISF::PunchThroughClassifier::scaleInputs(std::map<std::string, std::map<std::string, double> >& inputs) const{

    //apply MinMaxScaler to network inputs

    for (auto& var : inputs["node_0"]) {

        double x_std;
        if(m_scalerMaxMap.at(var.first) != m_scalerMinMap.at(var.first)){
            x_std = (var.second - m_scalerMinMap.at(var.first)) / (m_scalerMaxMap.at(var.first) - m_scalerMinMap.at(var.first));
        }
        else{
            x_std = (var.second - m_scalerMinMap.at(var.first));
        }
        var.second = x_std * (m_scalerMax - m_scalerMin) + m_scalerMin;
    }

    return inputs;
}

double ISF::PunchThroughClassifier::calibrateOutput(double& networkOutput) const {

    //calibrate output of network using isotonic regressor model

    //if network output is outside of the range of isotonic regressor then return min and max values
    if (networkOutput < m_calibrationMin){
        return m_calibrationMin;
    }
    else if (networkOutput > m_calibrationMax){
        return m_calibrationMax;
    }

    //otherwise find neighbouring points in isotonic regressor
    auto upper = m_calibrationMap.upper_bound(networkOutput);
    auto lower = upper--;

    //Perform linear interpolation between points
    double m = (upper->second - lower->second)/(upper->first - lower->first);
    double c = lower->second - m * lower->first;
    double calibrated = m * networkOutput + c;

    return calibrated;
}
