// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "ExampleMLInferenceWithTriton.h"

// Framework include(s).
#include "PathResolver/PathResolver.h"
#include <arpa/inet.h>
#include <fstream>

namespace AthInfer {

StatusCode ExampleMLInferenceWithTriton::initialize() {
    // Fetch tools
    ATH_CHECK( m_tritonTool.retrieve() );
       
   if(m_batchSize > 10000){
      ATH_MSG_INFO("The total no. of sample crossed the no. of available sample ....");
	   return StatusCode::FAILURE;
   }
   // read input file, and the target file for comparison.
   std::string pixelFilePath = PathResolver::find_calib_file(m_pixelFileName);
   ATH_MSG_INFO( "Using pixel file: " << pixelFilePath );
  
   m_input_tensor_values_notFlat = read_mnist_pixel_notFlat(pixelFilePath);
   ATH_MSG_INFO("Total no. of samples: "<<m_input_tensor_values_notFlat.size());
    
   return StatusCode::SUCCESS;
}

StatusCode ExampleMLInferenceWithTriton::execute( [[maybe_unused]] const EventContext& ctx ) const {

   // prepare inputs
   std::vector<float> inputDataVector;
   inputDataVector.reserve(m_input_tensor_values_notFlat.size());
   for (const std::vector<std::vector<float> >& imageData : m_input_tensor_values_notFlat){

      std::vector<float> flatten;
      int total_size = 0;
      for(const auto& feature : imageData) total_size += feature.size();
      flatten.reserve(total_size);
      for (const auto& feature : imageData)
         for (const auto& elem : feature)
            flatten.push_back(elem);

      inputDataVector.insert(inputDataVector.end(), flatten.begin(), flatten.end());
   }
   std::vector<int64_t> inputShape = {m_batchSize, 28, 28};

   AthInfer::InputDataMap inputData;
   inputData["flatten_input:0"] = std::make_pair(
      inputShape, std::move(inputDataVector)
   );

   AthInfer::OutputDataMap outputData;
   outputData["dense_1/Softmax:0"] = std::make_pair(
      std::vector<int64_t>{m_batchSize, 10}, std::vector<float>{}
   );

   ATH_CHECK(m_tritonTool->inference(inputData, outputData));

   auto& outputScores = std::get<std::vector<float>>(outputData["dense_1/Softmax:0"].second);
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
        ATH_MSG_ERROR("No maximum found in ExampleMLInferenceWithTriton::execute");
        return StatusCode::FAILURE;
      }
      ATH_MSG_DEBUG("Class: "<<max_index<<" has the highest score: "<<outputScores[max_index] << " in batch " << ibatch);
   }

   return StatusCode::SUCCESS;
}

std::vector<std::vector<std::vector<float>>> 
ExampleMLInferenceWithTriton::read_mnist_pixel_notFlat(const std::string &full_path) const
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

}
