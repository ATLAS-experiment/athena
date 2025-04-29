// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "BoostedJetTaggers/JSSMLTool.h"

// Framework include(s).
#include "PathResolver/PathResolver.h"

// ROOT includes
#include "TSystem.h"
#include "TH2D.h"

namespace AthONNX {

  //*******************************************************************
  // for reading jet images
  std::vector<float> JSSMLTool::ReadJetImagePixels( std::vector<TH2D> Images ) const //function to load test images
  {

    int n_rows = m_nPixelsX;
    int n_cols = m_nPixelsY;
    int n_colors = m_nPixelsZ;

    std::vector<float> input_tensor_values(n_rows*n_cols*n_colors);

    for(int iRow=0; iRow<n_rows; ++iRow){
      for(int iColumn=0; iColumn<n_cols; ++iColumn){
        for(int iColor=0; iColor<n_colors; ++iColor){
          input_tensor_values[ (n_colors*n_cols*iRow) + iColumn*n_colors + iColor] = Images[iColor].GetBinContent(iRow+1, iColumn+1);
        }
      }
    }

    return input_tensor_values;
  }

  //********************************************************************************
  // for reading DNN inputs
  std::vector<float> JSSMLTool::ReadJSSInputs(std::map<std::string, double> JSSVars) const //function to load test images
  {

    std::vector<float> input_tensor_values(m_nvars);

    // apply features scaling
    for(const auto & var : JSSVars){
      double mean = m_scaler.find(var.first)->second[0];
      double std  = m_scaler.find(var.first)->second[1];
      JSSVars[var.first] = (var.second - mean) / std;
    }

    // then dump it to a vector
    for(int v=0; v<m_nvars; ++v){
      std::string name = m_JSSInputMap.find(v)->second;
      input_tensor_values[v] = JSSVars[name];
    }

    return input_tensor_values;
  }

  //********************************************************************************
  // for reading jet labels for DNN
  // this can be extended in case of multi-class models
  std::vector<int> JSSMLTool::ReadOutputLabels() const
  {
    std::vector<int> output_tensor_values(1);

    output_tensor_values[0] = 1;

    return output_tensor_values;
  }

  // constructor --- 
  JSSMLTool::JSSMLTool(const std::string& name):
    AsgTool(name)
  {
    declareProperty("ModelPath", m_modelFileName);
    declareProperty("nPixelsX", m_nPixelsX);
    declareProperty("nPixelsY", m_nPixelsY);
    declareProperty("nPixelsZ", m_nPixelsZ);
  }

  // initialize ---
  StatusCode JSSMLTool::initialize( ) {

    // Access the service.    
    // Find the model file.    
    ATH_MSG_INFO( "Using model file: " << m_modelFileName );
    
    // Set up the ONNX Runtime session.
    Ort::SessionOptions sessionOptions;
    sessionOptions.SetIntraOpNumThreads( 1 );
    sessionOptions.SetGraphOptimizationLevel( ORT_ENABLE_BASIC );
    Ort::AllocatorWithDefaultOptions allocator;
    m_env = std::make_unique< Ort::Env >(ORT_LOGGING_LEVEL_WARNING, "");
    m_session = std::make_unique< Ort::Session >( *m_env,
						  m_modelFileName.c_str(),
						  sessionOptions );
    
    ATH_MSG_INFO( "Created the ONNX Runtime session" );
    
    m_num_input_nodes = m_session->GetInputCount();
    m_input_node_names.resize(m_num_input_nodes);
    
    for( std::size_t i = 0; i < m_num_input_nodes; i++ ) {
      // print input node names
      char* input_name = m_session->GetInputNameAllocated(i, allocator).release();
      ATH_MSG_DEBUG("Input "<<i<<" : "<<" name = "<<input_name);
      m_input_node_names[i] = input_name;
      // print input node types
      Ort::TypeInfo type_info = m_session->GetInputTypeInfo(i);
      auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
      ONNXTensorElementDataType type = tensor_info.GetElementType();
      ATH_MSG_DEBUG("Input "<<i<<" : "<<" type = "<<type);
      
      // print input shapes/dims
      m_input_node_dims = tensor_info.GetShape();
      ATH_MSG_DEBUG("Input "<<i<<" : num_dims = "<<m_input_node_dims.size());
      for (std::size_t j = 0; j < m_input_node_dims.size(); j++){
        if(m_input_node_dims[j]<0)
          m_input_node_dims[j] =1;
        ATH_MSG_DEBUG("Input"<<i<<" : dim "<<j<<" = "<<m_input_node_dims[j]);
      }  
    }
    
    m_num_output_nodes = m_session->GetOutputCount();
    m_output_node_names.resize(m_num_output_nodes);
    
    for( std::size_t i = 0; i < m_num_output_nodes; i++ ) {
      // print output node names
      char* output_name = m_session->GetOutputNameAllocated(i, allocator).release();
      ATH_MSG_DEBUG("Output "<<i<<" : "<<" name = "<<output_name);
      m_output_node_names[i] = output_name;
      
      Ort::TypeInfo type_info = m_session->GetOutputTypeInfo(i);
      auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
      ONNXTensorElementDataType type = tensor_info.GetElementType();
      ATH_MSG_DEBUG("Output "<<i<<" : "<<" type = "<<type);
      
      // print output shapes/dims
      m_output_node_dims = tensor_info.GetShape();
      ATH_MSG_INFO("Output "<<i<<" : num_dims = "<<m_output_node_dims.size());
      for (std::size_t j = 0; j < m_output_node_dims.size(); j++){
        if(m_output_node_dims[j]<0)
          m_output_node_dims[j] =1;
        ATH_MSG_INFO("Output"<<i<<" : dim "<<j<<" = "<<m_output_node_dims[j]);
      }  
    }

    // Return gracefully.
    return StatusCode::SUCCESS;
  } // end initialize ---

  // constituents image based
  double JSSMLTool::retrieveConstituentsScore(std::vector<TH2D> Images) const {
        
    //*************************************************************************
    // Score the model using sample data, and inspect values

    // preparing container to hold input data
    size_t input_tensor_size = m_nPixelsX*m_nPixelsY*m_nPixelsZ;
    std::vector<float> input_tensor_values(input_tensor_size);

    // loading input data
    input_tensor_values = ReadJetImagePixels(Images);
    
    // preparing container to hold output data
    int testSample = 0;    
    std::vector<int> output_tensor_values_ = ReadOutputLabels();
    int output_tensor_values = output_tensor_values_[testSample]; 

    // create input tensor object from data values
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_tensor_values.data(), input_tensor_size, m_input_node_dims.data(), m_input_node_dims.size());
    assert(input_tensor.IsTensor());
        
    auto output_tensors = m_session->Run(Ort::RunOptions{nullptr}, m_input_node_names.data(), &input_tensor, m_input_node_names.size(), m_output_node_names.data(), m_output_node_names.size());
    assert(output_tensors.size() == 1 && output_tensors.front().IsTensor());
    
    // Get pointer to output tensor float values
    float* floatarr = output_tensors.front().GetTensorMutableData<float>();
    int arrSize = sizeof(*floatarr)/sizeof(floatarr[0]);

    // show  true label for the test input
    ATH_MSG_DEBUG("Label for the input test data  = "<<output_tensor_values);
    float ConstScore = -999;
    int max_index = 0;
    for (int i = 0; i < arrSize; i++){
      ATH_MSG_VERBOSE("Score for class "<<i<<" = "<<floatarr[i]<<std::endl);
      if (ConstScore<floatarr[i]){
        ConstScore = floatarr[i];
        max_index = i;
      }
    }
    ATH_MSG_DEBUG("Class: "<<max_index<<" has the highest score: "<<floatarr[max_index]);
    
    return ConstScore;

  } // end retrieve CNN score ----

  // constituents transformer based
  double JSSMLTool::retrieveConstituentsScore(std::vector<std::vector<float>> constituents) const {
        
    // the format of the packed constituents is:
    // constituents.size() ---> 4, for example, (m pT, eta, phi)
    // constituents.at(0) ---> number of constituents
    // the packing can be done for any kind of low level inputs
    // i.e. PFO/UFO constituents, topo-towers, tracks, etc
    // they can be concatened one after the other in case of multiple inputs

    //*************************************************************************
    // Score the model using sample data, and inspect values
    // loading input data

    std::vector<int> output_tensor_values_ = ReadOutputLabels();
    
    int testSample = 0;    
    
    //preparing container to hold output data
    int output_tensor_values = output_tensor_values_[testSample]; 

    // prepare the inputs
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<Ort::Value> input_tensors;
    for (long unsigned int i=0; i<constituents.size(); i++) {

      // test
      std::vector<int64_t> const_dim = {1, static_cast<int64_t>(constituents.at(i).size())};

      input_tensors.push_back(Ort::Value::CreateTensor<float>(
        memory_info, 
        constituents.at(i).data(), constituents.at(i).size(), const_dim.data(), const_dim.size()
        )
      );
    }

    auto output_tensors = m_session->Run(Ort::RunOptions{nullptr}, m_input_node_names.data(), input_tensors.data(), m_input_node_names.size(), m_output_node_names.data(), m_output_node_names.size());
    assert(output_tensors.size() == 1 && output_tensors.front().IsTensor());
    
    // Get pointer to output tensor float values
    float* floatarr = output_tensors.front().GetTensorMutableData<float>();
    int arrSize = sizeof(*floatarr)/sizeof(floatarr[0]);

    // show  true label for the test input
    ATH_MSG_DEBUG("Label for the input test data  = "<<output_tensor_values);
    float ConstScore = -999;
    int max_index = 0;
    for (int i = 0; i < arrSize; i++){
      ATH_MSG_VERBOSE("Score for class "<<i<<" = "<<floatarr[i]<<std::endl);
      ATH_MSG_VERBOSE(" +++ Score for class "<<i<<" = "<<floatarr[i]<<std::endl);
      if (ConstScore<floatarr[i]){
        ConstScore = floatarr[i];
        max_index = i;
      }
    }
    ATH_MSG_DEBUG("Class: "<<max_index<<" has the highest score: "<<floatarr[max_index]);
    
    return ConstScore;

  } // end retrieve constituents score ----

  // constituents transformer based with const/inter variables
  double JSSMLTool::retrieveConstituentsScore(std::vector<std::vector<float>> constituents, std::vector<std::vector<std::vector<float>>> interactions) const {
        
    // the format of the constituents/interaction variables is:
    // constituents       ---> (nConstituents + nTowers, 7)
    // interactions       ---> (i, j, 4), with i, j in {nConstituents + nTowers}
    // the packing can be done for any kind of low level inputs
    // i.e. PFO/UFO constituents, topo-towers, tracks, etc
    // they can be concatened one after the other in case of multiple inputs

    //*************************************************************************
    // Score the model using sample data, and inspect values
    // loading input data

    std::vector<int> output_tensor_values_ = ReadOutputLabels();
    
    int testSample = 0;    
    
    //preparing container to hold output data
    int output_tensor_values = output_tensor_values_[testSample]; 

    // prepare the inputs
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<Ort::Value> input_tensors;

    // unroll the inputs
    std::vector<float> constituents_values; //(constituents.size()*7);
    for (long unsigned int i=0; i<constituents.size(); i++) {
      for (long unsigned int j=0; j<7; j++) {
        constituents_values.push_back(constituents.at(i).at(j));
      }
    }

    std::vector<float> interactions_values; //(interactions.size()*interactions.size()*4);
    for (long unsigned int i=0; i<interactions.size(); i++) {
      for (long unsigned int k=0; k<interactions.size(); k++) {
        for (long unsigned int j=0; j<4; j++) {
          interactions_values.push_back(interactions.at(i).at(k).at(j));
        }
      }
    }

    std::vector<int64_t> const_dim = {1, static_cast<int64_t>(constituents.size()), 7};
    input_tensors.push_back(Ort::Value::CreateTensor<float>(
      memory_info, 
      constituents_values.data(), constituents_values.size(), const_dim.data(), const_dim.size()
      )
    );

    std::vector<int64_t> inter_dim = {1, static_cast<int64_t>(constituents.size()), static_cast<int64_t>(constituents.size()), 4};
    input_tensors.push_back(Ort::Value::CreateTensor<float>(
      memory_info, 
      interactions_values.data(), interactions_values.size(), inter_dim.data(), inter_dim.size()
      )
    );

    auto output_tensors = m_session->Run(Ort::RunOptions{nullptr}, m_input_node_names.data(), input_tensors.data(), m_input_node_names.size(), m_output_node_names.data(), m_output_node_names.size());
    assert(output_tensors.size() == 1 && output_tensors.front().IsTensor());
    
    // Get pointer to output tensor float values
    float* floatarr = output_tensors.front().GetTensorMutableData<float>();
    int arrSize = sizeof(*floatarr)/sizeof(floatarr[0]);

    // show  true label for the test input
    ATH_MSG_DEBUG("Label for the input test data  = "<<output_tensor_values);
    float ConstScore = -999;
    int max_index = 0;
    for (int i = 0; i < arrSize; i++){
      ATH_MSG_VERBOSE("Score for class "<<i<<" = "<<floatarr[i]<<std::endl);
      ATH_MSG_VERBOSE(" +++ Score for class "<<i<<" = "<<floatarr[i]<<std::endl);
      if (ConstScore<floatarr[i]){
        ConstScore = floatarr[i];
        max_index = i;
      }
    }
    ATH_MSG_DEBUG("Class: "<<max_index<<" has the highest score: "<<floatarr[max_index]);
    
    return ConstScore;

  } // end retrieve constituents score ----


  // dedicated DisCo/DNN method ---
  double JSSMLTool::retrieveHighLevelScore(std::map<std::string, double> JSSVars) const {
        
    //*************************************************************************
    // Score the model using sample data, and inspect values
    
    //preparing container to hold input data
    size_t input_tensor_size = m_nvars;
    std::vector<float> input_tensor_values(m_nvars);

    // loading input data
    input_tensor_values = ReadJSSInputs(JSSVars);
    
    // preparing container to hold output data
    int testSample = 0; 
    std::vector<int> output_tensor_values_ = ReadOutputLabels();
    int output_tensor_values = output_tensor_values_[testSample]; 
    
    // create input tensor object from data values
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    
    // we need a multiple tensor input structure for DisCo model
    Ort::Value input1 = Ort::Value::CreateTensor<float>(memory_info, const_cast<float*>(input_tensor_values.data()), input_tensor_size, m_input_node_dims.data(), m_input_node_dims.size());
    std::vector<float> empty = {1.};
    Ort::Value input2 = Ort::Value::CreateTensor<float>(memory_info, empty.data(), 1, m_input_node_dims.data(), m_input_node_dims.size());
    Ort::Value input3 = Ort::Value::CreateTensor<float>(memory_info, empty.data(), 1, m_input_node_dims.data(), m_input_node_dims.size());
    Ort::Value input4 = Ort::Value::CreateTensor<float>(memory_info, empty.data(), 1, m_input_node_dims.data(), m_input_node_dims.size());
    std::vector<Ort::Value> input_tensor;
    std::vector<int64_t> aaa = {1, m_nvars};
    input_tensor.emplace_back(
      Ort::Value::CreateTensor<float>(memory_info, input_tensor_values.data(), input_tensor_size, aaa.data(), aaa.size())
    );
    input_tensor.emplace_back(
      Ort::Value::CreateTensor<float>(memory_info, input_tensor_values.data(), input_tensor_size, m_input_node_dims.data(), m_input_node_dims.size())
    );
    input_tensor.emplace_back(
      Ort::Value::CreateTensor<float>(memory_info, input_tensor_values.data(), input_tensor_size, m_input_node_dims.data(), m_input_node_dims.size())
    );
    input_tensor.emplace_back(
      Ort::Value::CreateTensor<float>(memory_info, input_tensor_values.data(), input_tensor_size, m_input_node_dims.data(), m_input_node_dims.size())
    );
        
    auto output_tensors = m_session->Run(Ort::RunOptions{nullptr}, m_input_node_names.data(), input_tensor.data(), m_input_node_names.size(), m_output_node_names.data(), m_output_node_names.size());
    assert(output_tensors.size() == 1 && output_tensors.front().IsTensor());
    
    // Get pointer to output tensor float values
    float* floatarr = output_tensors.front().GetTensorMutableData<float>();
    int arrSize = sizeof(*floatarr)/sizeof(floatarr[0]);

    // show  true label for the test input
    ATH_MSG_DEBUG("Label for the input test data  = "<<output_tensor_values);
    float HLScore = -999;
    int max_index = 0;
    for (int i = 0; i < arrSize; i++){
      ATH_MSG_VERBOSE("Score for class "<<i<<" = "<<floatarr[i]<<std::endl);
      if (HLScore<floatarr[i]){
	      HLScore = floatarr[i];
	      max_index = i;
      }
    }
    ATH_MSG_DEBUG("Class: "<<max_index<<" has the highest score: "<<floatarr[max_index]);
    
    return HLScore;

  } // end retrieve HighLevel score ----
    
  // extra methods
  StatusCode JSSMLTool::SetScaler(std::map<std::string, std::vector<double>> scaler){
    m_scaler = scaler;

    // ToDo:
    // this will have an overriding config as property
    m_JSSInputMap = {
      {0,"pT"}, {1,"CNN"}, {2,"D2"}, {3,"nTracks"}, {4,"ZCut12"},
      {5,"Tau1_wta"}, {6,"Tau2_wta"}, {7,"Tau3_wta"},
      {8,"KtDR"}, {9,"Split12"}, {10,"Split23"},
      {11,"ECF1"}, {12,"ECF2"}, {13,"ECF3"},
      {14,"Angularity"}, {15,"FoxWolfram0"}, {16,"FoxWolfram2"},
      {17,"Aplanarity"}, {18,"PlanarFlow"}, {19,"Qw"},
    };
    m_nvars = m_JSSInputMap.size();

    return StatusCode::SUCCESS;
  }

} // namespace AthONNX
