/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
  @class ElectronDNNCalculator
  @brief Used by AsgElectronSelectorTool to calculate the score of a python
         trained DNN using lwtnn as interface to do electron ID. Also applies a
         transformation to the input variables based on a QuantileTransformer.

  @author Lukas Ehrke
*/

#include "ElectronDNNCalculator.h"
#include "PathResolver/PathResolver.h"
#include "TSystem.h"
#include "TFile.h"
#include <map>
#include <fstream>
#include <iostream>
#include "lwtnn/parse_json.hh"
#include "lwtnn/InputOrder.hh"
#include <Eigen/Dense>


ElectronDNNCalculator::ElectronDNNCalculator(AsgElectronSelectorTool* owner,
                                             const std::string& modelFileName,
                                             const std::string& quantileFileName,
                                             const std::vector<std::string>& variables,
                                             const bool multiClass) :
                                            asg::AsgMessagingForward(owner),
                                            m_quantiles(variables.size()),
                                            m_multiClass(multiClass),
                                            m_variables(variables),
                                            m_var_size(variables.size())
{
  ATH_MSG_INFO("Initializing ElectronDNNCalculator...");

  if (modelFileName.empty()){
    throw std::runtime_error("No file found at '" + modelFileName + "'");
  }

  // Make an input file object
  std::ifstream inputFile;

  // Open your trained model
  ATH_MSG_INFO("Loading model: " << modelFileName.c_str());

  // Create input order for the NN, the data needs to be passed in this exact order
  lwt::InputOrder order;
  order.scalar.emplace_back("node_0", m_variables );

  // create the model
  inputFile.open(modelFileName);
  auto parsedGraph = lwt::parse_json_graph(inputFile);
  // Test whether the number of outputs of the given network corresponds to the expected number
  size_t nOutputs = parsedGraph.outputs.begin()->second.labels.size();
  if (nOutputs != 6 && nOutputs != 1){
    throw std::runtime_error("Given model does not have 1 or 6 outputs. Something seems to be wrong with the model file.");
  }
  else if (nOutputs == 1 && m_multiClass){
    throw std::runtime_error("Given model has 1 output but config file specifies mutliclass. Something is wrong");
  }
  else if (nOutputs == 6 && !m_multiClass){
    throw std::runtime_error("Given model has 6 output but config file does not specify mutliclass. Something is wrong");
  }

  m_graph = std::make_unique<lwt::generic::FastGraph<float>>(parsedGraph, order);

  if (quantileFileName.empty()){
    throw std::runtime_error("No file found at '" + quantileFileName + "'");
  }

  // Open quantiletransformer file
  ATH_MSG_INFO("Loading QuantileTransformer " << quantileFileName);
  std::unique_ptr<TFile> qtfile(TFile::Open(quantileFileName.data()));
  if (readQuantileTransformer((TTree*)qtfile->Get("tree")) == 0){
    throw std::runtime_error("Could not load all variables for the QuantileTransformer");

  }
}


// takes the input variables, transforms them according to the given QuantileTransformer and predicts the DNN value(s)
Eigen::Matrix<float, -1, 1> ElectronDNNCalculator::calculate( const std::vector<double>& variableValues ) const
{

  if(variableValues.size() != m_var_size)
    throw std::runtime_error("Passed vector of variables has wrong size");

  // Create the input for the model
  Eigen::VectorXf inputVector(m_variables.size());

  // This has to be in the same order as the InputOrder was defined
  for(uint i = 0; i < m_var_size; ++i){
    inputVector(i) = transformInput(m_quantiles.at(i), variableValues.at(i));
  }

  std::vector<Eigen::VectorXf> inp;
  inp.emplace_back(std::move(inputVector));

  auto output = m_graph->compute(inp);
  return output;
}


// transform the input based on a QuantileTransformer. quantiles are bins in the variable, while references are bins from 0 to 1
// The interpolation is done averaging the interpolation going from small to large bins and going from large to small bins
// to deal with non strictly monotonic rising bins.
double ElectronDNNCalculator::transformInput( const std::vector<double>& quantiles, double value ) const
{
  int size = quantiles.size();

  // if given value is outside of range of the given quantiles return min (0) or max (1) of references
  if (value <= quantiles[0]) return m_references[0];
  if (value >= quantiles[size-1]) return m_references[size-1];

  // find the bin where the value is smaller than the next bin (going from low bins to large bins)
  auto lowBound = std::lower_bound(quantiles.begin(), quantiles.end(), value);
  int lowBin = lowBound - quantiles.begin() - 1;

  // get x and y values on left and right side from value while going up
  double xLup = quantiles[lowBin], yLup = m_references[lowBin], xRup = quantiles[lowBin+1], yRup = m_references[lowBin+1];

  // find the bin where the value is larger than the next bin (going from large bins to low bins)
  auto upperBound = std::upper_bound(quantiles.begin(), quantiles.end(), value);
  int upperBin = upperBound - quantiles.begin();

  // get x and y values on left and right side from value while going down
  double xRdown = quantiles[upperBin], yRdown = m_references[upperBin], xLdown = quantiles[upperBin-1], yLdown = m_references[upperBin-1];

  // calculate the gradients
  double dydxup = ( yRup - yLup ) / ( xRup - xLup );
  double dydxdown = ( yRdown - yLdown ) / ( xRdown - xLdown );

  // average linear interpolation of up and down case
  return 0.5 * ((yLup + dydxup * (value - xLup)) + (yLdown + dydxdown * (value - xLdown)));
}


// Read the information needed for the QuantileTransformer from a ROOT TTree
int ElectronDNNCalculator::readQuantileTransformer( TTree* tree )
{
  int sc(1);
  // the reference bins to which the variables will be transformed to
  double references;
  sc = tree->SetBranchAddress("references", &references) == -5 ? 0 : 1;

  std::map<std::string, double> readVars;
  for ( const auto& var : m_variables ){
    sc = tree->SetBranchAddress(TString(var), &readVars[var]) == -5 ? 0 : 1;
  }

  for (int ientry = 0; ientry < tree->GetEntries(); ientry++){
    tree->GetEntry(ientry);
    m_references.push_back(references);
    for(uint ivar = 0; ivar < m_var_size; ++ivar){
      m_quantiles.at(ivar).push_back(readVars[m_variables.at(ivar)]);
    }
  }
  return sc;
}

