/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1RawDataSystems/VP1RawDataHandle_RPC_Digit.h"
#include "VP1Utils/VP1DetInfo.h"
#include "MuonDigitContainer/RpcDigit.h"
#include "MuonIdHelpers/RpcIdHelper.h"
#include "IdDictParser/IdDictParser.h"
#include "IdDict/IdDictMgr.h"
#include <iostream>
#include <stdexcept>

namespace {
const RpcIdHelper* testHelper = nullptr;
void check(bool condition, const char* message)
{
  if (!condition)
    throw std::runtime_error(message);
}
bool includes(const QStringList& lines, const QString& text)
{
  for (const auto& line : lines)
    if (line.contains(text))
      return true;
  return false;
}
}

// Supply real dictionary identifiers without an Athena job or readout geometry.
// These executable definitions replace the shared-library lookup functions.
const RpcIdHelper* VP1DetInfo::rpcIDHelper() { return testHelper; }
const MuonGM::MuonDetectorManager* VP1DetInfo::muonDetMgr() { return nullptr; }

int main()
{
  IdDictParser parser;
  parser.register_external_entity("MuonSpectrometer", "IdDictMuonSpectrometer_R.09.02.xml");
  IdDictMgr& dictionary = parser.parse("IdDictParser/ATLAS_IDS.xml");
  RpcIdHelper helper;
  check(helper.initialize_from_dictionary(dictionary) == 0, "RPC identifier dictionary");
  testHelper = &helper;
  for (int phi = 0; phi < 2; ++phi) {
    bool valid = false;
    const Identifier id = helper.channelID("BML", 1, 1, 1, 1, 1, 1, phi, 1, valid);
    check(valid, "RPC test identifier");
    for (bool side : {false, true}) {
      for (float tot : {-1.f, 0.f, 7.25f}) {
        RpcDigit digit(id, 12.5f, tot, side);
        // Click output does not require a collection or a constructed shape.
        VP1RawDataHandle_RPC_Digit handle(nullptr, &digit);
        const auto normal = handle.clicked(false);
        const auto verbose = handle.clicked(true);
        check(includes(normal, "Identifier: " + QString::fromStdString(id.getString())), "RPC raw identifier");
        check(includes(normal, phi ? "Strip view: Phi" : "Strip view: Eta"), "RPC view before drawing");
        check(includes(normal, "Time [ns]: 12.5"), "RPC time");
        check(includes(normal, "Time over threshold [ns]: " + QString::number(tot)), "RPC ToT");
        check(includes(normal, "not available") == (tot < 0), "RPC absent ToT sentinel");
        check(includes(normal, side ? "stripSide: true" : "stripSide: false"), "RPC readout side");
        check(!includes(normal, "Station name:"), "Decoded identifier is verbose only");
        for (const auto& line : normal)
          check(verbose.contains(line), "Verbose preserves normal quantities");
        for (const auto* text : {"Station name: BML", "Station eta index: 1", "Station phi index: 1",
                                 "Doublet R: 1", "Doublet Z: 1", "Doublet phi: 1", "Gas gap: 1", "Strip: 1",
                                 "Global position unavailable"})
          check(includes(verbose, text), "RPC verbose identifier and missing geometry");
        testHelper = nullptr;
        check(includes(handle.clicked(false), "Strip view: unavailable"), "Missing RPC helper");
        check(includes(handle.clicked(true), "Decoded identifier unavailable"), "Missing decoded RPC identifier");
        testHelper = &helper;
      }
    }
  }
  RpcDigit invalid(Identifier(), -5.f);
  VP1RawDataHandle_RPC_Digit invalidHandle(nullptr, &invalid);
  check(includes(invalidHandle.clicked(true), "Decoded identifier unavailable"), "Invalid identifier handled");
  check(includes(invalidHandle.clicked(false), "Time [ns]: -5"), "Negative digit time retained");
  return 0;
}
