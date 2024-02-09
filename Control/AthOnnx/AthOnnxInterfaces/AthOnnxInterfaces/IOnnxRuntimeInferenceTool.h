// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef AthOnnx_IOnnxRuntimeInferenceTool_H
#define AthOnnx_IOnnxRuntimeInferenceTool_H

// Gaudi include(s).
#include "GaudiKernel/IAlgTool.h"
#include <memory>

#include <core/session/onnxruntime_cxx_api.h>


namespace AthOnnx {
    /**
     * @class IOnnxRuntimeInferenceTool
     * @brief Interface class for creating Onnx Runtime sessions.
     * @details Interface class for creating Onnx Runtime sessions.
     * It is thread safe, supports models with various number of inputs and outputs,
     * supports models with dynamic batch size, and usess . It defines a standardized procedure to 
     * perform Onnx Runtime inference. The procedure is as follows, assuming the tool `m_onnxTool` is created and initialized:
     *    1. create input tensors from the input data: 
     *      ```c++
     *         std::vector<Ort::Value> inputTensors;
     *         std::vector<float> inputData_1;   // The input data is filled by users, possibly from the event information. 
     *         int64_t batchSize = m_onnxTool->getBatchSize(inputData_1.size(), 0);  // The batch size is determined by the input data size to support dynamic batch size.
     *         m_onnxTool->addInput(inputTensors, inputData_1, 0, batchSize);
     *         std::vector<int64_t> inputData_2;  // Some models may have multiple inputs. Add inputs one by one.
     *         int64_t batchSize_2 = m_onnxTool->getBatchSize(inputData_2.size(), 1);
     *         m_onnxTool->addInput(inputTensors, inputData_2, 1, batchSize_2);
     *     ```
     *    2. create output tensors:
     *      ```c++
     *          std::vector<Ort::Value> outputTensors;
     *          std::vector<float> outputData;   // The output data will be filled by the onnx session.
     *          m_onnxTool->addOutput(outputTensors, outputData, 0, batchSize);
     *      ```
     *   3. perform inference:
     *     ```c++
     *        m_onnxTool->inference(inputTensors, outputTensors);
     *    ```
     *   4. Model outputs will be automatically filled to outputData.
     * 
     * 
     * @author Xiangyang Ju <xju@cern.ch>
     */
    class IOnnxRuntimeInferenceTool : virtual public IAlgTool 
    {
        public:

        virtual ~IOnnxRuntimeInferenceTool() = default;
        
        // @name InterfaceID
        DeclareInterfaceID(IOnnxRuntimeInferenceTool, 1, 0);

        /**
         * @brief set batch size. 
         * @details If the model has dynamic batch size, 
         *          the batchSize value will be set to both input shapes and output shapes
         */ 
        virtual void setBatchSize(int64_t batchSize) = 0;

        /**
         * @brief methods for determining batch size from the data size
         * @param dataSize the size of the input data, like std::vector<T>::size()
         * @param idx the index of the input node
         * @return the batch size, which equals to dataSize / size of the rest dimensions.
         */ 
        virtual int64_t getBatchSize(int64_t dataSize, int idx = 0) const = 0;

        /**
         * @brief add the input data to the input tensors
         * @param inputTensors the input tensor container
         * @param data the input data
         * @param idx the index of the input node
         * @param batchSize the batch size
         * @return StatusCode::SUCCESS if the input data is added successfully
         */
        template <typename T>
        StatusCode addInput(std::vector<Ort::Value>& inputTensors, std::vector<T>& data, int idx = 0, int64_t batchSize = -1) const;

        /**
         * @brief add the output data to the output tensors
         * @param outputTensors the output tensor container
         * @param data the output data
         * @param idx the index of the output node
         * @param batchSize the batch size
         * @return StatusCode::SUCCESS if the output data is added successfully
         */
        template <typename T>
        StatusCode addOutput(std::vector<Ort::Value>& outputTensors, std::vector<T>& data, int idx = 0, int64_t batchSize = -1) const;

        /**
         * @brief perform inference
         * @param inputTensors the input tensor container
         * @param outputTensors the output tensor container
         * @return StatusCode::SUCCESS if the inference is performed successfully
         */
        virtual StatusCode inference(std::vector<Ort::Value>& inputTensors, std::vector<Ort::Value>& outputTensors) const = 0;

        virtual void printModelInfo() const = 0;

        protected:
        int m_numInputs;
        int m_numOutputs;
        std::vector<std::vector<int64_t> > m_inputShapes;
        std::vector<std::vector<int64_t> > m_outputShapes;

        private:
        template <typename T>
        Ort::Value createTensor(std::vector<T>& data, const std::vector<int64_t>& dataShape, int64_t batchSize) const;

    };

    #include "IOnnxRuntimeInferenceTool.icc"
} // namespace AthOnnx

#endif
