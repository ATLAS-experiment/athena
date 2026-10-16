/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#include "SpecRegistry.h"

#include "Utilities/binStrToHexStr.h"
#include "Utilities/hexStrToBinStr.h"

#include "CxxUtils/checker_macros.h"

#include "TClass.h"
#include "TEnv.h"
#include "THashList.h"
#include "TInterpreter.h"

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string_view>
#include <iostream>

namespace GlobalSim {

  namespace {

    using BitsGetter = std::function<std::string(const SG::AuxElement&)>;
    using BitsSetter = std::function<void(SG::AuxElement&, const std::string&)>;

    constexpr std::string_view s_prefix{"GlobalSim::"};

    /// The specs the dictionary was built with, read from the rootmap that the
    /// interpreter loads at start up, so the dictionary library itself does not
    /// need to be loaded. selection.xml lists only BitSpecs, so every GlobalSim
    /// class found is taken as one; templates and names starting with an
    /// underscore are skipped in case either is ever added there.
    const std::set<std::string>& registeredSpecs() {
      static const std::set<std::string> specs = [] {
        std::set<std::string> found;
        const TEnv* mapfile = gInterpreter->GetMapfile();
        if (mapfile == nullptr or mapfile->GetTable() == nullptr) { return found; }

        for (const TObject* record : *mapfile->GetTable()) {
          const std::string_view cls{record->GetName()};
          if (not cls.starts_with(s_prefix)) { continue; }
          const std::string_view name = cls.substr(s_prefix.size());
          if (name.empty() or name.front() == '_' or
              name.find('<') != std::string_view::npos) { continue; }
          // verify if not a namespace
          if(auto tcls = TClass::GetClass(cls.data()); tcls && (tcls->Property()&kIsNamespace)) {
              continue;
          }
          found.emplace(name);
        }
        return found;
      }();
      return specs;
    }

    /// Compile one statement and take ownership of the object it created.
    ///
    /// ProcessLine hands the address back as an integer, so the cast cannot be
    /// avoided; what makes it safe is that the type it casts to is fixed by the
    /// same string that constructed the object, a few lines below.
    template<class T>
    std::shared_ptr<T> interpret(const std::string& code, std::string& error) {
      TInterpreter::EErrorCode err = TInterpreter::kNoError;
      const Longptr_t address = gInterpreter->ProcessLine(code.c_str(), &err);
      if (address == 0 or err != TInterpreter::kNoError) {
        error = "the interpreter would not compile:\n" + code;
        return nullptr;
      }
      return std::shared_ptr<T>(reinterpret_cast<T*>(address));
    }

    /// Get the interpreter ready to compile code using one spec. Asking for
    /// the spec's TClass loads the dictionary library through the rootmap.
    /// Object.h is not part of the dictionary, as only the BitSpecs are listed
    /// in selection.xml, so the interpreter is shown it separately.
    bool prepareInterpreter(const std::string& spec, std::string& error) {
      if (TClass::GetClass(spec.c_str()) == nullptr) {
        error = "no dictionary could be loaded for " + spec;
        return false;
      }
      static const bool declared =
          gInterpreter->Declare("#include \"GlobalSimulation/Object.h\"");
      if (not declared) {
        error = "the interpreter could not read GlobalSimulation/Object.h";
      }
      return declared;
    }

    SpecEntry buildEntry(const std::string& name, std::string& error) {
      const std::string spec = "GlobalSim::" + name;
      const std::string object = "GlobalSim::Object<" + spec + ">";

      if (not prepareInterpreter(spec, error)) { return {}; }

      // The packed word crosses into the interpreter as a string of '0' and
      // '1' rather than as an integer: to_ullong() is fine for a 64 bit spec
      // but throws for anything wider, and JET1Jet is 128 bits.
      auto getter = interpret<BitsGetter>(
          "new std::function<std::string(const SG::AuxElement&)>("
          "[](const SG::AuxElement& tob) {"
          "  return " + object + "(tob).bits().to_string();"
          "});", error);
      if (not getter) { return {}; }

      auto setter = interpret<BitsSetter>(
          "new std::function<void(SG::AuxElement&, const std::string&)>("
          "[](SG::AuxElement& tob, const std::string& bits) {"
          "  " + object + " obj(tob);"
          "  obj = " + spec + "::bitset_type(bits);"
          "});", error);
      if (not setter) { return {}; }

      auto width = interpret<std::size_t>("new std::size_t(" + spec + "::width);",
                                          error);
      if (not width) { return {}; }

      // The hex conversion stays on this side: binStrToHexStr is declared under
      // src/Utilities, which is not part of the dictionary, so the interpreter
      // cannot see it. Keeping it here also leaves SpecEntry unchanged.
      return {*width,
              [g = std::move(getter)](const SG::AuxElement& tob) {
                return binStrToHexStr((*g)(tob));
              },
              [s = std::move(setter)](SG::AuxElement& tob, const std::string& token) {
                (*s)(tob, hexStrToBinStr(token));
              }};
    }

  } // anonymous namespace

  const SpecEntry* specEntry(const std::string& name, std::string& error) {
    // Reaching into the interpreter costs milliseconds and is not thread safe,
    // so each spec is built once behind a lock. Both algorithms ask in
    // initialize(), where the lock is uncontended; it is here so that the cache
    // stays correct if that ever changes. References into a std::map survive
    // later insertions, so the returned pointer stays valid.
    static std::map<std::string, SpecEntry> cache ATLAS_THREAD_SAFE; // guarded by mutex
    static std::mutex mutex;

    const std::scoped_lock lock(mutex);

    const auto cached = cache.find(name);
    if (cached != cache.end()) { return &cached->second; }

    if (not registeredSpecs().contains(name)) {
      error = "not listed in GlobalSimulation/selection.xml. Known specs are: "
              + knownSpecNames();
      return nullptr;
    }

    SpecEntry entry = buildEntry(name, error);
    if (entry.width == 0) { return nullptr; }

    return &cache.emplace(name, std::move(entry)).first->second;
  }

  std::string knownSpecNames() {
    std::string names;
    for (const auto& spec : registeredSpecs()) {
      if (not names.empty()) { names += ", "; }
      names += spec;
    }
    if (names.empty())[[unlikely]]{
     return "none -- no GlobalSim classes found in the rootmap";
    } 
    return names;
  }

}
