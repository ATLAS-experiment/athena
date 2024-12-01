/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ZdcUtils/ZDCTriggerSim.h
 * @author Brian Cole <bcole@cern.ch>, Matthew Hoppesch <mhoppesc@cern.ch>
 * @date May 2023
 * @brief A tool to make L1 decision using LUTs
 */

#ifndef ZDCTRIGVALID__ZDCTriggerSim__h
#define ZDCTRIGVALID__ZDCTriggerSim__h

#include <array>
#include <cmath>
#include <list>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace ZDCTriggerSim {
  enum DataType { TCombLUTOutput, TCombLUTInput, TSideLUTsInput, TModAmplsInput, TFADCInputs };

const std::vector<std::string> TypeStrings = {"CombLUTOutput", "CombLUTInput",
                                              "SideLUTsInput", "ModAmplsInput", "FADCInput"};
}  // namespace ZDCTriggerSim

//
// Base class for simulation data. Primarily defines interface and virtual
// destructor
//
class ZDCTriggerSimDataBase {
 public:
  virtual ~ZDCTriggerSimDataBase() = default;

  virtual unsigned int getNumBits() const = 0;
  virtual unsigned int getNumData() const = 0;
  virtual ZDCTriggerSim::DataType getType() const = 0;

  virtual unsigned int getValueTrunc(unsigned int idx = 0) const = 0;
  virtual void dump(std::ostream& strm) const = 0;
};

// Template class that defines data of type T -- usually but not always unsigned
// integer
//   with NData different values and which will be truncated to NBits
//
//   The class also allows the use of conversion factors that are applied to the
//   input values
//     when the data is actually used. This allows translation between (e.g.)
//     energies and ADC values or any other kind of required conversion.
//
//
template <typename T, unsigned int NData, unsigned int NBits,
          ZDCTriggerSim::DataType Type>
class ZDCTriggerSimData : public ZDCTriggerSimDataBase {
  bool m_doConvert{};
  std::vector<float> m_convertFactors;

  std::vector<T> m_data;
  bool m_haveData{};

  unsigned int doConvTrunc(const T& inValue) const {
    unsigned int value = inValue;

    if (m_doConvert) {
      value = std::floor(inValue * m_convertFactors.at(0));
    }

    unsigned int valueTruncZero = std::max(static_cast<unsigned int>(0), value);
    unsigned int valueTruncBits =
        std::min(valueTruncZero, static_cast<unsigned int>((1 << NBits) - 1));
    return valueTruncBits;
  }

  //  static const unsigned int maskBits = (2<<NBits) - 1;
 public:
  ZDCTriggerSimData()
      : m_doConvert(false), m_data(NData, 0), m_haveData(false) {
    static_assert(NData > 0, "ZDCTriggerSimData requires at least one datum");
    static_assert(NBits > 0, "ZDCTriggerSimData requires at least 1 bit");
  }

  ZDCTriggerSimData(const std::vector<float>& conversionFactors)
      : m_doConvert(true),
        m_convertFactors(conversionFactors),
        m_data(NData, 0),
        m_haveData(false) {
    static_assert(NData > 0, "ZDCTriggerSimData requires at least one datum");
    static_assert(NBits > 0, "ZDCTriggerSimData requires at least 1 bit");
  }

  virtual ~ZDCTriggerSimData() override {}

  unsigned int getNumBits() const override { return NBits; }
  unsigned int getNumData() const override { return NData; }
  virtual ZDCTriggerSim::DataType getType() const override { return Type; }

  virtual unsigned int getValueTrunc(unsigned int idx = 0) const override {
    if (!m_haveData)
      throw std::logic_error("No data available for ZDCTriggerSimData");
    return doConvTrunc(m_data.at(idx));
  }

  void setDatum(T datum) {
    if (NData != 1)
      throw std::logic_error(
          "ZDCTriggerSimData setDatum called with NData > 1");
    ;
    m_haveData = true;
    m_data[0] = datum;
  }

  void setData(const std::vector<T>& inData) {
    m_data = inData;
    m_haveData = true;
  }

  void clearData() { m_haveData = false; }

  virtual void dump(std::ostream& strm) const override {
    strm << "Data for Type: " << Type << ": ";
    for (auto datum : m_data) {
      strm << doConvTrunc(datum) << " ";
    }
    strm << std::endl;
  }
};

namespace ZDCTriggerSim {
// The usual way we provide the module amplitudes
//

// It's easier to input the FADC data as a single vector of 8 channels * NBCID* 8 samples/BCID 12-bit data
//   For physics data, we usually read 3 BCIDs 
// 
typedef ZDCTriggerSimData<unsigned int, 8*24, 12, TFADCInputs> FADCInputs;

typedef ZDCTriggerSimData<unsigned int, 8, 12, TModAmplsInput>
    ModuleAmplInputsInt;

// The usual way we provide the module amplitudes
//
typedef ZDCTriggerSimData<float, 8, 12, TModAmplsInput> ModuleAmplInputsFloat;

// The usual way we provide input to the side LUTs
//
typedef ZDCTriggerSimData<unsigned int, 2, 12, TSideLUTsInput> SideLUTInputsInt;

// In case we want to be able to convert from energies or other floating input
//
typedef ZDCTriggerSimData<float, 2, 12, TSideLUTsInput> SideLUTInputsFloat;

// The "usual" way we provide inputs to the combined LUT -- with unsigned
// integers
//
typedef ZDCTriggerSimData<unsigned int, 2, 4, TCombLUTInput> CombLUTInputsInt;

// In case we want to be able to convert from energies or other float
//   to the integer inputs to the combined LUT
//
//  typedef ZDCTriggerSimData<float, 2, 4> CombLUTInputsFloat;

// The combined LUT produces 3 output bits
//
typedef ZDCTriggerSimData<unsigned int, 1, 3, TCombLUTOutput> CombLUTOutput;

typedef std::shared_ptr<const ZDCTriggerSimDataBase> SimDataCPtr;
typedef std::shared_ptr<ZDCTriggerSimDataBase> SimDataPtr;
}  // namespace ZDCTriggerSim

// Base class for the ZDC trigger simulation.
//
// It is an abstract base that also provides the stack holding the intermediate
// results
//
//
class ZDCTriggerSimBase {
  typedef std::list<ZDCTriggerSim::SimDataCPtr> SimStack;

private:
  SimStack m_stack;

 protected:
  void stackClear() { m_stack.clear(); }

  void stackPush(const ZDCTriggerSim::SimDataCPtr& ptr) {
    m_stack.push_back(SimStack::value_type(ptr));
  }

  const ZDCTriggerSim::SimDataCPtr& stackTopData() const {
    return m_stack.back();
  }

  // Take the data on the "top" of the stack and use it as input, adding new
  // data to the stack
  //
  virtual void doSimStage() = 0;

 public:
  ZDCTriggerSimBase() = default;
  virtual ~ZDCTriggerSimBase() = default;

  // Every implementation of the base should ultimately produce the L1 bits
  //   possibly (usually) through recursion
  //
  virtual unsigned int simLevel1Trig(
      const ZDCTriggerSim::SimDataCPtr& data) = 0;

  SimStack::const_iterator stackBottom() const
  {
    return m_stack.begin();
  }

  SimStack::const_reverse_iterator stackTop() const {
    return m_stack.rbegin();
  }

  void dump(std::ostream& strm) const;
};

class ZDCTriggerSimCombLUT : virtual public ZDCTriggerSimBase {
  std::array<unsigned int, 256> m_combLUT{};

 protected:
  //
  // The data on the top of the stack should be the two 4 bit inputs
  //   to the combined LUT. The output is the combined LUT output
  //
  virtual void doSimStage() override;

 public:
  ZDCTriggerSimCombLUT(const std::array<unsigned int, 256>& inLUT)
      : m_combLUT(inLUT) {}

  virtual unsigned int simLevel1Trig(
      const ZDCTriggerSim::SimDataCPtr& inputBits) override {
    stackClear();
    stackPush(inputBits);

    doSimStage();
    return stackTopData()->getValueTrunc();
  }
};

class ZDCTriggerSimAllLUTs : virtual public ZDCTriggerSimBase,
                             public ZDCTriggerSimCombLUT {
  std::array<unsigned int, 4096> m_LUTA{};
  std::array<unsigned int, 4096> m_LUTC{};

 protected:
  //
  // The data on the top of the stack should be the two 12 bit inputs
  //   to each of the side LUT. The output is the two side LUT outputs.
  //
  // After we excute the side LUT, we call the CombLUT doSimStage();
  //
  virtual void doSimStage() override;

 public:
  ZDCTriggerSimAllLUTs(const std::array<unsigned int, 4096>& sideALUT,
                       const std::array<unsigned int, 4096>& sideCLUT,
                       const std::array<unsigned int, 256>& inCombLUT)
      : ZDCTriggerSimCombLUT(inCombLUT), m_LUTA(sideALUT), m_LUTC(sideCLUT) {}

  virtual unsigned int simLevel1Trig(
      const ZDCTriggerSim::SimDataCPtr& inputData) override {
    stackClear();
    stackPush(inputData);

    doSimStage();
    return stackTopData()->getValueTrunc();
  }
};

class ZDCTriggerSimModuleAmpls : virtual public ZDCTriggerSimBase,
                                 public ZDCTriggerSimAllLUTs {
 protected:
  //
  // The data on the top of the stack should be the 8 module amplitudes
  //   four for each ZDC. From these we obtain the module sums divided by four
  //
  // After we evalute the per-side sums, we call the AllLUTs doSimStage();
  //
  virtual void doSimStage() override;

 public:
  ZDCTriggerSimModuleAmpls(const std::array<unsigned int, 4096>& sideALUT,
                           const std::array<unsigned int, 4096>& sideCLUT,
                           const std::array<unsigned int, 256>& inCombLUT)
      : ZDCTriggerSimAllLUTs(sideALUT, sideCLUT, inCombLUT) {}

  virtual unsigned int simLevel1Trig(const ZDCTriggerSim::SimDataCPtr& inputData) override {
    stackClear();
    stackPush(inputData);

    doSimStage();
    return stackTopData()->getValueTrunc();
  }
};

class ZDCTriggerSimFADC : virtual public ZDCTriggerSimBase,
			  public ZDCTriggerSimModuleAmpls
{
  std::array<std::array<unsigned int, 4>, 2> m_deriv2ndThresholds;
  unsigned int m_minSampleAna{};
  unsigned int m_maxSampleAna{};
  unsigned int m_baselineDelta{};
  
  std::array<std::array<int, 4>, 2> m_maxNegDeriv2nd{};
  std::array<std::array<unsigned int, 4>, 2> m_maxADC{};
  std::array<std::array<unsigned int, 4>, 2> m_baseline{};

protected:
  //
  // The data on the top of the stack should be the FADC data for
  //   the eight ZDC channels. There should be a total of 8*3*8 samples
  //
  // After we excute 
  //
  virtual void doSimStage() override;

  std::vector<unsigned int> CalculateNeg2ndDerivatives(const std::vector<unsigned int>& samples, unsigned int step);

 public:
  ZDCTriggerSimFADC(const std::array<unsigned int, 4096>& sideALUT,
		    const std::array<unsigned int, 4096>& sideCLUT,
		    const std::array<unsigned int, 256>& inCombLUT,
		    const std::array<std::array<unsigned int, 4>, 2>& negDeriv2ndThresh,
		    unsigned int minSampleAna, unsigned int maxSampleAna, unsigned int baselineDelta) :
    ZDCTriggerSimModuleAmpls(sideALUT, sideCLUT, inCombLUT),
    m_deriv2ndThresholds(negDeriv2ndThresh),
    m_minSampleAna(minSampleAna),
    m_maxSampleAna(maxSampleAna),
    m_baselineDelta(baselineDelta)
  {}

  virtual unsigned int simLevel1Trig(const ZDCTriggerSim::SimDataCPtr& inputData) override {
    stackClear();
    stackPush(inputData);

    doSimStage();
    return stackTopData()->getValueTrunc();
  }


  const auto& getMaxNegDeriv2nds() const {return m_maxNegDeriv2nd;}
  const auto& getMaxADCs() const {return m_maxADC;}
  const auto& getBaselines() const {return m_baseline;}
};


#endif
