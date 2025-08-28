// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "EvaluateModelWithAthInfer.h"

// Framework include(s).
#include "AthOnnxUtils/OnnxUtils.h"
#include "EvaluateUtils.h"
#include "PathResolver/PathResolver.h"

namespace AthOnnx {

StatusCode EvaluateModelWithAthInfer::initialize() {
    // Fetch tools
    ATH_CHECK( m_onnxTool.retrieve() );
       
   if(m_batchSize > 10000){
      ATH_MSG_INFO("The total no. of sample crossed the no. of available sample ....");
	   return StatusCode::FAILURE;
   }
   // read input file, and the target file for comparison.
   std::string pixelFilePath = PathResolver::find_calib_file(m_pixelFileName.value());
   ATH_MSG_INFO( "Using pixel file: " << pixelFilePath );
  
   m_input_tensor_values_notFlat = EvaluateUtils::read_mnist_pixel_notFlat(pixelFilePath);
   ATH_MSG_INFO("Total no. of samples: "<<m_input_tensor_values_notFlat.size());
    
   return StatusCode::SUCCESS;
}

StatusCode EvaluateModelWithAthInfer::execute( [[maybe_unused]] const EventContext& ctx ) const {

   // prepare inputs
   std::vector<float> inputDataVector;
   inputDataVector.reserve(m_input_tensor_values_notFlat.size());
   for (const std::vector<std::vector<float> >& imageData : m_input_tensor_values_notFlat){
      std::vector<float> flatten = AthOnnxUtils::flattenNestedVectors(imageData);
      inputDataVector.insert(inputDataVector.end(), flatten.begin(), flatten.end());
   }
   std::vector<int64_t> inputShape = {m_batchSize, 28, 28};

   AthInfer::InputDataMap inputData;
   inputData["flatten_input"] = std::make_pair(
      inputShape, std::move(inputDataVector)
   );

   AthInfer::OutputDataMap outputData;
   outputData["dense_1/Softmax"] = std::make_pair(
      std::vector<int64_t>{m_batchSize, 10}, std::vector<float>{}
   );

   ATH_CHECK(m_onnxTool->inference(inputData, outputData));

   auto& outputScores = std::get<std::vector<float>>(outputData["dense_1/Softmax"].second);
   auto inRange = [&outputScores](int idx)->bool{return (idx>=0) and (idx<std::ssize(outputScores));};
   ATH_MSG_DEBUG("Label for the input test data: ");
   for(int ibatch = 0; ibatch < m_batchSize; ibatch++){
      float max = -999;
      int max_index{-1};
      for (int i = 0; i < 10; i++){
            ATH_MSG_DEBUG("Score for class "<< i <<" = "<<outputScores[i] << " in batch " << ibatch);
            int index = i + ibatch * 10;
            if (not inRange(index)) continue;
            if (max < outputScores[index]){
               max = outputScores[index];
               max_index = index;
            }
      }
      if (not inRange(max_index)){
        ATH_MSG_ERROR("No maximum found in EvaluateModelWithAthInfer::execute");
        return StatusCode::FAILURE;
      }
      ATH_MSG_DEBUG("Class: "<<max_index<<" has the highest score: "<<outputScores[max_index] << " in batch " << ibatch);
   }

   return StatusCode::SUCCESS;
}

} // namespace AthOnnx
