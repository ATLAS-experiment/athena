/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#undef NDEBUG

#include <chrono>
#include <iostream>
#include <vector>
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

struct TestData
{
  TestData() = default;

  TestData(const uint64_t sizeOfData)
  {
    // seed generation
    srand((unsigned) time(NULL));

    // random values
    for (unsigned i{0}; i < sizeOfData; ++i)
    {
      elementIdList.push_back(rand() % 1000);

      Eigen::Matrix<float,3,1> globPosition;
      for (unsigned j{0}; j < 3; ++j)
      {
        globPosition(j, 0) = rand() % 1000;
      }
      globalPosition.push_back(globPosition);

      varianceR.push_back(rand() % 1000);

      varianceZ.push_back(rand() % 1000);

      std::vector< const xAOD::UncalibratedMeasurement* > measurementIndex;
      measurementIndex.push_back(nullptr);
      measurementIndexes.push_back(measurementIndex);
    }
  }

  std::vector < unsigned int > elementIdList;
  std::vector < Eigen::Matrix<float,3,1> > globalPosition;
  std::vector < float > varianceR;
  std::vector < float > varianceZ;
  std::vector < std::vector< const xAOD::UncalibratedMeasurement* > > measurementIndexes;
};

void clearCache()
{
  constexpr size_t biggerThanCacheSize{10 * 1024 * 1024};
  long *p = new long[biggerThanCacheSize];

  const auto randValue{rand()};
  for(size_t i = 0; i < biggerThanCacheSize; i++)
  {
    p[i] = randValue;
  }
  delete[] p;
}

void testCopyMechanism(const xAOD::SpacePointContainer& container, const TestData& structure)
{
  bool checkResult = true;
  for (uint64_t i{0}; i < structure.elementIdList.size(); ++i)
  {
    checkResult &= (structure.elementIdList[i] == container[i]->elementIdList()[0]);
    checkResult &= (structure.globalPosition[i] == container[i]->globalPosition());
    checkResult &= (structure.varianceR[i] == container[i]->varianceR());
    checkResult &= (structure.varianceZ[i] == container[i]->varianceZ());
    checkResult &= (structure.measurementIndexes[i][0] == container[i]->measurements()[0]);
  }
  if (not checkResult)
  {
    std::cout << "\nCopy check failed" << std::endl;
  }
}

template <typename F>
void writeAndReadContainerWithGivenAlgorithm(const F& alg, const TestData& origTestPoints)
{
  constexpr uint8_t numOfTestsPerSet{5};
  std::chrono::microseconds writeTestResult{0};
  std::chrono::microseconds readTestResult{0};

  for (uint8_t runNumber{0}; runNumber < numOfTestsPerSet; ++runNumber)
  {
    xAOD::SpacePointContainer spContainer;
    xAOD::SpacePointAuxContainer spContainer_aux;
    spContainer.setStore(&spContainer_aux);

    // container write time measure
    {
      const auto startTime = std::chrono::high_resolution_clock::now();
      alg(spContainer);
      const auto stopTime = std::chrono::high_resolution_clock::now();

      writeTestResult += std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime);
      testCopyMechanism(spContainer, origTestPoints);
      clearCache();
    }

    TestData dataDestination{};

    // container read time measure
    {
      const auto startTime = std::chrono::high_resolution_clock::now();
      for (uint64_t i{0}; i < spContainer.size(); ++i)
      {
        const auto& sp{*spContainer[i]};
        dataDestination.elementIdList.push_back(sp.elementIdList()[0]);
        dataDestination.globalPosition.push_back(sp.globalPosition());
        dataDestination.varianceR.push_back(sp.varianceR());
        dataDestination.varianceZ.push_back(sp.varianceZ());
        dataDestination.measurementIndexes.push_back(sp.measurements());
      }
      const auto stopTime = std::chrono::high_resolution_clock::now();

      readTestResult += std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime);
      testCopyMechanism(spContainer, dataDestination);
      clearCache();
    }
  }

  std::cout << "Write time: " << (writeTestResult/numOfTestsPerSet).count() << ", ";
  std::cout << "Read time: " << (readTestResult/numOfTestsPerSet).count() << std::endl;
}

void setSpacePointFromNthPosition(xAOD::SpacePointContainer& spContainer, const TestData& testData, const uint64_t pos)
{
  spContainer[pos]->setSpacePoint(
    testData.elementIdList[pos],
    testData.globalPosition[pos],
    testData.varianceR[pos],
    testData.varianceZ[pos],
    testData.measurementIndexes[pos]
  );
}

int main(int argc, char *argv[])
{
  if (argc == 2 && strcmp(argv[1], "--help") == 0)
  {
    std::cout << "For custom number of data points type e.g. ./TransferBenchmark_test.exe 100" << std::endl;
    return 0;
  }
  std::cout << "Start of data transfer speed test, time in microseconds (average from 5 runs)" << std::endl;

  std::vector<uint64_t> testCasesNumOfDataPoints = {10, 100, 1000, 10000, 100000};
  if (argc > 1)
  {
    testCasesNumOfDataPoints.clear();
    for (uint8_t i{1}; i < argc; ++i)
    {
      testCasesNumOfDataPoints.push_back(std::stoi(argv[i]));
    }
  }
  
  for (uint64_t numOfDataPoints : testCasesNumOfDataPoints)
  {
    std::cout << "Number of data points: " << numOfDataPoints << std::endl;
    const TestData testData{numOfDataPoints};

    // Base
    {
      std::cout << "\tBase: ";
      auto baseAlg = [numOfDataPoints, testData](xAOD::SpacePointContainer& spContainer)
        {
          for (uint64_t i{0}; i < numOfDataPoints; ++i)
          {
            spContainer.push_back(new xAOD::SpacePoint());
            setSpacePointFromNthPosition(spContainer, testData, i);
          }
        };
      writeAndReadContainerWithGivenAlgorithm(baseAlg, testData);
    }

    // Reserve
    {
      std::cout << "\tReserve: ";
      auto earlierReserveAlg = [numOfDataPoints, testData](xAOD::SpacePointContainer& spContainer)
        {
          spContainer.reserve(numOfDataPoints);
          for (uint64_t i{0}; i < numOfDataPoints; ++i)
          {
            spContainer.push_back(new xAOD::SpacePoint());
            setSpacePointFromNthPosition(spContainer, testData, i);
          }
        };
      writeAndReadContainerWithGivenAlgorithm(earlierReserveAlg, testData);
    }

    // Resize
    {
      std::cout << "\tResize: ";
      auto earlierResizeAlg = [numOfDataPoints, testData](xAOD::SpacePointContainer& spContainer)
        {
          spContainer.resize(numOfDataPoints);
          for (uint64_t i{0}; i < numOfDataPoints; ++i)
          {
            spContainer.at(i) = new xAOD::SpacePoint();
            setSpacePointFromNthPosition(spContainer, testData, i);
          }
        };
      writeAndReadContainerWithGivenAlgorithm(earlierResizeAlg, testData);
    }

    // Reserve + preallocation
    {
      std::cout << "\tReserve + preallocation: ";
      auto earlierReservePreallocAlg = [numOfDataPoints, testData](xAOD::SpacePointContainer& spContainer)
        {
          spContainer.reserve(numOfDataPoints);
          for (uint64_t i{0}; i < numOfDataPoints; ++i)
          {
            spContainer.push_back(new xAOD::SpacePoint());
          }

          for (uint64_t i{0}; i < numOfDataPoints; ++i)
          {
            setSpacePointFromNthPosition(spContainer, testData, i);
          }
        };
      writeAndReadContainerWithGivenAlgorithm(earlierReservePreallocAlg, testData);
    }

    // Resize + preallocation
    {
      std::cout << "\tResize + preallocation: ";
      auto earlierResizePreallocAlg = [numOfDataPoints, testData](xAOD::SpacePointContainer& spContainer)
        {
          spContainer.resize(numOfDataPoints);
          for (uint64_t i{0}; i < numOfDataPoints; ++i)
          {
            spContainer.at(i) = new xAOD::SpacePoint();
          }

          for (uint64_t i{0}; i < numOfDataPoints; ++i)
          {
            setSpacePointFromNthPosition(spContainer, testData, i);
          }
        };
      writeAndReadContainerWithGivenAlgorithm(earlierResizePreallocAlg, testData);
    }

    std::cout << std::endl;
  }
}
