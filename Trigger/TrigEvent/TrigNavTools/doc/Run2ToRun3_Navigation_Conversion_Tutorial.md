# Review and Tutorial: HLT Navigation Conversion from Run2 to Run3 Format

**Author:** Documentation for ATLAS Trigger Navigation Conversion
**Date:** February 2026
**Version:** 1.0

---

## Summary

This document provides a detailed review of the ATLAS HLT (High-Level Trigger) navigation conversion system that transforms Run2 TriggerElement-based navigation into the Run3 TrigComposite/Decision-based format. This conversion is essential for analyzing legacy Run2 data with modern Run3 analysis tools.

---

## Table of Contents

1. [Background: Why Navigation Conversion is Needed](#1-background-why-navigation-conversion-is-needed)
2. [Architecture Overview](#2-architecture-overview)
3. [Run2 Navigation Structure (Input)](#3-run2-navigation-structure-input)
4. [Run3 Navigation Structure (Output)](#4-run3-navigation-structure-output)
5. [The Conversion Algorithm: Step-by-Step](#5-the-conversion-algorithm-step-by-step)
6. [Configuration](#6-configuration)
7. [Validation with NavigationTesterAlg](#7-validation-with-navigationtesteralg)
8. [Running the Conversion](#8-running-the-conversion)
9. [Key Implementation Details](#9-key-implementation-details)
10. [Comparison: Run2 vs Run3 Retrieval Tools](#10-comparison-run2-vs-run3-retrieval-tools)
11. [Troubleshooting](#11-troubleshooting)
12. [Summary](#12-summary)
- [Appendix A: Object Retrieval Tools and the "Subfeature" Mechanism](#appendix-a-object-retrieval-tools-and-the-subfeature-mechanism)
- [References](#references)

---

## 1. Background: Why Navigation Conversion is Needed

### 1.1 The Problem

ATLAS Run2 (2015-2018) data uses a **TriggerElement (TE)** tree-based navigation structure, while Run3 (2022+) uses a modern **TrigComposite/Decision** graph-based structure. Physics analysis tools designed for Run3 cannot directly process Run2 navigation, creating a compatibility barrier for legacy data analysis.

### 1.2 The Solution

The `Run2ToRun3TrigNavConverterV2` algorithm converts Run2 navigation into Run3 format on-the-fly, enabling:
- Use of modern analysis tools (TrigDecisionTool in Run3 mode)
- Consistent physics object retrieval across run periods
- Production of DAOD files with converted navigation

---

## 2. Architecture Overview

### 2.1 Key Files and Their Roles

| File | Purpose |
|------|---------|
| `testTrigR2ToR3NavGraphConversionV2.py` | Test/example job configuration |
| `NavConverterConfig.py` | Python configuration for conversion algorithm |
| `Run2ToRun3TrigNavConverterV2.h/.cxx` | Core conversion algorithm implementation |
| `NavigationTesterAlg.h/.cxx` | Validation algorithm comparing Run2 vs Run3 retrieval |
| `SpecialCases.h` | Regex patterns for chains requiring special handling |

### 2.2 Data Flow

```
Run2 AOD/ESD File
       │
       ▼
┌─────────────────────────────┐
│  TrigNavigation (xAOD)      │  ← Serialized Run2 navigation
│  or TrigDecisionTool        │
└─────────────────────────────┘
       │
       ▼
┌──────────────────────────────┐
│  Run2ToRun3TrigNavConverterV2│  ← Conversion Algorithm
│  (creates ConvProxy objects) │
└──────────────────────────────┘
       │
       ▼
┌──────────────────────────────┐
│  xAOD::TrigCompositeContainer│  ← Run3 navigation format
│  (HLTNav_R2ToR3Summary)      │
└──────────────────────────────┘
       │
       ▼
┌─────────────────────────────┐
│  NavigationTesterAlg        │  ← Validation (optional)
│  (compares Run2 vs Run3)    │
└─────────────────────────────┘
```

---

## 3. Run2 Navigation Structure (Input)

### 3.1 TriggerElement (TE) Tree

Run2 navigation is organized as a **tree of TriggerElements**:

```
Initial Node (root)
    │
    ├── RoI Node 1 (L1 seed)
    │       │
    │       ├── L2_mu10 (TE)
    │       │       │
    │       │       └── EF_mu10 (TE) ← features attached here
    │       │
    │       └── L2_e24 (TE)
    │               │
    │               └── EF_e24 (TE)
    │
    └── RoI Node 2
            └── ...
```

**Key concepts:**
- **TriggerElement (TE)**: A node in the navigation tree representing a reconstruction step
- **TE ID**: Hash of the TE name (e.g., "EF_mu10" → numeric hash)
- **Active State**: Whether the TE passed its hypothesis algorithm
- **FeatureAccessHelper (FEA)**: Pointer to attached physics objects (CLID + index)

### 3.2 Feature Storage

Features (physics objects) are attached to TEs via `FeatureAccessHelper`:
- **CLID**: Class ID identifying the object type (e.g., MuonContainer)
- **SubTypeIndex**: Collection index within that CLID
- **ObjectsBegin/End**: Index range within the sub-collection

**File Reference:** `Trigger/TrigEvent/TrigNavStructure/TrigNavStructure/TriggerElement.h:192-232`

---

## 4. Run3 Navigation Structure (Output)

### 4.1 Decision Graph

Run3 navigation is a **directed acyclic graph (DAG) of Decision nodes**:

```
       HLTPassRaw (terminus)
           │
    ┌──────┴──────┐
    SF           SF    (Summary Filter nodes)
    │             │
    H             H    (Hypothesis nodes - features attached)
    │             │
    IM           IM    (Input Maker nodes)
    │             │
    L1           L1    (HLT Seeding nodes - RoIs attached)
```

**Node Types:**
| Node | Name String | Purpose |
|------|-------------|---------|
| L1 | `"L1"` | HLT seeding from Level-1 |
| IM | `"IM"` | Input Maker - reconstruction entry |
| H | `"H"` | Hypothesis - decision point, features attached |
| SF | `"SF"` | Summary Filter - chain termination |
| HLTPassRaw | `"HLTPassRaw"` | Terminus collecting passed chains |

### 4.2 Decision ID System

Each node carries a set of **DecisionIDs** (chain hashes):
- `runChains`: Chains that executed through this node
- `passChains`: Chains that passed at this node

**Multi-leg chains** use leg IDs: `createLegName(chainId, legNumber)` → e.g., `leg000_HLT_2mu10`

**File Reference:** `Trigger/TrigSteer/TrigCompositeUtils/TrigCompositeUtils/TrigCompositeUtils.h:424-433`

---

## 5. The Conversion Algorithm: Step-by-Step

### 5.1 Algorithm Overview

The conversion happens in `Run2ToRun3TrigNavConverterV2::execute()`:

**File Reference:** `Trigger/TrigEvent/TrigNavTools/src/Run2ToRun3TrigNavConverterV2.cxx:253-355`

```cpp
StatusCode Run2ToRun3TrigNavConverterV2::execute(const EventContext &context) const {
    // 1. Extract TE-to-chain mapping (first event only)
    // 2. Mirror TE structure into ConvProxy objects
    // 3. Associate chains to proxies
    // 4. Cure unassociated proxies (propagate chains upward)
    // 5. Remove topological/unassociated proxies
    // 6. Optional: Compression (collapse similar proxies)
    // 7. Fill features, RoIs, tracks
    // 8. Create Run3 decision nodes (IM, H, L1, SF)
    // 9. Link features to H nodes
    // 10. Update terminus node
}
```

### 5.2 ConvProxy: The Intermediate Representation

`ConvProxy` is a temporary structure that bridges Run2 TEs to Run3 Decisions:

```cpp
struct ConvProxy {
    const HLT::TriggerElement *te;           // Original Run2 TE
    std::vector<HLT::te_id_type> teIDs;      // Merged TE IDs (post-compression)

    std::set<ConvProxy*> children;           // Downstream proxies
    std::set<ConvProxy*> parents;            // Upstream proxies

    std::set<HLT::Identifier> runChains;     // Chains that ran
    std::set<HLT::Identifier> passChains;    // Chains that passed

    uint64_t feaHash;                        // Hash of attached features
    std::vector<FeatureAccessHelper> features;  // Features to link
    std::vector<FeatureAccessHelper> rois;      // RoIs to link
    std::vector<FeatureAccessHelper> tracks;    // Tracks to link

    Decision *l1Node;                        // Created L1 node
    Decision *imNode;                        // Created IM node
    std::vector<Decision*> hNode;            // Created H node(s)
};
```

**File Reference:** `Trigger/TrigEvent/TrigNavTools/src/Run2ToRun3TrigNavConverterV2.h:34-59`

### 5.3 Step 1: Extract TE-to-Chain Mapping

`extractTECtoChainMapping()` builds maps from TE IDs to chains:

```cpp
TEIdToChainsMap_t m_allTEIdsToChains;   // All TEs → chains
TEIdToChainsMap_t m_finalTEIdsToChains; // Final TEs → chains (for SF nodes)
```

This uses trigger configuration (`TrigConf::HLTChain`) to:
1. Iterate all configured chains
2. Extract TE IDs from each chain's signatures
3. Handle multi-leg chains by creating leg IDs
4. Apply special handling for etcut, topo, bjet-mu chains

**File Reference:** `Trigger/TrigEvent/TrigNavTools/src/Run2ToRun3TrigNavConverterV2.cxx:401-545`

### 5.4 Step 2: Mirror TE Structure

`mirrorTEsStructure()` creates ConvProxy objects mirroring the TE tree:

```cpp
for (auto te : run2Nav.getAllTEs()) {
    if (HLT::TrigNavStructure::isInitialNode(te)) continue;

    auto proxy = new ConvProxy(te);
    convProxies.insert(proxy);
    teToProxy[te] = proxy;

    // Build parent-child links
    for (auto predecessor : HLT::TrigNavStructure::getDirectPredecessors(te)) {
        ConvProxy *predecessorProxy = teToProxy[predecessor];
        proxy->parents.insert(predecessorProxy);
        predecessorProxy->children.insert(proxy);
    }
}
```

**File Reference:** `Trigger/TrigEvent/TrigNavTools/src/Run2ToRun3TrigNavConverterV2.cxx:586-649`

### 5.5 Step 3: Associate Chains to Proxies

`associateChainsToProxies()` maps chains to proxies based on TE ID:

```cpp
for (auto &proxy : convProxies) {
    auto teId = proxy->te->getId();
    auto iter = allTEs.find(teId);
    if (iter != allTEs.end()) {
        proxy->runChains.insert(iter->second.begin(), iter->second.end());
        if (proxy->te->getActiveState()) {
            proxy->passChains.insert(iter->second.begin(), iter->second.end());
        }
    }
}
```

### 5.6 Step 4: Cure Unassociated Proxies

`cureUnassociatedProxies()` propagates chain IDs upward through the tree:

```cpp
// Repeat until no updates needed
while (true) {
    size_t numberOfUpdates = 0;
    for (auto p : convProxies) {
        for (auto child : p->children) {
            // Insert child's chains into parent
            p->runChains.insert(child->runChains.begin(), child->runChains.end());
            p->passChains.insert(child->runChains.begin(), child->runChains.end());
        }
    }
    if (numberOfUpdates == 0) break;
}
```

This ensures all proxies have associated chains (chains flow from leaves to root).

### 5.7 Step 5: Compression (Optional)

When `doCompression=true`, similar proxies are merged:

**Feature-based collapse:** Proxies with identical feature hashes merge:
```cpp
proxy->feaHash = feaToHash(proxy->te->getFeatureAccessHelpers(), ...);
// Group by hash, then merge
```

**Featureless collapse:** Adjacent featureless proxies with same parent/child merge.

**Merge operation:**
```cpp
void ConvProxy::merge(ConvProxy *other) {
    runChains.insert(other->runChains.begin(), other->runChains.end());
    passChains.insert(other->passChains.begin(), other->passChains.end());
    teIDs.push_back(other->te->getId());
    // Rewire parent/child links
}
```

**File Reference:** `Trigger/TrigEvent/TrigNavTools/src/Run2ToRun3TrigNavConverterV2.cxx:762-917`

### 5.8 Step 6: Create Run3 Nodes

**IM and H nodes** (`createIMHNodes()`):
```cpp
for (auto &proxy : convProxies) {
    proxy->imNode = newDecisionIn(&decisions, "IM");
    for (auto chainId : proxy->runChains)
        addDecisionID(chainId, proxy->imNode);

    proxy->hNode.push_back(newDecisionIn(&decisions, "H"));
    for (auto chainId : proxy->passChains)
        addDecisionID(chainId, proxy->hNode.back());

    linkToPrevious(proxy->hNode.front(), proxy->imNode, context);
}
// Connect IM to parent's H nodes
for (auto &parentProxy : proxy->parents) {
    linkToPrevious(proxy->imNode, parentProxy->hNode.front(), context);
}
```

**L1 nodes** (`createL1Nodes()`): Created for proxies without parents (root nodes).

**SF nodes** (`createSFNodes()`): Created for terminal proxies, linking to terminus.

### 5.9 Step 7: Link Features

`linkFeaNode()` attaches physics objects to H nodes:

```cpp
for (auto &proxy : convProxies) {
    for (auto &fea : proxy->features) {
        auto [sgKey, sgCLID, sgName] = getSgKey(run2Nav, fea);
        // Link feature to H node
        hNode->typelessSetObjectLink("feature", sgKey, sgCLID, index);
    }
}
```

**Feature selection:** When multiple features exist, the highest-pT object is linked as "feature", others as "subfeature".

**File Reference:** `Trigger/TrigEvent/TrigNavTools/src/Run2ToRun3TrigNavConverterV2.cxx:1263-1320`

### 5.10 Step 8: Update Terminus Node

The final step of the conversion is `updateTerminusNode()`, which cleans up the **HLTPassRaw terminus node** — the single top-level node that downstream analysis tools inspect to determine which chains fired in the event.

**File Reference:** `Trigger/TrigEvent/TrigNavTools/src/Run2ToRun3TrigNavConverterV2.cxx` (method `updateTerminusNode`)

At this stage, the terminus node may contain both whole-chain IDs and leg IDs (e.g. `leg000_HLT_2mu10`) accumulated during SF node creation. The method applies three filters:

**1. De-legging:** Leg IDs are converted back to their parent chain IDs so that only whole-chain identifiers remain in the terminus:

```cpp
const TCU::DecisionID idToCheck = ( TCU::isLegId(id)
    ? TCU::getIDFromLeg( HLT::Identifier(id) ).numeric()
    : id );
```

**2. Sanity check against `m_chainsToSave`:** If a restricted chain list was configured, the method verifies that every chain found in the terminus is actually on that list. If not, the algorithm aborts with `StatusCode::FAILURE`.

**3. Filtering by `isPassed` with `allowResurrectedDecision`:** This is the core pass/fail filter that determines which chains are retained:

```cpp
if (m_tdt->isPassed(chainName, TrigDefs::Physics | TrigDefs::allowResurrectedDecision))
{
    filteredIDs.insert(idToCheck);
}
```

The two flags combined with bitwise OR have the following meaning:

| Flag | Purpose |
|------|---------|
| `TrigDefs::Physics` | Requires the chain to have passed the full physics-level decision, including all HLT steps and prescale. This is the standard "did this chain fire?" check. |
| `TrigDefs::allowResurrectedDecision` | Additionally accepts chains that were **resurrected**. In ATLAS, some chains that were originally prescaled away can be re-executed ("resurrected") for monitoring, calibration, or other purposes. Without this flag, such chains would be filtered out even though they produced valid navigation and physics objects. |

The combination `TrigDefs::Physics | TrigDefs::allowResurrectedDecision` therefore means: *accept a chain if it passed the physics decision **or** if it was resurrected and passed upon re-execution*. This is deliberately more permissive than a plain `isPassed(chainName)` call (which uses default flags and would exclude resurrected chains), ensuring that the converted Run3 navigation faithfully preserves all chains that produced valid trigger decisions, including those rerun for monitoring.

After filtering, the terminus node's decisions are replaced with only the filtered, passing chain IDs:

```cpp
terminus->setDecisions( std::vector<TCU::DecisionID>() );  // Clear
TCU::insertDecisionIDs(filteredIDs, terminus);               // Insert filtered set
```

This guarantees that the HLTPassRaw node — the entry point for any analysis querying which chains fired — contains exactly the set of whole-chain IDs (no legs) that genuinely passed (including resurrected chains) and belong to the configured conversion list.

---

## 6. Configuration

### 6.1 NavConverterConfig.py

The Python configuration:

**File:** `Trigger/TrigEvent/TrigNavTools/python/NavConverterConfig.py`

```python
def NavConverterCfg(flags, chainsList=[], runTheChecker=False):
    # Only run for Run 1/2 data
    if flags.Trigger.EDMVersion >= 3:
        return acc
    if not flags.Trigger.doEDMVersionConversion:
        return acc

    # Configure converter algorithm
    cnvAlg = CompFactory.Run2ToRun3TrigNavConverterV2("TrigRun2ToRun3NavConverter")
    cnvAlg.TrigDecisionTool = tdt
    cnvAlg.OutputNavKey = "HLTNav_R2ToR3Summary"
    cnvAlg.Rois = ["initialRoI", "forID", "forMS", ...]
    cnvAlg.Collections = types  # From TriggerEDM AODCONV list
    cnvAlg.Chains = chainsList
    cnvAlg.doCompression = True
```

### 6.2 Key Configuration Properties

| Property | Type | Description |
|----------|------|-------------|
| `Chains` | `vector<string>` | Chains to convert (empty = all) |
| `Collections` | `vector<string>` | Collections to link (from TriggerEDM) |
| `Rois` | `vector<string>` | RoI names to preserve |
| `doCompression` | `bool` | Enable proxy merging |
| `doLinkFeatures` | `bool` | Link physics objects |
| `doSelfValidation` | `bool` | Run consistency checks |

---

## 7. Validation with NavigationTesterAlg

### 7.1 Purpose

`NavigationTesterAlg` validates conversion by comparing particle retrieval:

```cpp
// Retrieve particles via Run2 navigation
m_toolRun2->retrieveParticles(vecCombinationsRun2, chain);

// Retrieve particles via converted Run3 navigation
m_toolRun3->retrieveParticles(vecCombinationsRun3, chain);

// Compare results
verifyCombinationsContent(combsRun2, combsRun3, chain);
```

### 7.2 What it Validates

1. **Combination sizes**: Run3 should have ≥ Run2 combinations
2. **Object identity**: Same physics objects should be retrieved
3. **Pointer equality**: Direct comparison of `xAOD::IParticle*`

**File Reference:** `Trigger/TrigEvent/TrigNavTools/src/NavigationTesterAlg.cxx:76-144`

### 7.3 Special Cases

Some chains require special handling:

**File:** `Trigger/TrigEvent/TrigNavTools/src/SpecialCases.h`

```cpp
// Excluded chains (known issues)
const std::vector<std::string> excludedChains{
    "HLT_mu20_msonly_mu6noL1_msonly_nscan05",
    "HLT_2mu4_bJpsimumu",
    ...
};

// Chains needing special decoding
const std::regex gammaXeChain{"HLT_g.*_xe.*"};
const std::regex isTopo{".*(Jpsi|Zee).*"};
const std::regex bjetMuChain{"HLT_mu.*_j.*_split_.*"};
```

---

## 8. Running the Conversion

### 8.1 Test Script

```bash
cd athena/Trigger/TrigEvent/TrigNavTools/share/
python testTrigR2ToR3NavGraphConversionV2.py \
    --filesInput /path/to/run2_aod.pool.root \
    --evtMax 100
```

### 8.2 In Production Jobs

Add to your job configuration:
```python
from TrigNavTools.NavConverterConfig import NavConverterCfg
cfg.merge(NavConverterCfg(flags, chainsList=myChains, runTheChecker=True))
```

---

## 9. Key Implementation Details

### 9.1 CLID Handling

The converter must translate between Run2 CLID-based storage and Run3:

```cpp
// CLIDs cached at initialization
m_MuonContainerCLID       // xAOD::MuonContainer
m_ElectronContainerCLID   // xAOD::ElectronContainer
m_TauJetContainerCLID     // xAOD::TauJetContainer
// ... etc
```

### 9.2 TE-Based Particle Type Detection

For EGamma triggers, the converter infers expected particle type from TE name:

```cpp
CLID getExpectedParticleCLID(const std::string& teName) const {
    if (teName.find("etcut") != std::string::npos)
        return m_CaloClusterContainerCLID;
    if (teName.rfind("EF_e", 0) == 0)
        return m_ElectronContainerCLID;
    if (teName.rfind("EF_g", 0) == 0)
        return m_PhotonContainerCLID;
    // ...
}
```

**File Reference:** `Trigger/TrigEvent/TrigNavTools/src/Run2ToRun3TrigNavConverterV2.cxx:1632-1664`

### 9.3 Multi-Leg Chain Handling

Chains like `HLT_2mu10_nomucomb` require leg-aware conversion:

```cpp
std::vector<int> multiplicities = ChainNameParser::multiplicities(chainName);
if (multiplicities.size() > 1) {
    for (size_t legNumber = 0; legNumber < teIds.size(); ++legNumber) {
        HLT::Identifier chainLegId = createLegName(chainId, legNumber);
        allTEs[teIds[legNumber]].insert(chainLegId);
    }
}
```

---

## 10. Comparison: Run2 vs Run3 Retrieval Tools

| Aspect | Run2 (IParticleRetrievalTool) | Run3 (R3IParticleRetrievalTool) |
|--------|-------------------------------|--------------------------------|
| Navigation | TE tree traversal | LinkInfo direct access |
| Code complexity | ~350 lines, recursive | ~90 lines, utility-based |
| Type handling | CLID lookup + TE name parsing | Generic IParticleContainer |
| Thread safety | Limited | Mutex-protected |
| Feature selection | Highest-pT per TE | All linked particles |

---

## 11. Troubleshooting

### 11.1 Common Issues

| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| Empty Run3 combinations | Chain not in conversion list | Add to `Chains` property |
| Missing features | Collection not in EDM list | Check `AODCONV` in TriggerEDM |
| Wrong particle type | TE name heuristics failed | Check `getExpectedParticleCLID()` |
| Validation failures | Special case chain | Add to `SpecialCases.h` exclusions |

### 11.2 Debug Output

Enable verbose logging:
```python
cnvAlg.OutputLevel = 2  # DEBUG
checker.OutputLevel = 1 # VERBOSE
```

---

## 12. Summary

The Run2-to-Run3 navigation converter is a sophisticated algorithm that:

1. **Reads** Run2 TriggerElement tree navigation
2. **Builds** ConvProxy intermediate representation
3. **Maps** chains to proxies using trigger configuration
4. **Compresses** similar proxies for efficiency
5. **Creates** Run3 Decision graph (L1 → IM → H → SF → HLTPassRaw)
6. **Links** physics objects to appropriate nodes
7. **Validates** via dual-retrieval comparison

This enables seamless analysis of Run2 data using modern Run3 tools, critical for long-term physics analysis spanning multiple ATLAS run periods.

---

## Appendix A: Object Retrieval Tools and the "Subfeature" Mechanism

While not directly part of the conversion algorithm, understanding the particle retrieval tools is essential for comprehending **why** certain design decisions were made in the converter, particularly the introduction of **"subfeature"** links.

### A.1 The Multiple-Objects-Per-RoI Problem

In Run2, a single TriggerElement within a Region of Interest (RoI) can have **multiple physics objects** attached to it. This is encoded in the `FeatureAccessHelper` via the `objectsBegin` and `objectsEnd` indices:

```cpp
// FeatureAccessHelper contains:
ObjectIndex idx;
idx.objectsBegin();  // Start index in container
idx.objectsEnd();    // End index in container
// If objectsEnd - objectsBegin > 1, multiple objects exist!
```

**This situation occurs primarily in EGamma triggers**, where reconstruction may produce multiple electron/photon candidates within the same RoI. The `IParticleRetrievalTool` handles this in `retrieveFeatureParticle()`:

**File Reference:** `Trigger/TrigAnalysis/TriggerMatchingTool/Root/IParticleRetrievalTool.cxx:318-348`

```cpp
switch (particleFeatures.size()) {
    case 0:
        // Error: no particles retrieved
        navFailure = true;
        break;
    case 1:
        // Normal case: single particle
        particle = particleFeatures.at(0);
        break;
    default:
        // PROBLEM: Multiple outputs within the same RoI!
        // "Some TEs can end up reporting multiple outputs within the same RoI.
        //  AFAIK this only happens within EGamma TEs but I don't know that for
        //  sure. In any case this shouldn't matter too much for the matching
        //  given that they will be nearby each other. Just return the highest pT
        //  object."
        particle = *(std::max_element(
            particleFeatures.begin(), particleFeatures.end(),
            [] (const xAOD::IParticle* lhs, const xAOD::IParticle* rhs)
            { return lhs->pt() < rhs->pt(); }));
}
```

**Key insight:** Run2's `IParticleRetrievalTool` returns **only the highest-pT object** when multiple objects exist in the same RoI.

### A.2 IParticleRetrievalTool (Run2 Navigation)

**File:** `Trigger/TrigAnalysis/TriggerMatchingTool/Root/IParticleRetrievalTool.cxx`

**Architecture:**
- Recursive TE tree traversal
- CLID-based type lookup (hardcoded mapping)
- Special EGamma handling via TE name parsing
- ~350 lines of code

**CLID Mapping** (lines 20-26):
```cpp
static const std::vector<CLIDTuple_t> CLIDVector {
    {1178459224, xAOD::Type::Muon, "xAOD::MuonContainer"},
    {1087532415, xAOD::Type::Electron, "xAOD::ElectronContainer"},
    {1219821989, xAOD::Type::CaloCluster, "xAOD::CaloClusterContainer"},
    {1105575213, xAOD::Type::Photon, "xAOD::PhotonContainer"},
    {1177172564, xAOD::Type::Tau, "xAOD::TauJetContainer"}
};
```

**EGamma Type Detection** (lines 226-240):
```cpp
xAOD::Type::ObjectType IParticleRetrievalTool::getEGammaTEType(
    const HLT::TriggerElement* te) const
{
    std::string teName = Trig::getTEName(*te);
    if (teName.find("etcut") != std::string::npos &&
        teName.find("trkcut") == std::string::npos)
        return xAOD::Type::CaloCluster;  // etcut chains use CaloCluster
    else if (teName.starts_with("EF_e"))
        return xAOD::Type::Electron;
    else if (teName.starts_with("EF_g"))
        return xAOD::Type::Photon;
    else
        return xAOD::Type::Other;
}
```

**Retrieval Flow:**
1. Get chain group from TrigDecisionTool
2. Get FeatureContainer with all combinations
3. For each combination, iterate TriggerElements
4. For each TE, search FeatureAccessHelpers by CLID
5. Handle EGamma special cases (CaloCluster vs Electron/Photon)
6. Extract particle(s) from container using ObjectIndex
7. **If multiple particles: return highest-pT only**

### A.3 R3IParticleRetrievalTool (Run3 Navigation)

**File:** `Trigger/TrigAnalysis/TriggerMatchingTool/Root/R3IParticleRetrievalTool.cxx`

**Architecture:**
- Direct `LinkInfo<IParticleContainer>` access
- Uses `TrigCompositeUtils::buildCombinations()`
- Generic particle handling (no CLID mapping needed)
- Thread-safe with mutex-protected warning cache
- ~90 lines of code

**Core Implementation** (lines 26-88):
```cpp
StatusCode R3IParticleRetrievalTool::retrieveParticles(
    std::vector<std::vector<const xAOD::IParticle*>>& combinations,
    const std::string& chain, bool rerun) const
{
    combinations.clear();
    const ChainGroup* cg = m_tdt->getChainGroup(chain);

    for (const std::string& name : cg->getListOfTriggers()) {
        if (!m_tdt->isPassed(name, ...)) continue;

        // Direct feature access - returns ALL linked particles
        const auto& features = m_tdt->features<xAOD::IParticleContainer>(name);

        // Check for dead legs (invalid links)
        if (std::any_of(features.begin(), features.end(),
            [](const auto& li) { return !li.link.isValid(); })) {
            // Warn and skip
            continue;
        }

        // Build combinations using Run3 utilities
        TrigCompositeUtils::Combinations trigCombinations =
            TrigCompositeUtils::buildCombinations(
                name, features,
                m_tdt->ExperimentalAndExpertMethods().getChainConfigurationDetails(name),
                TrigCompositeUtils::FilterType::UniqueObjects);

        // Extract particles from LinkInfo
        for (const VecLinkInfo_t& combo : trigCombinations) {
            std::vector<const xAOD::IParticle*> comboOut;
            for (const IPartLinkInfo_t& info : combo)
                comboOut.push_back(*info.link);  // ALL particles included
            combinations.push_back(std::move(comboOut));
        }
    }
    return StatusCode::SUCCESS;
}
```

**Key Difference:** Run3 retrieves **ALL linked particles**, not just the highest-pT.

### A.4 The Subfeature Solution

Without special handling, a naive conversion would create a mismatch:
- **Run2 retrieval**: Returns 1 particle (highest-pT) per TE
- **Run3 retrieval**: Returns N particles (all linked) per Decision node

This would cause validation failures in `NavigationTesterAlg`.

**Solution:** The converter introduces a **"subfeature"** link name to distinguish:
- **`"feature"`**: The primary (highest-pT) object — what Run2 retrieval returns
- **`"subfeature"`**: Additional objects in the same RoI — preserved for completeness

### A.5 How Subfeatures Are Created

**Step 1: Identify the highest-pT object**

**File Reference:** `Trigger/TrigEvent/TrigNavTools/src/Run2ToRun3TrigNavConverterV2.cxx:1591-1630`

```cpp
std::pair<std::size_t, std::size_t> Run2ToRun3TrigNavConverterV2::getHighestPtObject(
    const ConvProxy& proxy, const HLT::TrigNavStructure& run2Nav) const
{
    std::size_t bestFea = std::numeric_limits<std::size_t>::max();
    std::size_t bestObj = 0;
    float bestPt = -1.0;

    // Iterate all features attached to this proxy
    for (std::size_t i = 0; i < proxy.features.size(); ++i) {
        const auto& fea = proxy.features[i];
        auto [sgKey, sgCLID, sgName] = getSgKey(run2Nav, fea);
        if (!feaToSave(fea, sgName)) continue;
        if (sgKey == 0) continue;

        // Retrieve container from event store
        const xAOD::IParticleContainer* cont = nullptr;
        if (evtStore()->retrieve(cont, *keyStr).isFailure()) continue;

        // Check each object in the feature's index range
        for (auto n = fea.getIndex().objectsBegin(); n < fea.getIndex().objectsEnd(); ++n) {
            const xAOD::IParticle* p = (*cont)[n];
            if (p->pt() > bestPt) {
                bestPt = p->pt();
                bestFea = i;   // Which feature
                bestObj = n;   // Which object within that feature
            }
        }
    }
    return {bestFea, bestObj};
}
```

**Step 2: Link features with appropriate names**

**File Reference:** `Trigger/TrigEvent/TrigNavTools/src/Run2ToRun3TrigNavConverterV2.cxx:1263-1320`

```cpp
StatusCode Run2ToRun3TrigNavConverterV2::linkFeaNode(...) const
{
    for (const auto& proxy : convProxies) {
        // Find the highest-pT object
        auto [bestFeaIdx, bestObjIdx] = getHighestPtObject(*proxy, run2Nav);

        // Expand H nodes if multiple features exist
        auto feaN = getFeaSize(*proxy);
        if (feaN > 1) {
            // Create additional H nodes for each object
            while (--feaN) {
                proxy->hNode.push_back(newDecisionIn(&decisions, "H"));
                // ... connect to IM node and child IMs
            }
        }

        // Link each object with appropriate name
        for (std::size_t feaIdx = 0; feaIdx < proxy->features.size(); ++feaIdx) {
            auto& fea = proxy->features[feaIdx];

            for (auto n = fea.getIndex().objectsBegin(); n < fea.getIndex().objectsEnd(); ++n) {
                // KEY LOGIC: Choose link name based on whether this is the best object
                const std::string& linkName =
                    (feaIdx == bestFeaIdx && n == bestObjIdx) ?
                    TrigCompositeUtils::featureString() :  // "feature"
                    "subfeature";                          // secondary objects

                (*hNodeIter)->typelessSetObjectLink(linkName, sgKey, sgCLID, n, n + 1);
                ++hNodeIter;
            }
        }
    }
    return StatusCode::SUCCESS;
}
```

### A.6 When Subfeatures Are Created

Subfeatures are created when a TE has:
1. **Multiple FeatureAccessHelpers** attached (e.g., different reconstruction stages)
2. **A single FeatureAccessHelper with a range** (`objectsEnd - objectsBegin > 1`)

**Common scenarios:**
- EGamma chains with overlapping electron/photon candidates
- Tau chains with multiple tau candidates in the same RoI
- Chains where reconstruction produces backup candidates

### A.7 Accessing Subfeatures in Analysis

While standard Run3 retrieval (`m_tdt->features<T>(chain)`) returns the **"feature"** link by default, subfeatures can be explicitly retrieved:

```cpp
// Standard retrieval - gets "feature" links only
auto features = m_tdt->features<xAOD::IParticleContainer>(chain);

// Explicit subfeature retrieval
Trig::FeatureRequestDescriptor subFrd;
subFrd.setChainGroup(chain);
subFrd.setLinkName("subfeature");  // Request subfeatures specifically
subFrd.setFeatureCollectionMode(TrigDefs::allFeaturesOfType);
auto subfeatures = m_tdt->features<xAOD::IParticleContainer>(subFrd);
```

### A.8 Summary: Retrieval Tool Comparison

| Aspect | IParticleRetrievalTool (Run2) | R3IParticleRetrievalTool (Run3) |
|--------|-------------------------------|--------------------------------|
| **Location** | `TriggerMatchingTool/Root/IParticleRetrievalTool.cxx` | `TriggerMatchingTool/Root/R3IParticleRetrievalTool.cxx` |
| **Lines of code** | ~350 | ~90 |
| **Navigation access** | Recursive TE traversal | Direct LinkInfo access |
| **Type handling** | CLID lookup table + TE name parsing | Generic IParticleContainer |
| **EGamma handling** | Explicit `getEGammaTEType()` method | Automatic via feature links |
| **Multi-object RoIs** | Returns highest-pT only | Returns all (feature + subfeature) |
| **Thread safety** | No explicit protection | Mutex-protected warning cache |
| **Container lookup** | Manual SG key formatting | Direct link dereferencing |

### A.9 Why This Matters for Validation

The `NavigationTesterAlg` compares results from both tools. For validation to pass:

1. **Run2 tool** returns: `{highest-pT object}` per TE
2. **Run3 tool** (with default "feature" link) should return: `{highest-pT object}` per H node

The subfeature mechanism ensures this equivalence by:
- Marking the highest-pT object as "feature" (what both tools return)
- Preserving additional objects as "subfeature" (accessible if needed, but not breaking validation)

This design maintains **backward compatibility** while enabling **forward-looking** access to all reconstructed objects.

---

## References

- Main implementation: `Trigger/TrigEvent/TrigNavTools/src/Run2ToRun3TrigNavConverterV2.cxx`
- Configuration: `Trigger/TrigEvent/TrigNavTools/python/NavConverterConfig.py`
- Run2 Navigation: `Trigger/TrigEvent/TrigNavStructure/`
- Run3 Utilities: `Trigger/TrigSteer/TrigCompositeUtils/`
- Particle Retrieval Tools:
  - Run2: `Trigger/TrigAnalysis/TriggerMatchingTool/Root/IParticleRetrievalTool.cxx`
  - Run3: `Trigger/TrigAnalysis/TriggerMatchingTool/Root/R3IParticleRetrievalTool.cxx`

---

*Document generated for ATLAS Collaboration internal use.*
