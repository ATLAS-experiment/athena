// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "EvaluateModel.h"
#include <tuple>
#include <fstream>
#include <chrono>
#include <arpa/inet.h>

// Framework include(s).
#include "AthOnnxUtils/OnnxUtils.h"

namespace AthOnnx {

   //*******************************************************************
   // for reading MNIST images
   std::vector<std::vector<std::vector<float>>> read_mnist_pixel_notFlat(const std::string &full_path) //function to load test images
   {
     std::vector<std::vector<std::vector<float>>> input_tensor_values;
     input_tensor_values.resize(10000, std::vector<std::vector<float> >(28,std::vector<float>(28)));
     std::ifstream file (full_path.c_str(), std::ios::binary);
     int magic_number=0;
     int number_of_images=0;
     int n_rows=0;
     int n_cols=0;
     file.read((char*)&magic_number,sizeof(magic_number));
     magic_number= ntohl(magic_number);
     file.read((char*)&number_of_images,sizeof(number_of_images));
     number_of_images= ntohl(number_of_images);
     file.read((char*)&n_rows,sizeof(n_rows));
     n_rows= ntohl(n_rows);
     file.read((char*)&n_cols,sizeof(n_cols));
     n_cols= ntohl(n_cols);
     for(int i=0;i<number_of_images;++i)
     {
      	for(int r=0;r<n_rows;++r)
        {
           for(int c=0;c<n_cols;++c)
           {
             unsigned char temp=0;
             file.read((char*)&temp,sizeof(temp));
             input_tensor_values[i][r][c]= float(temp)/255;
           }
	}
     }
     return input_tensor_values;
   }

   //********************************************************************************
   // for reading MNIST labels
   std::vector<int> read_mnist_label(const std::string &full_path) //function to load test labels
   {
     std::vector<int> output_tensor_values(1*10000);
     std::ifstream file (full_path.c_str(), std::ios::binary);
     int magic_number=0;
     int number_of_labels=0;
     file.read((char*)&magic_number,sizeof(magic_number));
     magic_number= ntohl(magic_number);
     file.read((char*)&number_of_labels,sizeof(number_of_labels));
     number_of_labels= ntohl(number_of_labels);
     for(int i=0;i<number_of_labels;++i)
     {
          unsigned char temp=0;
          file.read((char*)&temp,sizeof(temp));
          output_tensor_values[i]= int(temp);
     }
      return output_tensor_values;
    }

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
      ATH_MSG_INFO( "Using pixel file: " << m_pixelFileName.value() );
  
      m_input_tensor_values_notFlat = read_mnist_pixel_notFlat(m_pixelFileName);
      ATH_MSG_INFO("Total no. of samples: "<<m_input_tensor_values_notFlat.size());
    
      return StatusCode::SUCCESS;
}

   StatusCode EvaluateModel::execute( const EventContext& /*ctx*/ ) const {
   
   // prepare inputs
   std::vector<float> inputData;
   for (int ibatch = 0; ibatch < m_batchSize; ibatch++){
      const std::vector<std::vector<float> >& imageData = m_input_tensor_values_notFlat[ibatch];
      std::vector<float> flatten = AthOnnx::flattenNestedVectors(imageData);
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
     	int max_index;
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
   StatusCode EvaluateModel::finalize() {

      return StatusCode::SUCCESS;
   }

} // namespace AthOnnx
