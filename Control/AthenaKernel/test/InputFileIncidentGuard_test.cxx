/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InputFileIncidentGuard_test.cxx
 * @brief Unit tests for InputFileIncidentGuard RAII pairing guarantee.
 *
 * Tests both the guard itself and demonstrates the problem it solves:
 * manual Begin/EndInputFile firing can leave incidents unmatched when
 * code paths diverge (early return, exception, forgotten call).
 *
 * Uses the IncidentSvc (via initGaudi) with a lightweight
 * RecordingListener to observe what was fired.
 */

#undef NDEBUG

#include "AthenaKernel/InputFileIncidentGuard.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/implements.h"
#include "TestTools/initGaudi.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <print>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


/// Lightweight listener that records every incident dispatched by IncidentSvc
class RecordingListener : public implements<IIncidentListener> {
public:
   struct Record {
      std::string type;
      std::string source;
      std::string fileName;
      std::string fileGuid;
   };

   std::vector<Record> log;

   void handle(const Incident& inc) override {
      const auto* fi = dynamic_cast<const FileIncident*>(&inc);
      if (fi) {
         log.push_back({inc.type(), inc.source(), fi->fileName(), fi->fileGuid()});
      } else {
         log.push_back({inc.type(), inc.source(), "", ""});
      }
   }

   void clear() { log.clear(); }

   int count(std::string_view type) const {
      return std::count_if(log.begin(), log.end(),
         [&](const Record& r) { return r.type == type; });
   }

   /// Check that every Begin has a matching End with the same guid,
   /// in proper nesting order (no End before Begin, no unmatched at end).
   bool allPaired() const {
      std::vector<std::string> stack;
      for (const auto& r : log) {
         if (r.type.starts_with("Begin")) {
            stack.push_back(r.fileGuid);
         } else if (r.type.starts_with("End")) {
            if (stack.empty() || stack.back() != r.fileGuid) return false;
            stack.pop_back();
         }
      }
      return stack.empty();
   }
};


// ========================================================================
//  Test 1: Basic pairing — Begin on construction, End on destruction
// ========================================================================
void test_basic_pairing(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_basic_pairing... ");
   listener.clear();
   {
      auto guard = InputFileIncidentGuard::begin(svc, "Selector", "file1.pool", "guid-1");
      assert(listener.count(IncidentType::BeginInputFile) == 1);
      assert(listener.count(IncidentType::EndInputFile) == 0);
      // guard goes out of scope here
   }
   assert(listener.count(IncidentType::BeginInputFile) == 1);
   assert(listener.count(IncidentType::EndInputFile) == 1);
   assert(listener.allPaired());
   assert(listener.log[0].fileGuid == "guid-1");
   assert(listener.log[1].fileGuid == "guid-1");
   std::println ("OK");
}

// ========================================================================
//  Test 2: optional::reset() fires EndInputFile
// ========================================================================
void test_optional_reset(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_optional_reset... ");
   listener.clear();
   std::optional<InputFileIncidentGuard> guard;

   guard = InputFileIncidentGuard::begin(svc, "Sel", "file.pool", "guid-A");
   assert(listener.count(IncidentType::BeginInputFile) == 1);
   assert(listener.count(IncidentType::EndInputFile) == 0);

   guard.reset();
   assert(listener.count(IncidentType::EndInputFile) == 1);
   assert(listener.allPaired());
   std::println ("OK");
}

// ========================================================================
//  Test 3: Reassigning optional — ordering subtlety
//
//  C++ evaluates  guard = Guard::begin(svc, ..., "guid-2")  as:
//    1. Construct temporary via begin()  →  fires Begin(guid-2)
//    2. optional::operator= calls Guard::operator=(Guard&&)
//       →  fires End(guid-1), then takes ownership of guid-2
//
//  So the log order is: Begin(1), Begin(2), End(1) — NOT End(1), Begin(2).
//  There is a brief overlap where both files have had Begin fired.
//  When strict End-before-Begin ordering is needed, use explicit
//  reset-then-begin (see test 3b below).
// ========================================================================
void test_optional_reassign(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_optional_reassign... ");
   listener.clear();
   std::optional<InputFileIncidentGuard> guard;

   guard = InputFileIncidentGuard::begin(svc, "Sel", "file1.pool", "guid-1");
   assert(listener.log.size() == 1);
   assert(listener.log[0].type == IncidentType::BeginInputFile);
   assert(listener.log[0].fileGuid == "guid-1");

   // Reassign — see ordering note above
   guard = InputFileIncidentGuard::begin(svc, "Sel", "file2.pool", "guid-2");
   assert(listener.log.size() == 3);
   assert(listener.log[1].type == IncidentType::BeginInputFile);
   assert(listener.log[1].fileGuid == "guid-2");
   assert(listener.log[2].type == IncidentType::EndInputFile);
   assert(listener.log[2].fileGuid == "guid-1");

   // Final cleanup
   guard.reset();
   assert(listener.log.size() == 4);
   assert(listener.log[3].type == IncidentType::EndInputFile);
   assert(listener.log[3].fileGuid == "guid-2");

   // Both files paired (2 Begin, 2 End)
   assert(listener.count(IncidentType::BeginInputFile) == 2);
   assert(listener.count(IncidentType::EndInputFile) == 2);
   std::println ("OK");
}

// ========================================================================
//  Test 3b: Explicit reset-then-begin gives strict End(old), Begin(new)
//  This is the pattern EventSelectorAthenaPool should use when listeners
//  (e.g. MetaDataSvc) need End before Begin.
// ========================================================================
void test_explicit_reset_then_begin(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_explicit_reset_then_begin... ");
   listener.clear();
   std::optional<InputFileIncidentGuard> guard;

   guard = InputFileIncidentGuard::begin(svc, "Sel", "file1.pool", "guid-1");

   // Explicit reset first, then begin — guarantees End before Begin
   guard.reset();
   guard = InputFileIncidentGuard::begin(svc, "Sel", "file2.pool", "guid-2");

   assert(listener.log.size() == 3);
   assert(listener.log[0].type == IncidentType::BeginInputFile);
   assert(listener.log[0].fileGuid == "guid-1");
   assert(listener.log[1].type == IncidentType::EndInputFile);    // End(old) first
   assert(listener.log[1].fileGuid == "guid-1");
   assert(listener.log[2].type == IncidentType::BeginInputFile);  // then Begin(new)
   assert(listener.log[2].fileGuid == "guid-2");

   guard.reset();
   assert(listener.count(IncidentType::BeginInputFile) == 2);
   assert(listener.count(IncidentType::EndInputFile) == 2);
   assert(listener.allPaired());
   std::println ("OK");
}

// ========================================================================
//  Test 3c: transition — same as 3b but in one call
// ========================================================================
void test_begin_replace(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_begin_replace... ");
   listener.clear();
   std::optional<InputFileIncidentGuard> guard;

   guard = InputFileIncidentGuard::begin(svc, "Sel", "file1.pool", "guid-1");

   // transition does reset+begin atomically
   InputFileIncidentGuard::transition(guard, svc, "Sel", "file2.pool", "guid-2");

   assert(listener.log.size() == 3);
   assert(listener.log[0].type == IncidentType::BeginInputFile);
   assert(listener.log[0].fileGuid == "guid-1");
   assert(listener.log[1].type == IncidentType::EndInputFile);    // End(old) first
   assert(listener.log[1].fileGuid == "guid-1");
   assert(listener.log[2].type == IncidentType::BeginInputFile);  // then Begin(new)
   assert(listener.log[2].fileGuid == "guid-2");

   guard.reset();
   assert(listener.allPaired());
   std::println ("OK");
}

// ========================================================================
//  Test 4: Move disarms the source — only the destination fires End
// ========================================================================
void test_move_disarms(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_move_disarms... ");
   listener.clear();
   {
      auto guard1 = InputFileIncidentGuard::begin(svc, "Sel", "file.pool", "guid-M");
      assert(listener.count(IncidentType::BeginInputFile) == 1);

      auto guard2 = std::move(guard1);
      // guard1 is disarmed, guard2 owns the incident
   }
   // Both destroyed, but only guard2 should have fired End
   assert(listener.count(IncidentType::EndInputFile) == 1);
   assert(listener.allPaired());
   std::println ("OK");
}

// ========================================================================
//  Test 5: Exception safety — End fires even when exception is thrown
// ========================================================================
void test_exception_safety(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_exception_safety... ");
   listener.clear();
   try {
      auto guard = InputFileIncidentGuard::begin(svc, "Sel", "file.pool", "guid-E");
      assert(listener.count(IncidentType::BeginInputFile) == 1);
      throw std::runtime_error("simulated error during event processing");
   } catch (const std::runtime_error&) {
      // expected
   }
   // End must have fired during stack unwinding
   assert(listener.count(IncidentType::EndInputFile) == 1);
   assert(listener.allPaired());
   std::println ("OK");
}

// ========================================================================
//  Test 5b: Custom endFileName (eventless files)
// ========================================================================
void test_custom_end_filename(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_custom_end_filename... ");
   listener.clear();
   {
      auto guard = InputFileIncidentGuard::begin(svc, "Sel",
                       "myfile.pool", {},
                       "eventless:myfile.pool");
      assert(listener.log.size() == 1);
      assert(listener.log[0].type == IncidentType::BeginInputFile);
      assert(listener.log[0].fileName == "myfile.pool");
   }
   assert(listener.log.size() == 2);
   assert(listener.log[1].type == IncidentType::EndInputFile);
   assert(listener.log[1].fileName == "eventless:myfile.pool");
   assert(listener.allPaired());
   std::println ("OK");
}

// ========================================================================
//  Test 5c: Custom incident types (e.g. MemFile)
// ========================================================================
void test_custom_incident_types(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_custom_incident_types... ");
   listener.clear();
   {
      auto guard = InputFileIncidentGuard::begin(svc, "SharedIO",
                       "SHM[NUM=1]", {},
                       /*endFileName=*/"SHM[NUM=1]",
                       "BeginInputMemFile", "EndInputMemFile");
      assert(listener.log.size() == 1);
      assert(listener.log[0].type == "BeginInputMemFile");
      assert(listener.log[0].fileName == "SHM[NUM=1]");
   }
   assert(listener.log.size() == 2);
   assert(listener.log[1].type == "EndInputMemFile");
   assert(listener.log[1].fileName == "SHM[NUM=1]");
   assert(listener.allPaired());
   std::println ("OK");
}

// ========================================================================
//  Test 6: Double reset is safe — resetting empty optional is a no-op
// ========================================================================
void test_double_reset_safe(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_double_reset_safe... ");
   listener.clear();
   std::optional<InputFileIncidentGuard> guard;

   guard = InputFileIncidentGuard::begin(svc, "Sel", "file.pool", "guid-D");
   guard.reset();
   assert(listener.count(IncidentType::EndInputFile) == 1);

   guard.reset();  // should be a no-op
   assert(listener.count(IncidentType::EndInputFile) == 1);  // still 1
   assert(listener.allPaired());
   std::println ("OK");
}

// ========================================================================
//  Test 7: Demonstrates the BUG — manual firing misses End on early return
// ========================================================================
void test_manual_firing_bug(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_manual_firing_bug... ");
   listener.clear();

   // Manual Begin/End firing: if we return early, End never fires.
   auto simulateManualFiring = [&](bool earlyReturn) {
      svc.fireIncident(FileIncident("Sel", IncidentType::BeginInputFile, "file.pool", "guid-B"));
      // ... process events ...
      if (earlyReturn) {
         return;  // BUG: EndInputFile never fires
      }
      svc.fireIncident(FileIncident("Sel", IncidentType::EndInputFile, "FID:guid-B", "guid-B"));
   };

   // Normal case: paired
   simulateManualFiring(false);
   assert(listener.count(IncidentType::BeginInputFile) == 1);
   assert(listener.count(IncidentType::EndInputFile) == 1);
   assert(listener.allPaired());

   // Early return case: Begin without End — the bug
   listener.clear();
   simulateManualFiring(true);
   assert(listener.count(IncidentType::BeginInputFile) == 1);
   assert(listener.count(IncidentType::EndInputFile) == 0);  // BUG: unmatched
   assert(!listener.allPaired());

   std::println ("OK (bug demonstrated)");
}

// ========================================================================
//  Test 8: Guard FIXES the bug — End fires regardless of early return
// ========================================================================
void test_guard_fixes_bug(IIncidentSvc& svc, RecordingListener& listener) {
   std::println ("test_guard_fixes_bug... ");
   listener.clear();

   // Same scenario as test 7, but using the guard.
   // The guard fires End on scope exit regardless of how we leave.
   auto simulateGuardedFiring = [&](bool earlyReturn) {
      std::optional<InputFileIncidentGuard> guard;
      guard = InputFileIncidentGuard::begin(svc, "Sel", "file.pool", "guid-G");
      // ... process events ...
      if (earlyReturn) {
         return;  // guard destroyed here — fires EndInputFile
      }
      // guard destroyed here — fires EndInputFile
   };

   // Normal case
   simulateGuardedFiring(false);
   assert(listener.allPaired());

   // Early return case: STILL paired
   listener.clear();
   simulateGuardedFiring(true);
   assert(listener.count(IncidentType::BeginInputFile) == 1);
   assert(listener.count(IncidentType::EndInputFile) == 1);  // FIXED: End always fires
   assert(listener.allPaired());

   std::println ("OK (bug fixed)");
}


// ========================================================================
int main() {
  std::println ("*** InputFileIncidentGuard_test BEGIN ***");

   ISvcLocator* svcloc = nullptr;
   if (!Athena_test::initGaudi("", svcloc)) {
      std::println (std::cerr, "This test cannot be run");
      return 1;
   }

   SmartIF<IIncidentSvc> incSvc{svcloc->service<IIncidentSvc>("IncidentSvc")};
   assert(incSvc);

   // Register for all incidents (empty type string) so custom types
   // like "BeginInputMemFile" are also captured.
   RecordingListener listener;
   incSvc->addListener(&listener, "");

   test_basic_pairing(*incSvc, listener);
   test_optional_reset(*incSvc, listener);
   test_optional_reassign(*incSvc, listener);
   test_explicit_reset_then_begin(*incSvc, listener);
   test_begin_replace(*incSvc, listener);
   test_move_disarms(*incSvc, listener);
   test_exception_safety(*incSvc, listener);
   test_custom_end_filename(*incSvc, listener);
   test_custom_incident_types(*incSvc, listener);
   test_double_reset_safe(*incSvc, listener);
   test_manual_firing_bug(*incSvc, listener);
   test_guard_fixes_bug(*incSvc, listener);

   incSvc->removeListener(&listener);

   std::println ("*** InputFileIncidentGuard_test END — all passed ***");
   return 0;
}
