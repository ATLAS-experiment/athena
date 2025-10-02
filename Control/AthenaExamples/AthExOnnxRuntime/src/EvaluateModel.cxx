// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "EvaluateModel.h"

// Framework include(s).
#include "AthOnnxUtils/OnnxUtils.h"
#include "EvaluateUtils.h"
#include "PathResolver/PathResolver.h"

namespace AthOnnx {

   StatusCode EvaluateModel::initialize() {
    // Fetch tools
    ATH_CHECK( m_onnxTool.retrieve() );
    m_onnxTool->printModelInfo();

      /*****
       The combination of no. of batches and batch size shouldn't cross 
       the total smple size which is 10000 for this example
      *****/         
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

   StatusCode EvaluateModel::execute( const EventContext& /*ctx*/ ) const {
   
   // prepare inputs
   std::vector<float> inputData;
   for (int ibatch = 0; ibatch < m_batchSize; ibatch++){
      const std::vector<std::vector<float> >& imageData = m_input_tensor_values_notFlat[ibatch];
      std::vector<float> flatten = AthOnnxUtils::flattenNestedVectors(imageData);
      inputData.insert(inputData.end(), flatten.begin(), flatten.end());
   }

   int64_t batchSize = m_onnxTool->getBatchSize(inputData.size());
   ATH_MSG_INFO("Batch size is " << batchSize << ".");
   assert(batchSize == m_batchSize);

   // bind the input data to the input tensor
   std::vector<Ort::Value> inputTensors;
   ATH_CHECK( m_onnxTool->addInput(inputTensors, inputData, 0, batchSize) );

   // reserve space for output data and bind it to the output tensor
   std::vector<float> outputScores;
   std::vector<Ort::Value> outputTensors;
   ATH_CHECK( m_onnxTool->addOutput(outputTensors, outputScores, 0, batchSize) );

   // run the inference
   // the output will be filled to the outputScores.
   ATH_CHECK( m_onnxTool->inference(inputTensors, outputTensors) );

     	ATH_MSG_INFO("Label for the input test data: ");
   for(int ibatch = 0; ibatch < m_batchSize; ibatch++){
     	float max = -999;
     	int max_index = 0;
     	for (int i = 0; i < 10; i++){
       		ATH_MSG_DEBUG("Score for class "<< i <<" = "<<outputScores[i] << " in batch " << ibatch);
            int index = i + ibatch * 10;
       		if (max < outputScores[index]){
          		max = outputScores[index];
          		max_index = index;
       		}
     	}
      ATH_MSG_INFO("Class: "<<max_index<<" has the highest score: "<<outputScores[max_index] << " in batch " << ibatch);
   }

      return StatusCode::SUCCESS;
   }

} // namespace AthOnnx
