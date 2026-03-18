# NavigationDAODTesterAlgv2: DAOD-Level Navigation Conversion Validation

## Summary

`NavigationDAODTesterAlgv2` is an independent validation algorithm that verifies trigger object matching on Derived Analysis Object Data (DAOD) files produced from Run2 data with converted navigation. It compares matching results from two independent sources:

1. **Run2 source (R2)**: Pre-stored `TrigMatch_*` containers created during DAOD production using original Run2 navigation
2. **Run3 source (R3)**: Real-time matching using converted Run2→Run3 navigation

This validation ensures that physics analyses using converted navigation will retrieve equivalent trigger-matched objects as they would with the original Run2 approach.

---

## 1. Context: The DAOD Trigger Matching Pipeline

### 1.1 What Happens During DAOD Production

When a DAOD (e.g., DAOD_PHYS) is produced from Run2 AOD data, the **DerivationFramework TriggerMatchingTool** pre-computes trigger-to-offline matching:

```
Run2 AOD
    │
    ▼
┌─────────────────────────────────────────────┐
│  DerivationFramework::TriggerMatchingTool   │
│  • Uses IParticleRetrievalTool (Run2)       │
│  • For each chain, finds all offline        │
│    particles that match trigger objects     │
│  • Stores results in TrigMatch_* containers │
└─────────────────────────────────────────────┘
    │
    ▼
DAOD_PHYS with:
  • HLTNav_R2ToR3Summary (converted navigation)
  • TrigMatch_HLT_mu24 (pre-matched muons)
  • TrigMatch_HLT_e26_lhtight (pre-matched electrons)
  • TrigMatch_HLT_g200_etcut (pre-matched photons)
  • TrigMatch_HLT_tau25_medium1_tracktwo (pre-matched taus)
  • ... etc for all supported triggers
```

### 1.2 Why DAOD-Level Validation is Needed

The `NavigationDAODTesterAlgv2` addresses a critical question:

> **Do physics analyses get the same trigger-matched objects whether they use:**
> - The pre-stored `TrigMatch_*` containers (based on Run2 navigation), OR
> - Real-time matching using the converted Run3 navigation?

This is distinct from `NavigationTesterAlg` (covered in the other tutorial) which compares particle retrieval at AOD level. Here we validate the complete matching workflow as used by physics analyzers.

---

## 2. Key Files

| File | Purpose |
|------|---------|
| [NavigationDAODTesterAlgv2.h](../src/NavigationDAODTesterAlgv2.h) | Algorithm header with tool handles and counters |
| [NavigationDAODTesterAlgv2.cxx](../src/NavigationDAODTesterAlgv2.cxx) | Main validation implementation |
| [testTrigR2ToR3NavDAOD.py](../share/testTrigR2ToR3NavDAOD.py) | Test job configuration |

### Related Tools

| Tool | File | Role |
|------|------|------|
| `R3MatchingTool` | [TriggerMatchingTool/R3MatchingTool.h](../../TrigAnalysis/TriggerMatchingTool/TriggerMatchingTool/R3MatchingTool.h) | Matches offline particles to trigger objects using Run3 navigation (supports `IncludeSubfeatures`) |
| `MatchFromCompositeTool` | [TriggerMatchingTool/MatchFromCompositeTool.h](../../TrigAnalysis/TriggerMatchingTool/TriggerMatchingTool/MatchFromCompositeTool.h) | Matches using pre-stored `TrigMatch_*` containers |
| `TriggerMatchingTool` (DF) | [DerivationFrameworkTrigger/TriggerMatchingTool.cxx](../../../PhysicsAnalysis/DerivationFramework/DerivationFrameworkTrigger/src/TriggerMatchingTool.cxx) | Creates `TrigMatch_*` containers during DAOD production |
| `FeatureRequestDescriptor` | [TrigAnalysisHelpers/FeatureRequestDescriptor.h](../../TrigAnalysis/TrigAnalysisHelpers/TrigAnalysisHelpers/FeatureRequestDescriptor.h) | Configures feature retrieval from TrigDecisionTool |

---

## 3. Architecture

### 3.1 Data Flow

```
DAOD_PHYS Input File
        │
        ├── HLTNav_R2ToR3Summary  ─────────────────────┐
        │   (converted navigation)                     │
        │                                              ▼
        │                              ┌──────────────────────────────┐
        │                              │  TrigDecisionTool (R3 mode)  │
        │                              │  ↓                           │
        │                              │  R3MatchingTool              │
        │                              │  • buildCombinations()       │
        │                              │  • DR-based matching         │
        │                              └──────────────────────────────┘
        │                                               │
        │                                               ▼
        │                                          passR3: bool
        │
        ├── TrigMatch_HLT_2mu10_nomucomb ─────────────┐
        │   (pre-computed combinations)               │
        │                                             ▼
        │                              ┌──────────────────────────────┐
        │                              │  MatchFromCompositeTool      │
        │                              │  • Reads TrigMatchedObjects  │
        │                              │  • DR-based matching         │
        │                              └──────────────────────────────┘
        │                                              │
        │                                              ▼
        │                                         passR2: bool
        │
        ├── Muons, Electrons, TauJets, Photons ─┐
        │   (offline containers)                │
        │                                       ▼
        │                   ┌────────────────────────────────────┐
        │                   │  NestedUniqueCombinationGenerator  │
        │                   │  • Generate all N-object combos    │
        │                   │  • Based on chain multiplicity     │
        │                   └────────────────────────────────────┘
        │                                  │
        │                                  ▼
        │                          For each offline combo:
        │                          Compare passR2 vs passR3
        │
        └─────────────────────────────────────────────────────┘
                                   │
                                   ▼
                    ┌────────────────────────────────┐
                    │  NavigationDAODTesterAlgv2     │
                    │  • If passR2 && !passR3: ERROR │
                    │  • If passR3 && !passR2: OK    │
                    │  • If passR2 == passR3: OK     │
                    └────────────────────────────────┘
```

### 3.2 The Validation Logic

The key insight is the **asymmetric comparison**:

| passR2 | passR3 | Interpretation |
|--------|--------|----------------|
| true | true | ✅ Both agree - match found |
| false | false | ✅ Both agree - no match |
| false | true | ✅ Expected: R3 finds more combinations (by design, but rare after R2 matching fix) |
| true | false | ❌ **CRITICAL ERROR**: R3 is missing trigger information |

**Why R3 can find more matches than R2:**

The Run2 `IParticleRetrievalTool` returns only the **highest-pT object per TriggerElement**, while Run3 navigation pools **all features** and builds combinations based on chain multiplicities. This architectural difference means R3 may form valid combinations that R2 would miss.

> **Note (Adaptive R2 matching):** The R2 matching strategy adapts based on TrigMatch container structure:
> - **Combo size = 1** (e.g., `HLT_2mu10_nomucomb`): Uses **per-particle** matching — each particle is checked individually against the TrigMatch container.
> - **Combo size > 1** (e.g., `HLT_e17_lhloose_nod0_2e9_lhloose_nod0` with size=3): Uses **vector-based** matching — the full offline combination must exist as a stored combo.
>
> This adaptive approach is necessary because the DerivationFramework `TriggerMatchingTool` stores different formats depending on the chain: some chains produce individual particle entries, while multi-leg chains produce full N-particle combination entries. Using per-particle matching on multi-particle combos would be too permissive (allowing the same particle in multiple legs), while using vector matching on single-particle entries would always fail for multi-object chains.

---

## 4. The Matching Tools in Detail

### 4.1 R3MatchingTool (Converted Navigation)

**File:** [TriggerMatchingTool/Root/R3MatchingTool.cxx](../../TrigAnalysis/TriggerMatchingTool/Root/R3MatchingTool.cxx)

**How it works:**

```cpp
bool R3MatchingTool::match(
    const std::vector<const xAOD::IParticle*>& recoObjects,
    const std::string& chain, ...) const
{
    // 1. Get all features from converted navigation
    VecLinkInfo_t features = m_trigDecTool->features<xAOD::IParticleContainer>(chainName);

    // 2. Build combinations using chain multiplicities
    TrigCompositeUtils::Combinations combinations = TrigCompositeUtils::buildCombinations(
        chainName, features, chainInfo.first, TrigCompositeUtils::FilterType::UniqueObjects);

    // 3. For each trigger combination, try all permutations of offline objects
    for (const VecLinkInfo_t& combination : combinations) {
        std::vector<std::size_t> onlineIndices(combination.size());
        std::iota(onlineIndices.begin(), onlineIndices.end(), 0);
        do {
            bool match = true;
            for (std::size_t recoIdx = 0; recoIdx < recoObjects.size(); ++recoIdx) {
                if (!matchObjects(recoObjects[recoIdx], combination[onlineIndices[recoIdx]], ...))
                    match = false;
            }
            if (match) return true;
        } while (std::next_permutation(onlineIndices.begin(), onlineIndices.end()));
    }
    return false;
}
```

**Key characteristics:**
- Uses `TrigDecisionTool::features<>()` to retrieve trigger objects from converted navigation
- Builds combinations with `TrigCompositeUtils::buildCombinations()`
- Matches based on ΔR threshold (default 0.1)
- Handles CaloCluster-to-EGamma matching for etcut chains

### 4.2 MatchFromCompositeTool (Pre-Stored Combinations)

**File:** [TriggerMatchingTool/Root/MatchFromCompositeTool.cxx](../../TrigAnalysis/TriggerMatchingTool/Root/MatchFromCompositeTool.cxx)

**How it works:**

```cpp
bool MatchFromCompositeTool::match(
    const std::vector<const xAOD::IParticle*>& recoObjects,
    const std::string& chain, ...) const
{
    // 1. Retrieve pre-stored container (created during DAOD production)
    std::string containerName = m_inputPrefix + chain;  // "TrigMatch_HLT_2mu10"
    std::replace(containerName.begin(), containerName.end(), '.', '_');
    const xAOD::TrigCompositeContainer* composites = evtStore()->retrieve(containerName);

    // 2. Each composite holds one trigger combination's matched offline objects
    for (const xAOD::TrigComposite* composite : *composites) {
        const vecLink_t<xAOD::IParticleContainer>& onlineLinks = accMatched(*composite);
        if (testCombination(onlineLinks, recoObjects))
            return true;
    }
    return false;
}

bool MatchFromCompositeTool::testCombination(...) const {
    // Each offline particle must find a unique online match
    for (const xAOD::IParticle* offlinePart : offline) {
        for (auto itr = online.begin(); itr != online.end(); ++itr) {
            if (areTheSame(*offlinePart, **itr)) {
                online.erase(itr);  // Don't reuse this online particle
                isMatched = true;
                break;
            }
        }
        if (!isMatched) return false;
    }
    return true;
}
```

**Key characteristics:**
- Reads `TrigMatch_*` containers created by DerivationFramework
- Uses `TrigMatchedObjects` accessor to get pre-linked offline particles
- Supports three comparison modes:
  - Pointer equality (default)
  - Shallow copy comparison (`MatchShallow=true`)
  - ΔR threshold matching (`DRThreshold > 0`)

> **Important:** The `match(vector)` overload expects all particles to be present in a **single** composite entry. The tester uses an **adaptive strategy** based on TrigMatch combo entry size:
> - **Combo size = 1** (e.g., `HLT_2mu10_nomucomb`): Calls `match()` per-particle, since each entry stores one individual matched particle.
> - **Combo size > 1** (e.g., `HLT_e17_lhloose_nod0_2e9_lhloose_nod0` with size=3): Calls `match(vector)` with the full offline combination, since entries store complete N-particle combos.
>
> See Section 5.2, Step 4 for the actual code used.

### 4.3 DerivationFramework TriggerMatchingTool (DAOD Producer)

**File:** [DerivationFrameworkTrigger/TriggerMatchingTool.cxx](../../../PhysicsAnalysis/DerivationFramework/DerivationFrameworkTrigger/src/TriggerMatchingTool.cxx)

This tool runs during DAOD production and creates the `TrigMatch_*` containers:

```cpp
StatusCode TriggerMatchingTool::addBranches(const EventContext& ctx) const {
    for (const std::string& chain : m_chainNames) {
        // Create output container
        xAOD::TrigCompositeContainer* container;
        createOutputContainer(container, chain);  // "TrigMatch_HLT_2mu10"

        // Get trigger combinations using IParticleRetrievalTool (Run2 navigation!)
        std::vector<particleVec_t> onlineCombinations;
        m_trigParticleTool->retrieveParticles(onlineCombinations, chain, m_rerun);

        // For each trigger combination, find matching offline particles
        for (const particleVec_t& combination : onlineCombinations) {
            std::vector<particleRange_t> matchCandidates;
            for (const xAOD::IParticle* part : combination) {
                matchCandidates.push_back(getCandidateMatchesFor(part, ...));
            }
            // Get all distinct offline combinations matching this trigger combo
            auto offlineCombinations = getAllDistinctCombinations(matchCandidates);

            // Store each valid offline combination
            for (const particleVec_t& offlineCombo : offlineCombinations) {
                xAOD::TrigComposite* composite = new xAOD::TrigComposite();
                container->push_back(composite);
                dec_links(*composite) = makeElementLinks(offlineCombo);
            }
        }
    }
}
```

**Key insight:** The `TrigMatch_*` containers store **offline particles that match trigger objects**, not the trigger objects themselves. The comparison in `MatchFromCompositeTool` checks if the user's offline particles are among those pre-matched.

---

## 5. The Algorithm: Step by Step

### 5.1 Initialization

**File Reference:** [NavigationDAODTesterAlgv2.cxx:41-71](../src/NavigationDAODTesterAlgv2.cxx#L41-L71)

```cpp
StatusCode NavigationDAODTesterAlgv2::initialize() {
    ATH_CHECK(m_containerKey.initialize());
    ATH_CHECK(m_trigcompositecontainer.initialize());
    ATH_CHECK(m_tdt.retrieve());
    ATH_CHECK(m_matchingTool.retrieve());       // R3MatchingTool
    ATH_CHECK(m_matchFromCompositeTool.retrieve()); // MatchFromCompositeTool

    if (m_chains.empty()) {
        ATH_MSG_WARNING("No chains provided, algorithm will be no-op");
    }
    // Initialize counters...
    return StatusCode::SUCCESS;
}
```

### 5.2 Event Processing

**File Reference:** [NavigationDAODTesterAlgv2.cxx:73-283](../src/NavigationDAODTesterAlgv2.cxx#L73-L283)

#### Step 1: Retrieve Offline Containers

The algorithm retrieves all supported offline particle containers:

```cpp
SG::ReadHandle<xAOD::IParticleContainer> particles_muons{"Muons", ctx};
SG::ReadHandle<xAOD::IParticleContainer> particles_electrons{"Electrons", ctx};
SG::ReadHandle<xAOD::IParticleContainer> particles_taus{"TauJets", ctx};
SG::ReadHandle<xAOD::IParticleContainer> particles_photons{"Photons", ctx};

std::map<std::string, SG::ReadHandle<xAOD::IParticleContainer>*> read_handles;
read_handles["e"] = &particles_electrons;
read_handles["mu"] = &particles_muons;
read_handles["tau"] = &particles_taus;
read_handles["g"] = &particles_photons;
```

**Supported object types:**

| Signature | Container | Example Chains |
|-----------|-----------|----------------|
| `e` | `Electrons` | `HLT_e26_lhtight_ivarloose`, `HLT_2e12_lhvloose_L12EM10VH` |
| `mu` | `Muons` | `HLT_mu26_ivarmedium`, `HLT_2mu14` |
| `g` | `Photons` | `HLT_g200_etcut`, `HLT_2g20_tight` |
| `tau` | `TauJets` | `HLT_tau25_medium1_tracktwo`, `HLT_tau35_medium1_tracktwo` |

#### Step 2: Parse Chain Structure and Filter

```cpp
for (const std::string& chain : m_chains) {
    if (!m_tdt->isPassed(chain)) continue;

    // Get chain leg structure
    auto ChainMultiplicity = ChainNameParser::multiplicities(chain);
    auto ChainNameParseSignature = ChainNameParser::signatures(chain);
    // Example: "HLT_2mu10" → multiplicities=[2], signatures=["mu"]
    // Example: "HLT_e7_lhmedium_nod0_mu24" → multiplicities=[1,1], signatures=["e","mu"]
    // Example: "HLT_g200_etcut" → multiplicities=[1], signatures=["g"]
    // Example: "HLT_tau25_medium1_tracktwo" → multiplicities=[1], signatures=["tau"]

    // Filter to only process chains with supported signatures (e, mu, g, tau)
    bool hasSupported = false;
    for (const std::string& sig : ChainNameParseSignature) {
        if (read_handles.find(sig) != read_handles.end()) {
            hasSupported = true;
            break;
        }
    }
    if (!hasSupported) {
        ATH_MSG_DEBUG("Chain " << chain << " has no supported signatures, skipping");
        continue;
    }
```

The filter dynamically checks if the chain contains any supported signature type (`e`, `mu`, `g`, `tau`). Chains with only unsupported signatures (like `j` for jets) are skipped.

#### Step 3: Generate Offline Combinations

Using `NestedUniqueCombinationGenerator` to iterate all possible combinations:

```cpp
HLT::NestedUniqueCombinationGenerator nucg;
for (size_t i = 0; i < ChainNameParseSignature.size(); i++) {
    const std::string& sig = ChainNameParseSignature[i];
    // For "HLT_2mu10" with 5 muons: add({5, 2}) → C(5,2) = 10 combinations
    nucg.add({(*read_handles[sig])->size(), ChainMultiplicity[i]});
}

do {
    const std::vector<size_t> combination = nucg();  // e.g., [0, 2] → muon indices
    ++nucg;

    // Build particle vector from indices
    std::vector<const xAOD::IParticle*> particles;
    for (size_t idx : combination) {
        particles.push_back(container->at(idx));
    }
    // ...
} while (nucg);
```

#### Step 4: Determine R2 Matching Strategy and Compare

Before the combination loop, the TrigMatch container is retrieved and inspected to choose the R2 matching strategy:

```cpp
// Retrieve TrigMatch container and determine matching strategy
std::string containerName = m_inputPrefix + chain;
const xAOD::TrigCompositeContainer* composites(nullptr);
evtStore()->retrieve(composites, containerName);

size_t trigMatchComboSize = 0;
if (!composites->empty()) {
    trigMatchComboSize = accMatched(*composites->at(0)).size();
}
const bool usePerParticleR2 = (trigMatchComboSize <= 1);
// Output: "TrigMatch combo size=1 → using per-particle R2 matching"
//     or: "TrigMatch combo size=3 → using vector-based R2 matching"
```

Then in the combination loop:

```cpp
// R3: full combination matching via buildCombinations
bool passR3 = m_matchingTool->match(particles, chain, 0.1, false);

// R2: adaptive matching based on TrigMatch combo size
bool passR2;
if (usePerParticleR2) {
    // TrigMatch stores individual particles (e.g., HLT_2mu10_nomucomb, combo size=1)
    passR2 = true;
    for (const auto* p : particles) {
        if (!m_matchFromCompositeTool->match(*p, chain, 0.1, false)) {
            passR2 = false;
            break;
        }
    }
} else {
    // TrigMatch stores full N-particle combos (e.g., HLT_e17_lhloose_nod0_2e9_lhloose_nod0, combo size=3)
    passR2 = m_matchFromCompositeTool->match(particles, chain, 0.1, false);
}

if (passR2 && !passR3) {
    // CRITICAL ERROR: R3 conversion is missing trigger information
    ATH_MSG_ERROR("R2 passes but R3 fails for chain " << chain
                  << " — R3 conversion may be missing trigger information");
    isR2R3different = true;
}
else if (!passR2 && passR3) {
    // Expected but rare: R3 finds more combinations due to architectural differences
    ATH_MSG_DEBUG("R3 passes but R2 fails (expected) for chain " << chain);
}
else {
    // Both agree
    ++nCombinationsTested;
    ATH_MSG_DEBUG("R2/R3 match for chain " << chain
                  << " passR2: " << passR2 << " passR3: " << passR3);
}
```

#### Step 5: Print Subfeatures (Optional)

When `PrintSubfeatures = True`, the algorithm prints detailed feature information for each passing chain:

```cpp
if (m_printSubfeatures) {
    // Print primary features per leg
    for (std::size_t leg = 0; leg < nLegs; ++leg) {
        Trig::FeatureRequestDescriptor frd;
        frd.setChainGroup(chain);
        frd.setRestrictRequestToLeg(static_cast<int>(leg));
        frd.setFeatureCollectionMode(TrigDefs::allFeaturesOfType);
        auto features = m_tdt->features<xAOD::IParticleContainer>(frd);
        // Print each feature...
    }

    // Print subfeatures per leg
    for (std::size_t leg = 0; leg < nLegs; ++leg) {
        Trig::FeatureRequestDescriptor subFrd;
        subFrd.setChainGroup(chain);
        subFrd.setRestrictRequestToLeg(static_cast<int>(leg));
        subFrd.setLinkName("subfeature");
        subFrd.setFeatureCollectionMode(TrigDefs::allFeaturesOfType);
        auto subfeatures = m_tdt->features<xAOD::IParticleContainer>(subFrd);
        // Print each subfeature...
    }
}
```

#### Step 6: Diagnostic Output on Failure

When a discrepancy is found, the algorithm dumps detailed information:

```cpp
if (isR2R3different) {
    // Thread-safe counter update
    {
        std::lock_guard<std::mutex> lock(m_failingChainsMutex);
        m_failingChains[chain]++;
    }

    // Dump TrigMatch container content
    for (const xAOD::TrigComposite* combination : *composites) {
        for (const ElementLink<xAOD::IParticleContainer>& f : accMatched(*combination)) {
            ATH_MSG_ERROR("R2 object: pt: " << (*f)->pt() << " eta: " << (*f)->eta());
        }
    }

    // Dump converted navigation features per leg
    for (std::size_t leg = 0; leg < nLegs; ++leg) {
        Trig::FeatureRequestDescriptor frd;
        frd.setChainGroup(chain);
        frd.setRestrictRequestToLeg(leg);
        auto features = m_tdt->features<xAOD::IParticleContainer>(frd);
        for (const auto& f : features) {
            ATH_MSG_ERROR("R3 Leg " << leg << ": pt:" << (*f.link)->pt());
        }
    }

    // Check for subfeatures (lower-pT objects from Run2→Run3 conversion)
    Trig::FeatureRequestDescriptor subFrd;
    subFrd.setLinkName("subfeature");
    auto subfeatures = m_tdt->features<xAOD::IParticleContainer>(subFrd);
    ATH_MSG_ERROR("Found " << subfeatures.size() << " subfeatures");
}
```

**Note:** The `FeatureRequestDescriptor` is imported from `TrigAnalysisHelpers/FeatureRequestDescriptor.h`.

---

## 6. Configuration

### 6.1 Test Script: testTrigR2ToR3NavDAOD.py

**File:** [testTrigR2ToR3NavDAOD.py](../share/testTrigR2ToR3NavDAOD.py)

```python
#!/usr/bin/env python

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Input DAOD file (must have HLTNav_R2ToR3Summary and TrigMatch_* containers)
    flags.Input.Files = ["DAOD_PHYS.DAOD.NEW.pool.root"]
    flags.Exec.MaxEvents = 100
    flags.fillFromArgs()
    flags.lock()

    # Standard configuration
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    # Configure TrigDecisionTool for Run3 navigation
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = cfg.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    tdt.HLTSummary = "HLTNav_Summary_DAODSlimmed"
    tdt.NavigationFormat = "TrigComposite"  # Use Run3 format

    # R3 matching tool (uses converted navigation)
    r3MatchingTool = CompFactory.Trig.R3MatchingTool("R3MatchingTool")
    r3MatchingTool.TrigDecisionTool = tdt
    r3MatchingTool.IncludeSubfeatures = True  # Include lower-pT objects

    # R2 matching tool (uses TrigMatch_* containers)
    matchFromCompositeTool = CompFactory.Trig.MatchFromCompositeTool("MatchFromCompositeTool")
    matchFromCompositeTool.InputPrefix = "TrigMatch_"
    matchFromCompositeTool.DRThreshold = 0.1  # CRITICAL: Enable DR matching

    # Select triggers to test - examples for different object types:
    # Muon chains:
    list_triggers = ['HLT_2mu10_nomucomb']
    # Electron chains:
    # list_triggers = ['HLT_e26_lhtight_ivarloose', 'HLT_2e12_lhvloose_L12EM10VH']
    # Photon chains:
    # list_triggers = ['HLT_g200_etcut', 'HLT_2g20_tight']
    # Tau chains:
    # list_triggers = ['HLT_tau25_medium1_tracktwo', 'HLT_tau35_medium1_tracktwo']
    # Combined chains:
    # list_triggers = ['HLT_e17_lhmedium_nod0_tau80_medium1_tracktwo']
    #
    # Or use TriggerListsHelper for comprehensive testing:
    # from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    # triggerListsHelper = TriggerListsHelper(flags)
    # list_triggers = triggerListsHelper.Run2TriggerNamesNoTau + triggerListsHelper.Run2TriggerNamesTau

    # Create the tester algorithm
    checker = CompFactory.Trig.NavigationDAODTesterAlgv2(
        TrigDecisionTool = tdt,
        R3MatchingTool = r3MatchingTool,
        MatchFromCompositeTool = matchFromCompositeTool,
        ContainerName = "Muons",
        PrintSubfeatures = True  # Print subfeatures for investigation
    )
    checker.Chains = list_triggers
    cfg.addEventAlgo(checker)

    cfg.run()
```

### 6.2 Key Configuration Options

| Property | Default | Description |
|----------|---------|-------------|
| `Chains` | `[]` | Trigger chains to test |
| `ContainerName` | `"Muons"` | Default offline container (used for interface compatibility) |
| `FailOnDifference` | `false` | Return FAILURE status if R2 != R3 |
| `VerifyCombinationsSize` | `true` | Check if combination sizes match |
| `VerifyCombinationsContent` | `true` | Check if combinations point to same objects |
| `InputPrefix` | `"TrigMatch_"` | Prefix for TrigMatch containers |
| `PrintSubfeatures` | `false` | Print subfeatures (lower-pT objects from R2→R3 conversion) for each passing chain |

### 6.3 Tool Configuration Details

**R3MatchingTool:**
```python
r3MatchingTool.IncludeSubfeatures = True  # Include all converted features (subfeatures)
r3MatchingTool.ScoringTool = "Trig::DRScoringTool"  # ΔR-based matching
```

**MatchFromCompositeTool:**
```python
matchFromCompositeTool.DRThreshold = 0.1  # Use ΔR matching (important!)
# Alternative modes:
matchFromCompositeTool.DRThreshold = -1  # Pointer equality (stricter)
matchFromCompositeTool.MatchShallow = True  # Shallow copy comparison
```

**NavigationDAODTesterAlgv2:**
```python
checker.PrintSubfeatures = True  # Print primary features and subfeatures per leg
```

---

## 7. The TrigMatch Container Structure

### 7.1 Container Naming

For each trigger chain, a container is created:
- Chain: `HLT_2mu10_nomucomb` → Container: `TrigMatch_HLT_2mu10_nomucomb`
- Chain: `HLT_e26_lhtight_nod0` → Container: `TrigMatch_HLT_e26_lhtight_nod0`

Periods (`.`) are replaced with underscores (`_`) for valid container names.

### 7.2 Container Content

The TrigMatch container format **varies by chain**. The DerivationFramework `TriggerMatchingTool` stores entries whose size depends on how it builds trigger combinations.

#### Format A: Individual particles (combo size = 1)

For some chains, each `xAOD::TrigComposite` stores **one individually matched offline particle**:

```
TrigMatch_HLT_2mu10_nomucomb (xAOD::TrigCompositeContainer)
    │
    ├── TrigComposite[0]
    │   └── TrigMatchedObjects: [Muons#1]   ← one matched muon (pT=14.0 GeV)
    │
    └── TrigComposite[1]
        └── TrigMatchedObjects: [Muons#0]   ← one matched muon (pT=22.2 GeV)
```

For these chains, an offline combination passes R2 if **every** particle in the combination is individually present in the TrigMatch container. The tester uses **per-particle** matching.

#### Format B: Full N-particle combinations (combo size > 1)

For multi-leg chains, each `xAOD::TrigComposite` stores a **complete N-particle combination**:

```
TrigMatch_HLT_e17_lhloose_nod0_2e9_lhloose_nod0 (xAOD::TrigCompositeContainer)
    │
    └── TrigComposite[0]
        └── TrigMatchedObjects: [Electrons#2, Electrons#1, Electrons#0]
            ← full 3-electron combination (pT=10.1, 20.6, 92.9 GeV)
```

For these chains, an offline combination passes R2 only if the **exact combination** exists as a stored entry. The tester uses **vector-based** matching.

#### How the tester detects the format

The tester inspects the first entry's `TrigMatchedObjects` size at the start of each chain:

```
TrigMatch combo size=1 → using per-particle R2 matching   (Format A)
TrigMatch combo size=3 → using vector-based R2 matching   (Format B)
```

The `TrigMatchedObjects` auxiliary data contains `ElementLink<xAOD::IParticleContainer>` pointing to offline particles.

### 7.3 Accessing TrigMatch Content

```cpp
const xAOD::TrigCompositeContainer* composites;
evtStore()->retrieve(composites, "TrigMatch_HLT_2mu10_nomucomb");

static const SG::AuxElement::ConstAccessor<
    std::vector<ElementLink<xAOD::IParticleContainer>>> accMatched("TrigMatchedObjects");

for (const xAOD::TrigComposite* combo : *composites) {
    const auto& links = accMatched(*combo);
    for (const auto& link : links) {
        if (link.isValid()) {
            const xAOD::IParticle* particle = *link;
            // Use particle...
        }
    }
}
```

---

## 8. Understanding Combination Generation

### 8.1 NestedUniqueCombinationGenerator

**File:** [TrigCompositeUtils/Combinators.h](../../TrigSteer/TrigCompositeUtils/TrigCompositeUtils/Combinators.h)

This utility generates all unique combinations of indices, crucial for testing multi-object triggers.

**Example: HLT_2mu10_nomucomb with 4 muons**

```
Input: 4 muons [m0, m1, m2, m3], need 2-muon combinations

Generator setup: nucg.add({4, 2})  // C(4,2) = 6 combinations

Generated combinations:
  [0, 1] → [m0, m1]
  [0, 2] → [m0, m2]
  [0, 3] → [m0, m3]
  [1, 2] → [m1, m2]
  [1, 3] → [m1, m3]
  [2, 3] → [m2, m3]
```

**Example: HLT_e7_mu24 with 2 electrons and 3 muons**

```
Generator setup:
  nucg.add({2, 1})  // 2 electrons, need 1
  nucg.add({3, 1})  // 3 muons, need 1

Generated combinations: C(2,1) × C(3,1) = 6 combinations
  [0, 0] → [e0, m0]
  [0, 1] → [e0, m1]
  [0, 2] → [e0, m2]
  [1, 0] → [e1, m0]
  [1, 1] → [e1, m1]
  [1, 2] → [e1, m2]
```

---

## 9. Common Scenarios and Expected Behavior

### 9.1 Scenario: R2 and R3 Both Pass

```
Chain: HLT_2mu10_nomucomb
Offline: 3 muons [m0:pT=30, m1:pT=25, m2:pT=12]

Testing combination [m0, m1]:
  R2 (TrigMatch): ✓ Found in pre-stored combinations
  R3 (converted): ✓ Matched to trigger objects via ΔR

Result: OK - Both agree
```

### 9.2 Scenario: R3 Passes but R2 Fails (Rare After Bug Fix)

```
Chain: HLT_2mu10_nomucomb
Offline: 3 muons [m0:pT=30, m1:pT=25, m2:pT=8]

Testing combination [m0, m2]:
  R2 (TrigMatch): ✗ m2 not individually present in TrigMatch (below threshold)
  R3 (converted): ✓ Found valid trigger combination via subfeatures

Result: EXPECTED - R3 finds more due to subfeatures or buildCombinations differences
```

**Why this can still happen (legitimately):**
- R3 with `IncludeSubfeatures=True` may find lower-pT trigger objects that R2 doesn't expose
- R3 `buildCombinations` pools all features and can form new combinations from adjacent TEs
- Run2 `IParticleRetrievalTool` returns only the highest-pT per TriggerElement

> **Note:** Before the adaptive R2 matching fix, this scenario appeared very frequently — even when all particles were individually matched. That was caused by using the wrong matching strategy for the TrigMatch container format. The fix uses an **adaptive strategy**: per-particle matching for combo size=1 containers (e.g., `HLT_2mu10_nomucomb`), and vector-based matching for combo size>1 containers (e.g., `HLT_e17_lhloose_nod0_2e9_lhloose_nod0`). After this fix, R3>R2 cases are much rarer and only occur for genuine architectural differences (subfeatures, different combination building).

### 9.3 Scenario: R2 Passes but R3 Fails (ERROR!)

```
Chain: HLT_2mu10_nomucomb
Offline: 2 muons [m0, m1]

Testing combination [m0, m1]:
  R2 (TrigMatch): ✓ Found in pre-stored combinations
  R3 (converted): ✗ Could not find matching trigger objects

Result: CRITICAL ERROR - R3 conversion is missing trigger information!
```

**This indicates a bug in:**
- Navigation conversion (missing features/links)
- Feature slimming (wrong objects removed)
- Decision node structure (incorrect chain associations)

### 9.4 Scenario: Photon Chain Testing

```
Chain: HLT_g200_etcut
Offline: 2 photons [g0:pT=250, g1:pT=180]

Testing combination [g0]:
  R2 (TrigMatch): ✓ Found in pre-stored combinations
  R3 (converted): ✓ Matched to trigger objects via ΔR

Result: OK - Both agree
```

**Note:** For "etcut" chains, R3MatchingTool automatically handles CaloCluster-to-Photon matching by extracting the embedded calorimeter cluster from the photon object.

### 9.5 Scenario: Tau Chain Testing

```
Chain: HLT_tau25_medium1_tracktwo
Offline: 3 tau jets [tau0:pT=40, tau1:pT=30, tau2:pT=20]

Testing combination [tau0]:
  R2 (TrigMatch): ✓ Found in pre-stored combinations
  R3 (converted): ✓ Matched to trigger objects via ΔR

Result: OK - Both agree
```

### 9.6 Scenario: Combined Chain (e.g., e+tau)

```
Chain: HLT_e17_lhmedium_nod0_tau80_medium1_tracktwo
Offline: 2 electrons [e0:pT=25, e1:pT=18], 2 tau jets [tau0:pT=90, tau1:pT=50]

Testing combination [e0, tau0]:
  R2 (TrigMatch): ✓ Found in pre-stored combinations
  R3 (converted): ✓ Matched to trigger objects via ΔR

Result: OK - Both agree
```

Combined chains test matching across multiple object types simultaneously.

---

## 10. Subfeatures and Their Role

### 10.1 What Are Subfeatures?

During Run2→Run3 navigation conversion, when multiple objects exist per TriggerElement:
- **"feature"**: The highest-pT object (what Run2 `IParticleRetrievalTool` would return)
- **"subfeature"**: Additional objects in the same RoI (lower-pT objects)

This is created in `Run2ToRun3TrigNavConverterV2.cxx` where the highest-pT object is stored as "feature" and remaining objects are stored as "subfeature".

### 10.2 Enabling Subfeatures in Matching

```python
r3MatchingTool.IncludeSubfeatures = True
```

When enabled, `R3MatchingTool` retrieves both "feature" and "subfeature" links, ensuring all converted objects are available for matching. This is critical for multi-object triggers where the Run2 approach would have found matches among lower-pT objects.

### 10.3 PrintSubfeatures Property

The `PrintSubfeatures` property enables detailed output of subfeature information for each passing chain:

```python
checker = CompFactory.Trig.NavigationDAODTesterAlgv2(
    ...
    PrintSubfeatures = True  # Enable subfeature printing
)
```

When enabled, the algorithm prints:
1. Primary features per leg (for reference)
2. Subfeatures per leg
3. Total subfeatures count

### 10.4 Sample PrintSubfeatures Output

```
=== Subfeatures (lower-pT objects from R2->R3 conversion) for HLT_2mu10_nomucomb ===
  --- Primary features (per leg) ---
  Leg 0 features (2):
    [feature] pt:18207.8 eta:1.05757 phi:-1.78807 container:HLTNav_RepackedFeatures_Particle index:0
    [feature] pt:18207.8 eta:1.05757 phi:-1.78807 container:HLTNav_RepackedFeatures_Particle index:2
  --- Subfeatures (per leg) ---
  Leg 0 subfeatures (2):
    [subfeature] pt:16366.5 eta:1.05053 phi:-1.74478 container:HLTNav_RepackedFeatures_Particle index:1
    [subfeature] pt:16366.5 eta:1.05053 phi:-1.74478 container:HLTNav_RepackedFeatures_Particle index:3
  --- Total subfeatures (all legs): 2 ---
```

This output shows:
- The primary "feature" (highest-pT: 18207.8 MeV) is stored at index 0 and 2
- The "subfeature" (lower-pT: 16366.5 MeV) is stored at index 1 and 3
- Both correspond to the muons in the TrigMatch container (offline pT ~17823 and ~16332 MeV)

### 10.5 Diagnostic Subfeature Check on Failures

The tester also explicitly checks for subfeatures when R2/R3 discrepancies are found:

```cpp
Trig::FeatureRequestDescriptor subFrd;
subFrd.setChainGroup(chain);
subFrd.setLinkName("subfeature");
subFrd.setFeatureCollectionMode(TrigDefs::allFeaturesOfType);
auto subfeatures = m_tdt->features<xAOD::IParticleContainer>(subFrd);
ATH_MSG_ERROR("Found " << subfeatures.size() << " subfeatures");
```

---

## 11. TriggerListsHelper: Comprehensive Testing

**File:** [DerivationFrameworkPhys/TriggerListsHelper.py](../../../PhysicsAnalysis/DerivationFramework/DerivationFrameworkPhys/python/TriggerListsHelper.py)

For comprehensive DAOD validation, use the same trigger lists as DAOD production:

```python
from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
triggerListsHelper = TriggerListsHelper(flags)

# Run2 triggers (split by tau/non-tau)
list_triggers = triggerListsHelper.Run2TriggerNamesNoTau + triggerListsHelper.Run2TriggerNamesTau

# This includes all triggers from:
# - TriggerAPI lowest unprescaled (e, mu, g, tau, cross-triggers)
# - Extra triggers from run2ExtraMatchingTriggers.txt
# - Custom triggers from flags.Trigger.derivationsExtraChains
```

---

## 12. Thread-Safety Considerations

The algorithm inherits from `AthReentrantAlgorithm` and is designed to be thread-safe:

- **Atomic counters**: Statistics counters (`m_R3R2_ok`, `m_R3_greater_R2`, etc.) use `std::atomic<unsigned int>` for lock-free updates
- **Mutex-protected map**: The `m_failingChains` map uses `std::mutex` for thread-safe access:

```cpp
mutable std::map<std::string, int> m_failingChains ATLAS_THREAD_SAFE;
mutable std::mutex m_failingChainsMutex;

// In execute():
{
    std::lock_guard<std::mutex> lock(m_failingChainsMutex);
    m_failingChains[chain]++;
}
```

The `ATLAS_THREAD_SAFE` macro from `CxxUtils/checker_macros.h` signals to the ATLAS thread checker plugin that thread safety is being handled manually.

---

## 13. Troubleshooting

### 13.1 Common Issues

| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| "No branch in this DAOD for chain" | Chain not in DAOD production list | Add to derivation config or use supported chain |
| R2 passes, R3 fails | Missing features in converted navigation | Check navigation conversion, feature slimming |
| All combinations fail R2 | Wrong InputPrefix | Verify `TrigMatch_` prefix matches DAOD |
| Type mismatch errors | Container not configured | Add container to `read_handles` map |

### 13.2 Debug Output

Enable verbose logging:

```python
checker.OutputLevel = 1  # VERBOSE
msg.debugLimit = 10000
```

### 13.3 Verifying Container Contents

To check if TrigMatch containers exist:

```bash
checkFile.py DAOD_PHYS.pool.root | grep TrigMatch
```

---

## 14. Summary

`NavigationDAODTesterAlgv2` validates the complete trigger matching workflow on DAOD files by:

1. **Generating** all possible offline particle combinations using `NestedUniqueCombinationGenerator`
2. **Comparing** matching results from:
   - `R3MatchingTool`: Uses converted Run2→Run3 navigation in real-time
   - `MatchFromCompositeTool`: Uses pre-stored `TrigMatch_*` containers from DAOD production
3. **Flagging** discrepancies with asymmetric logic:
   - R3 > R2 matches: Expected (different combination building)
   - R2 > R3 matches: **Critical error** (missing trigger information)

This ensures physics analyses using converted navigation will retrieve at least the same (and often more) trigger-matched objects as with the original Run2 approach.

---

## References

- Main implementation: [NavigationDAODTesterAlgv2.cxx](../src/NavigationDAODTesterAlgv2.cxx)
- Test configuration: [testTrigR2ToR3NavDAOD.py](../share/testTrigR2ToR3NavDAOD.py)
- R3 matching: [TriggerMatchingTool/R3MatchingTool.h](../../TrigAnalysis/TriggerMatchingTool/TriggerMatchingTool/R3MatchingTool.h)
- R2 matching: [TriggerMatchingTool/MatchFromCompositeTool.h](../../TrigAnalysis/TriggerMatchingTool/TriggerMatchingTool/MatchFromCompositeTool.h)
- DAOD producer: [DerivationFrameworkTrigger/TriggerMatchingTool.cxx](../../../PhysicsAnalysis/DerivationFramework/DerivationFrameworkTrigger/src/TriggerMatchingTool.cxx)
- Combination utilities: [TrigCompositeUtils/Combinators.h](../../TrigSteer/TrigCompositeUtils/TrigCompositeUtils/Combinators.h)
- Trigger lists: [DerivationFrameworkPhys/TriggerListsHelper.py](../../../PhysicsAnalysis/DerivationFramework/DerivationFrameworkPhys/python/TriggerListsHelper.py)
