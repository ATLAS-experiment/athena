// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "EvaluateUtils.h"

#include <fstream>
#include <arpa/inet.h>

namespace EvaluateUtils {
   //*******************************************************************
   // for reading MNIST images
   std::vector<std::vector<std::vector<float>>> read_mnist_pixel_notFlat(const std::string &full_path)
   {
     std::vector<std::vector<std::vector<float>>> input_tensor_values;
     input_tensor_values.resize(10000, std::vector<std::vector<float> >(28,std::vector<float>(28)));
     std::ifstream file (full_path.c_str(), std::ios::binary);
     int magic_number=0;
     int number_of_images=0;
     int n_rows=0;
     int n_cols=0;
     file.read(reinterpret_cast<char*>(&magic_number),sizeof(magic_number));
     magic_number= ntohl(magic_number);
     file.read(reinterpret_cast<char*>(&number_of_images),sizeof(number_of_images));
     number_of_images= ntohl(number_of_images);
     file.read(reinterpret_cast<char*>(&n_rows),sizeof(n_rows));
     n_rows= ntohl(n_rows);
     file.read(reinterpret_cast<char*>(&n_cols),sizeof(n_cols));
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
