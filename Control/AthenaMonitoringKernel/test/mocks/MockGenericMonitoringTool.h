#ifndef AthenaMonitoringKernel_test_mocks_MockGenericMonitoringTool_h
#define AthenaMonitoringKernel_test_mocks_MockGenericMonitoringTool_h

#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include <functional>

class MockGenericMonitoringTool : public GenericMonitoringTool {
  public:
    MockGenericMonitoringTool(IInterface* parent)
      : GenericMonitoringTool("MockGenericMonitoringTool", "MonTool", parent) {}

    std::function<uint32_t()> mock_lumiBlock;
    uint32_t lumiBlock() const override {
      return mock_lumiBlock ? mock_lumiBlock() : 0;
    }

    std::function<uint32_t()> mock_runNumber;
    uint32_t runNumber() const override {
      return mock_runNumber ? mock_runNumber() : 0;
    }
};

#endif /* AthenaMonitoringKernel_test_mocks_MockGenericMonitoringTool_h */
