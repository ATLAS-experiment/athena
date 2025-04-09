/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "PhotonVertexSelection/PhotonVertexSelectionTool.h"
#include "PhotonVertexSelection/PhotonVertexHelpers.h"

// EDM includes
#include "xAODTracking/VertexContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEgamma/EgammaDefs.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

// Framework includes
#include "PathResolver/PathResolver.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"

// ROOT includes
#include "TMVA/Reader.h"

// std includes
#include <algorithm>

namespace CP {

  // helper function to get the vertex of a track
  const xAOD::Vertex* getVertexFromTrack(const xAOD::TrackParticle* track,
                                         const xAOD::VertexContainer* vertices)
  {
    const xAOD::Vertex* vtxWithLargestWeight = nullptr;
    float largestWeight = 0;

    for (const auto *vtx : *vertices) {
      //Search for vertex linked to this track
      const auto& trkLinks=vtx->trackParticleLinks();
      const size_t nTrackLinks=trkLinks.size();
      for (unsigned i=0;i<nTrackLinks;++i) {
        if (trkLinks[i].isValid() && *(trkLinks[i]) == track) {//ptr comparison
          if( vtx->trackWeights()[i] > largestWeight ){
            vtxWithLargestWeight = vtx;
            largestWeight = vtx->trackWeights()[i];
          }
        }
      }
    }

    return vtxWithLargestWeight;
  }

  //____________________________________________________________________________
  PhotonVertexSelectionTool::PhotonVertexSelectionTool(const std::string &name)
  : asg::AsgTool(name)
  {
    // run 2 NN model:
    //  m_doSkipByZSigma = true, m_isTMVA = true
    // run 3 NN model:
    //  m_doSkipByZSigma = false, m_isTMVA = false

    // default variables
    declareProperty("nVars",  m_nVars = 4);
    declareProperty("conversionPtCut", m_convPtCut = 2e3);
    declareProperty("DoSkipByZSigma",  m_doSkipByZSigma = false);

    declareProperty("derivationPrefix", m_derivationPrefix = "");

    // boolean for TMVA, default true
    declareProperty("isTMVA", m_isTMVA = false);

    // config files (TMVA), default paths if not set
    declareProperty("ConfigFileCase1",
        m_TMVAModelFilePath1 = "PhotonVertexSelection/v1/DiphotonVertex_case1.weights.xml");
    declareProperty("ConfigFileCase2",
        m_TMVAModelFilePath2 = "PhotonVertexSelection/v1/DiphotonVertex_case2.weights.xml");

    // config files (ONNX), default paths if not set
    declareProperty("ONNXModelFileCase1", m_ONNXModelFilePath1 = "PhotonVertexSelection/run3nn/model1.onnx");
    declareProperty("ONNXModelFileCase2", m_ONNXModelFilePath2 = "PhotonVertexSelection/run3nn/model2.onnx");
  }

  //____________________________________________________________________________
  PhotonVertexSelectionTool::~PhotonVertexSelectionTool()
  = default;

  //____________________________________________________________________________
  //new additions for ONNX
  float PhotonVertexSelectionTool::getScore(int nVars, const std::vector<std::vector<float>>& input_data, const std::shared_ptr<Ort::Session> sessionHandle, std::vector<int64_t> input_node_dims, std::vector<const char*> input_node_names, std::vector<const char*> output_node_names) const{
     //*************************************************************************
     // score the model using sample data, and inspect values
     // loading input data
     std::vector<std::vector<float>> input_tensor_values_ = input_data;

     //preparing container to hold input data
     size_t input_tensor_size = nVars;
     std::vector<float> input_tensor_values(nVars);
     input_tensor_values = input_tensor_values_[0]; //0th element since only batch_size of 1, otherwise loop

     // create input tensor object from data values
     auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
     // create tensor using info from inputs
     Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_tensor_values.data(), input_tensor_size, input_node_dims.data(), input_node_dims.size());

     // check if input is of type tensor
     assert(input_tensor.IsTensor());

     // run the inference
     auto output_tensors = sessionHandle->Run(Ort::RunOptions{nullptr}, input_node_names.data(), &input_tensor, input_node_names.size(), output_node_names.data(), output_node_names.size());

     // check size of output tensor
     assert(output_tensors.size() == 1 && output_tensors.front().IsTensor());

     // get pointer to output tensor float values
     //float* floatarr = output_tensors.front().GetTensorMutableData<float>();
     float* floatarr = output_tensors[0].GetTensorMutableData<float>();

     int arrSize = sizeof(*floatarr)/sizeof(floatarr[0]);
     ATH_MSG_DEBUG("The size of the array is: " << arrSize);
     ATH_MSG_DEBUG("floatarr[0] = " << floatarr[0]);
     return floatarr[0];
  }

  //new additions for ONNX
  std::tuple<std::vector<int64_t>, std::vector<const char*>> PhotonVertexSelectionTool::getInputNodes(const std::shared_ptr<Ort::Session> sessionHandle, Ort::AllocatorWithDefaultOptions& allocator){
    // input nodes
    std::vector<int64_t> input_node_dims;
    size_t num_input_nodes = sessionHandle->GetInputCount();
    std::vector<const char*> input_node_names(num_input_nodes);

    // Loop the input nodes
    for( std::size_t i = 0; i < num_input_nodes; i++ ) {
      // Print input node names
      char* input_name = sessionHandle->GetInputNameAllocated(i, allocator).release();
      ATH_MSG_DEBUG("Input "<<i<<" : "<<" name= "<<input_name);
      input_node_names[i] = input_name;

      // Print input node types
      Ort::TypeInfo type_info = sessionHandle->GetInputTypeInfo(i);
      auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
      ONNXTensorElementDataType type = tensor_info.GetElementType();
      ATH_MSG_DEBUG("Input "<<i<<" : "<<" type= "<<type);

      // Print input shapes/dims
      input_node_dims = tensor_info.GetShape();
      ATH_MSG_DEBUG("Input "<<i<<" : num_dims= "<<input_node_dims.size());
      for (std::size_t j = 0; j < input_node_dims.size(); j++){
        if(input_node_dims[j]<0){input_node_dims[j] =1;}
        ATH_MSG_DEBUG("Input"<<i<<" : dim "<<j<<"= "<<input_node_dims[j]);
      }
    }
    return std::make_tuple(input_node_dims, input_node_names);
  }

  //new additions for ONNX
  std::tuple<std::vector<int64_t>, std::vector<const char*>> PhotonVertexSelectionTool::getOutputNodes(const std::shared_ptr<Ort::Session> sessionHandle, Ort::AllocatorWithDefaultOptions& allocator){
    // output nodes
    std::vector<int64_t> output_node_dims;
    size_t num_output_nodes = sessionHandle->GetOutputCount();
    std::vector<const char*> output_node_names(num_output_nodes);

    // Loop the output nodes
    for( std::size_t i = 0; i < num_output_nodes; i++ ) {
      // Print output node names
      char* output_name = sessionHandle->GetOutputNameAllocated(i, allocator).release();
      ATH_MSG_DEBUG("Output "<<i<<" : "<<" name= "<<output_name);
      output_node_names[i] = output_name;

      Ort::TypeInfo type_info = sessionHandle->GetOutputTypeInfo(i);
      auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
      ONNXTensorElementDataType type = tensor_info.GetElementType();
      ATH_MSG_DEBUG("Output "<<i<<" : "<<" type= "<<type);

      // Print output shapes/dims
      output_node_dims = tensor_info.GetShape();
      ATH_MSG_DEBUG("Output "<<i<<" : num_dims= "<<output_node_dims.size());
      for (std::size_t j = 0; j < output_node_dims.size(); j++){
        if(output_node_dims[j]<0){output_node_dims[j] =1;}
        ATH_MSG_DEBUG("Output"<<i<<" : dim "<<j<<"= "<<output_node_dims[j]);
      }
    }
    return std::make_tuple(output_node_dims, output_node_names);
  }

  //new additions for ONNX
  std::tuple<std::shared_ptr<Ort::Session>, Ort::AllocatorWithDefaultOptions> PhotonVertexSelectionTool::setONNXSession(Ort::Env& env, const std::string& modelFilePath){
    // Find the model file.
    const std::string modelFileName = PathResolverFindCalibFile( modelFilePath );
    ATH_MSG_INFO( "Using model file: " << modelFileName );

    // set onnx session options
    Ort::SessionOptions sessionOptions;
    sessionOptions.SetIntraOpNumThreads( 1 );
    sessionOptions.SetGraphOptimizationLevel( ORT_ENABLE_BASIC );
    // set allocator
    Ort::AllocatorWithDefaultOptions allocator;
    // set the onnx runtime session
    std::shared_ptr<Ort::Session> sessionHandle = std::make_shared<Ort::Session>( env, modelFileName.c_str(), sessionOptions );

    ATH_MSG_INFO( "Created the ONNX Runtime session for model file = " << modelFileName);
    return std::make_tuple(sessionHandle, allocator);
  }

  //____________________________________________________________________________
  StatusCode PhotonVertexSelectionTool::initialize()
  {
    ATH_MSG_INFO("Initializing PhotonVertexSelectionTool...");
    // initialize the readers or sessions
    if(m_isTMVA){
      // Get full path of configuration files for MVA
      m_TMVAModelFilePath1  = PathResolverFindCalibFile( m_TMVAModelFilePath1 );
      m_TMVAModelFilePath2  = PathResolverFindCalibFile( m_TMVAModelFilePath2 );
      // Setup MVAs
      std::vector<std::string> var_names = {
        "deltaZ := TMath::Min(abs(PrimaryVerticesAuxDyn.z-zCommon)/zCommonError,20)",
        "deltaPhi := abs(deltaPhi(PrimaryVerticesAuxDyn.phi,egamma_phi))"           ,
        "logSumpt := log10(PrimaryVerticesAuxDyn.sumPt)"                            ,
        "logSumpt2 := log10(PrimaryVerticesAuxDyn.sumPt2)"
      };
      auto mva1 = new TMVA::Reader(var_names, "!Silent:Color");
      mva1->BookMVA    ("MLP method", m_TMVAModelFilePath1 );
      m_mva1 = std::unique_ptr<TMVA::Reader>( std::move(mva1) );

      auto mva2 = std::make_unique<TMVA::Reader>(var_names, "!Silent:Color");
      mva2->BookMVA    ("MLP method", m_TMVAModelFilePath2 );
      m_mva2 = std::unique_ptr<TMVA::Reader>( std::move(mva2) );
    }
    else{ // assume only ONNX for now
      // create onnx environment
      Ort::Env env;
      // converted
      std::tie(m_sessionHandle1, m_allocator1) = setONNXSession(env, m_ONNXModelFilePath1);
      std::tie(m_input_node_dims1,  m_input_node_names1 ) = getInputNodes( m_sessionHandle1, m_allocator1);
      std::tie(m_output_node_dims1, m_output_node_names1) = getOutputNodes(m_sessionHandle1, m_allocator1);

      // unconverted
      std::tie(m_sessionHandle2, m_allocator2) = setONNXSession(env, m_ONNXModelFilePath2);
      std::tie(m_input_node_dims2,  m_input_node_names2 ) = getInputNodes( m_sessionHandle2, m_allocator2);
      std::tie(m_output_node_dims2, m_output_node_names2) = getOutputNodes(m_sessionHandle2, m_allocator2);
    }

    // initialize the containers
    ATH_CHECK( m_eventInfo.initialize() );
    ATH_CHECK( m_vertexContainer.initialize() );

    m_deltaPhiKey = m_derivationPrefix + SG::decorKeyFromKey (m_deltaPhiKey.key());
    m_deltaZKey = m_derivationPrefix + SG::decorKeyFromKey (m_deltaZKey.key());
    m_sumPt2Key = m_derivationPrefix + SG::decorKeyFromKey (m_sumPt2Key.key());
    m_sumPtKey = m_derivationPrefix + SG::decorKeyFromKey (m_sumPtKey.key());
    ATH_CHECK( m_deltaPhiKey.initialize() );
    ATH_CHECK( m_deltaZKey.initialize() );
    ATH_CHECK( m_sumPt2Key.initialize() );
    ATH_CHECK( m_sumPtKey.initialize() );
#ifndef XAOD_STANDALONE
    renounce (m_sumPt2Key);
    renounce (m_sumPtKey);
#endif

    return StatusCode::SUCCESS;
  }

  //____________________________________________________________________________
  StatusCode PhotonVertexSelectionTool::decorateInputs(const xAOD::EgammaContainer &egammas, FailType* failType) const{
    auto fail = FailType::NoFail;

    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::WriteDecorHandle<xAOD::VertexContainer, float> deltaPhi (m_deltaPhiKey, ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> deltaZ (m_deltaZKey, ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> sumPt2 (m_sumPt2Key, ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> sumPt (m_sumPtKey, ctx);

    // Get the EventInfo
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo, ctx);

    // Find the common z-position from beam / photon pointing information
    std::pair<float, float> zCommon = xAOD::PVHelpers::getZCommonAndError(&*eventInfo, &egammas, m_convPtCut);
    // Vector sum of photons
    TLorentzVector vegamma = getEgammaVector(&egammas, fail);

    // Retrieve PV collection from TEvent
    SG::ReadHandle<xAOD::VertexContainer> vertices(m_vertexContainer, ctx);

    bool writeSumPt2 = !sumPt2.isAvailable();
    bool writeSumPt = !sumPt.isAvailable();

    for (const xAOD::Vertex* vertex: *vertices) {

      // Skip dummy vertices
      if (!(vertex->vertexType() == xAOD::VxType::VertexType::PriVtx ||
        vertex->vertexType() == xAOD::VxType::VertexType::PileUp)) continue;

      // Set input variables
      if (writeSumPt) {
        sumPt(*vertex) = xAOD::PVHelpers::getVertexSumPt(vertex, 1, false);
      }

      if (writeSumPt2) {
        sumPt2(*vertex) = xAOD::PVHelpers::getVertexSumPt(vertex, 2);
      }

      // Get momentum vector of vertex
      TLorentzVector vmom = xAOD::PVHelpers::getVertexMomentum(vertex, true, m_derivationPrefix);

      deltaPhi(*vertex) = (fail != FailType::FailEgamVect) ? std::abs(vmom.DeltaPhi(vegamma)) : -999.;
      deltaZ(*vertex) = std::abs((zCommon.first - vertex->z())/zCommon.second);

    } // loop over vertices

    ATH_MSG_DEBUG("DecorateInputs exit code "<< fail);
    if(failType!=nullptr)
      *failType = fail;
    return StatusCode::SUCCESS;
  }

  //____________________________________________________________________________
  std::vector<std::pair<const xAOD::Vertex*, float> >
  PhotonVertexSelectionTool::getVertex(const xAOD::EgammaContainer &egammas, bool ignoreConv, bool noDecorate, yyVtxType* vtxCasePtr, FailType* failTypePtr) const
  {
    const xAOD::Vertex *vertex = nullptr;
    std::vector<std::pair<const xAOD::Vertex*, float> > vertexMLP;
    yyVtxType vtxCase = yyVtxType::Unknown;
    FailType failType = FailType::NoFail;
    if (getVertexImp( egammas, vertex, ignoreConv, noDecorate, vertexMLP, vtxCase, failType ).isSuccess()) {
      std::sort(vertexMLP.begin(), vertexMLP.end(), sortMLP);
    }
    if(vtxCasePtr!=nullptr)
      *vtxCasePtr = vtxCase;
    if(failTypePtr!=nullptr)
      *failTypePtr = failType;

    return vertexMLP;
  }

  //____________________________________________________________________________
  StatusCode PhotonVertexSelectionTool::getVertex(const xAOD::EgammaContainer &egammas,
                                                  const xAOD::Vertex* &prime_vertex,
                                                  bool ignoreConv) const
  {
    std::vector<std::pair<const xAOD::Vertex*, float> > vertexMLP;
    yyVtxType vtxcase = yyVtxType::Unknown;
    FailType failType = FailType::NoFail;
    return getVertexImp( egammas, prime_vertex, ignoreConv, false, vertexMLP, vtxcase, failType );
  }

  StatusCode PhotonVertexSelectionTool::getVertexImp(const xAOD::EgammaContainer &egammas,
                                                     const xAOD::Vertex* &prime_vertex,
                                                     bool ignoreConv,
                                                     bool noDecorate,
                                                     std::vector<std::pair<const xAOD::Vertex*, float> >&  vertexMLP, yyVtxType& vtxCase, FailType& fail) const
  {
    // Set default vertex case and declare photon container
    vtxCase = yyVtxType::Unknown;
    const xAOD::PhotonContainer *photons = dynamic_cast<const xAOD::PhotonContainer*>(&egammas);

    // Retrieve PV collection from TEvent
    SG::ReadHandle<xAOD::VertexContainer> vertices(m_vertexContainer);

    if (!noDecorate && !decorateInputs(egammas).isSuccess()){
      return StatusCode::FAILURE;
    }

    // Check if a conversion photon has a track attached to a primary/pileup vertex
    if (!ignoreConv && photons) {
      prime_vertex = getPrimaryVertexFromConv(photons);
      if (prime_vertex != nullptr) {
        vtxCase = yyVtxType::ConvTrack;
        fail = FailType::MatchedTrack;
        vertexMLP.emplace_back(prime_vertex, 0.);
        return StatusCode::SUCCESS;
      }
    }

    if (fail != FailType::NoFail){
      ATH_MSG_VERBOSE("Returning hardest vertex. Fail detected (type="<< fail <<")");
      vertexMLP.clear();
      prime_vertex = xAOD::PVHelpers::getHardestVertex(&*vertices);
      vertexMLP.emplace_back(prime_vertex, 10.);
      return StatusCode::SUCCESS;
    }

    // Get the EventInfo
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo);

    // If there are any silicon conversions passing selection
    // ==> use Model 1 (Conv) otherwise Model 2 (Unconv)
    // Set default for conversion bool as false unless otherwise
    bool isConverted = false;

    // assume default NoSiTrack (unconverted) unless otherwise
    vtxCase = yyVtxType::NoSiTracks;
    if (!ignoreConv && photons) {
      for (const auto *photon: *photons) {
        if (!photon)
        {
          ATH_MSG_WARNING("Null pointer to photon");
          return StatusCode::FAILURE;
        }
        // find out if pass conversion selection criteria and tag as SiConvTrack case
        if (xAOD::PVHelpers::passConvSelection(photon, m_convPtCut))
        {
          isConverted = true;
          vtxCase = yyVtxType::SiConvTrack;
        }
      }
    }

    // if TMVA chosen, declare tmva_reader only once (before for looping vertex)
    TMVA::Reader *tmva_reader = new TMVA::Reader();
    if(m_isTMVA){
      if(isConverted){
        // If there are any silicon conversions passing selection, use MVA1 (converted case)
        tmva_reader = m_mva1.get();
      }
      // Otherwise, use MVA2 (unconverted case)
      if(!isConverted){
        tmva_reader = m_mva2.get();
      }
    }
    ATH_MSG_DEBUG("Vtx Case: " << vtxCase);

    // Vector sum of photons
    TLorentzVector vegamma = getEgammaVector(&egammas, fail);

    SG::AuxElement::ConstAccessor<float> sumPtA(m_derivationPrefix + "sumPt");
    SG::AuxElement::ConstAccessor<float> sumPt2A(m_derivationPrefix + "sumPt2");
    SG::AuxElement::ConstAccessor<float> deltaPhiA(m_derivationPrefix + "deltaPhi");
    SG::AuxElement::ConstAccessor<float> deltaZA(m_derivationPrefix + "deltaZ");

    // Loop over vertices and find best candidate
    std::vector<float> ONNXInputVector;
    std::vector<std::vector<float>> onnx_input_tensor_values;
    std::vector<float> TMVAInputVector;
    TString TMVAMethod;
    float mlp = 0.0, mlp_max = -99999.0;
    float doSkipByZSigmaScore = -9999.0;
    // assign threshold score value to compare later for good vtx
    float thresGoodVtxScore;
    if(m_doSkipByZSigma){thresGoodVtxScore = doSkipByZSigmaScore;}
    else{thresGoodVtxScore = mlp_max;}
    for (const xAOD::Vertex* vertex: *vertices) {
      // Skip dummy vertices
      if (!(vertex->vertexType() == xAOD::VxType::VertexType::PriVtx ||
        vertex->vertexType() == xAOD::VxType::VertexType::PileUp)) continue;

      onnx_input_tensor_values.clear();

      // Variables used as input features in classifier
      float sumPt, sumPt2, deltaPhi, deltaZ;
      float log10_sumPt, log10_sumPt2;

      sumPt     = (sumPtA)(*vertex);
      sumPt2    = (sumPt2A)(*vertex);
      deltaPhi  = (deltaPhiA)(*vertex);
      deltaZ    = (deltaZA)(*vertex);
      ATH_MSG_VERBOSE("sumPt: "     << sumPt    <<
                      " sumPt2: "   << sumPt2   <<
                      " deltaPhi: " << deltaPhi <<
                      " deltaZ: "   << deltaZ);

      // setup the vector of input features based on selected inference framework
      if(m_isTMVA){
        // Get likelihood probability from TMVA model
        TMVAMethod = "MLP method";
        log10_sumPt  = static_cast<float>(log10(sumPt));
        log10_sumPt2 = static_cast<float>(log10(sumPt2));
        TMVAInputVector = {deltaZ,deltaPhi,log10_sumPt,log10_sumPt2};
      }
      else{ //assume ony ONNX for now
        // Get likelihood probability from onnx model
        // check if value is 0, assign small number like 1e-8 as dummy, as we will take log later (log(0) is nan)
        // note that the ordering here is a bit different, following the order used when training
        ONNXInputVector = {sumPt2, sumPt, deltaPhi, deltaZ};
        for (long unsigned int i = 0; i < ONNXInputVector.size(); i++) {
          // skip log for deltaPhi and take log for the rest
          if (i == 2) {
            continue;
          }
          if (ONNXInputVector[i] != 0 && std::isinf(ONNXInputVector[i]) != true && std::isnan(ONNXInputVector[i]) != true){
            ONNXInputVector[i] = log(std::abs(ONNXInputVector[i]));
          }
          else{
            ONNXInputVector[i] = log(std::abs(0.00000001)); //log(abs(1e-8))
          }
        } //end ONNXInputVector for loop
        onnx_input_tensor_values.push_back(ONNXInputVector);
      }

      // Do the actual calculation of classifier score part
      if(m_isTMVA){
        mlp = tmva_reader->EvaluateMVA(TMVAInputVector, TMVAMethod);
        ATH_MSG_VERBOSE("TMVA output: "  << (tmva_reader == m_mva1.get() ? "MVA1 ": "MVA2 ")<< mlp);
      }
      else{ //assume ony ONNX for now
        if(isConverted){
          mlp = getScore(m_nVars, onnx_input_tensor_values,
                         m_sessionHandle1, m_input_node_dims1,
                         m_input_node_names1, m_output_node_names1);
        }
        if(!isConverted){
          mlp = getScore(m_nVars, onnx_input_tensor_values,
                         m_sessionHandle2, m_input_node_dims2,
                         m_input_node_names2, m_output_node_names2);
        }
        ATH_MSG_VERBOSE("log(abs(sumPt)): "     << sumPt    <<
                        " log(abs(sumPt2)): "   << sumPt2   <<
                        " deltaPhi: "           << deltaPhi <<
                        " log(abs(deltaZ)): "   << deltaZ);
        ATH_MSG_VERBOSE("ONNX output, isConverted = " << isConverted << ", mlp=" << mlp);
      }

      // Skip vertices above 10 sigma from pointing or 15 sigma from conversion (HPV)
      // Simply displace the mlp variable we calculate before by a predefined value
      if(m_doSkipByZSigma){
        if ((isConverted && deltaZ > 15) || (!isConverted && deltaZ > 10)) {
          mlp = doSkipByZSigmaScore;
        }
      }

      // add the new vertex and its score to vertexMLP container
      vertexMLP.emplace_back(vertex, mlp);

      // Keep track of maximal likelihood vertex
      if (mlp > mlp_max) {
        mlp_max = mlp;
        prime_vertex = vertex;
      }
    } // end loop over vertices

    // from all the looped vertices, decide the max score which should be more than the minimum we set
    // (which should be more than the initial mlp_max value above or more than the skip vertex by z-sigma score)
    // if this does not pass, return hardest primary vertex
    if (mlp_max <= thresGoodVtxScore) {
      ATH_MSG_DEBUG("No good vertex candidates from pointing, returning hardest vertex.");
      prime_vertex = xAOD::PVHelpers::getHardestVertex(&*vertices);
      fail = FailType::NoGdCandidate;
      vertexMLP.clear();
      vertexMLP.emplace_back(xAOD::PVHelpers::getHardestVertex(&*vertices), 20.);
    }

    ATH_MSG_VERBOSE("getVertex case "<< (int)vtxCase << " exit code "<< (int)fail);
    return StatusCode::SUCCESS;
  }

  //____________________________________________________________________________
  bool PhotonVertexSelectionTool::sortMLP(const std::pair<const xAOD::Vertex*, float> &a,
                                          const std::pair<const xAOD::Vertex*, float> &b)
  { return a.second > b.second; }

  //____________________________________________________________________________
  const xAOD::Vertex* PhotonVertexSelectionTool::getPrimaryVertexFromConv(const xAOD::PhotonContainer *photons) const
  {
    if (photons == nullptr) {
      ATH_MSG_WARNING("Passed nullptr photon container, returning nullptr vertex from getPrimaryVertexFromConv");
      return nullptr;
    }

    std::vector<const xAOD::Vertex*> vertices;
    const xAOD::Vertex *conversionVertex = nullptr, *primary = nullptr;
    const xAOD::TrackParticle *tp = nullptr;
    size_t NumberOfTracks = 0;

    // Retrieve PV collection from TEvent
    SG::ReadHandle<xAOD::VertexContainer> all_vertices(m_vertexContainer);


    for (const auto *photon: *photons) {
      conversionVertex = photon->vertex();
      if (conversionVertex == nullptr) continue;

      NumberOfTracks = conversionVertex->nTrackParticles();
      for (size_t i = 0; i < NumberOfTracks; ++i) {
        // Get trackParticle in GSF collection
        const auto *gsfTp = conversionVertex->trackParticle(i);
        if (gsfTp == nullptr) continue;
        if (!xAOD::PVHelpers::passConvSelection(*conversionVertex, i, m_convPtCut)) continue;

        // Get trackParticle in InDet collection
        tp = xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF(gsfTp);
        if (tp == nullptr) continue;

        primary = getVertexFromTrack(tp, &*all_vertices);
        if (primary == nullptr) continue;

        if (primary->vertexType() == xAOD::VxType::VertexType::PriVtx ||
            primary->vertexType() == xAOD::VxType::VertexType::PileUp) {
          if (std::find(vertices.begin(), vertices.end(), primary) == vertices.end()) {
            vertices.push_back(primary);
            continue;
          }
        }
      }
    }

    if (!vertices.empty()) {
      if (vertices.size() > 1)
        ATH_MSG_WARNING("Photons associated to different vertices! Returning lead photon association.");
      return vertices[0];
    }

    return nullptr;
  }

  //____________________________________________________________________________
  TLorentzVector PhotonVertexSelectionTool::getEgammaVector(const xAOD::EgammaContainer *egammas, FailType& failType) const
  {
    TLorentzVector v, v1;
    const xAOD::CaloCluster *cluster = nullptr;
    for (const xAOD::Egamma* egamma: *egammas) {
      if (egamma == nullptr) {
        ATH_MSG_DEBUG("No egamma object to get four vector");
        failType = FailType::FailEgamVect;
        continue;
      }
      cluster = egamma->caloCluster();
      if (cluster == nullptr) {
        ATH_MSG_WARNING("No cluster associated to egamma, not adding to 4-vector.");
        continue;
      }

      v1.SetPtEtaPhiM(egamma->e()/cosh(cluster->etaBE(2)),
                      cluster->etaBE(2),
                      cluster->phiBE(2),
                      0.0);
      v += v1;
    }
    return v;
  }

} // namespace CP
