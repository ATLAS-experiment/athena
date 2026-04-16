# Truth Vertex Labelling for ML Training

## Overview

The truth vertex labelling system assigns truth vertex labels to truth particles, reconstructed tracks, and jets for use in ML-based vertex reconstruction training. The core problem it solves is bridging the gap between the raw MC truth record — which contains thousands of truth vertices per event including generator intermediates, Geant4 re-scatterings, and broken truth chains — and a clean, physics-motivated set of vertex labels suitable for training a vertex-finding neural network.

The system produces per-particle and per-track vertex IDs and vertex type classifications, along with per-jet vertex count summaries. These labels enable supervised training of models that predict which tracks originated from the same displaced vertex, and what type of vertex produced them.

The code lives in `ParticleJetTools` within the `ParticleJetTools::FatVertex` namespace. It is introduced in MR [!84862](https://gitlab.cern.ch/atlas/athena/-/merge_requests/84862).

## Algorithms

### TruthVertexDecoratorAlg

**File**: `src/TruthVertexDecoratorAlg.cxx` / `src/TruthVertexDecoratorAlg.h`

This is the primary algorithm. It runs once per event and performs the full truth vertex reconstruction pipeline:

1. Locates the truth primary vertex (PV) by matching the `TruthPrimaryVertices` container entry against the full `TruthVertices` container (the PV container does not carry particle links, so a spatial match within 0.001 mm is used to find the linked version).
2. Traces all truth particles back to their initial-state ancestors to collect particles that should be part of the "fat PV" (handling ISR, FSR, and multi-parton interactions).
3. Generates the full set of fat vertices by recursively walking the truth particle decay tree from the PV outwards.
4. Establishes parent-child links between fat vertices.
5. Classifies each fat vertex by type (B decay, C decay, tau decay, material interaction, etc.).
6. Decorates truth particles with vertex ID, vertex type, decay multiplicities, and distances.
7. Decorates reconstructed tracks by looking up their truth-matched particle and transferring the vertex labels.

It inherits from `AthReentrantAlgorithm` and is fully thread-safe.

### JetTruthVertexSummaryDecoratorAlg

**File**: `src/JetTruthVertexSummaryDecoratorAlg.cxx` / `src/JetTruthVertexSummaryDecoratorAlg.h`

This is a downstream algorithm that must run after `TruthVertexDecoratorAlg`. It reads the per-truth-particle vertex decorations and aggregates them into per-jet summary counts. For each jet, it counts the number of truth vertices of each type that fall within a configurable delta-R cone. This produces jet-level labels like "this jet contains 1 B vertex, 2 C vertices, 3 strange vertices", which are useful for jet-level classification tasks.

Truth particles are matched to jets via `DeltaR(jet, truth_particle) < drThreshold`. An optional inner-detector volume cut restricts counting to vertices that decay within the ID acceptance.

## The FatVertex Concept

**File**: `ParticleJetTools/FatVertex.h`, `Root/FatVertex.cxx`

A **FatVertex** is a merged representation of one or more nearby truth vertices that together represent a single physically meaningful decay point. The MC truth record often splits a single physical decay into multiple truth vertices due to:

- Generator intermediates (e.g. a W boson between the B hadron and its visible decay products)
- Geant4 re-scatterings at the same spatial location
- Single-child same-PDG-ID vertices (a particle scattering without changing identity)
- Strange hadron oscillation chains (K0 -> K0bar at the same point)

The FatVertex merges all of these into a single object with three particle lists:

- **In-parts** (`m_inparts`): The particles entering the vertex. For a B hadron decay, this is the B hadron itself. For the PV, these are the initial-state particles (protons, ISR gluons, etc.).
- **Internal** (`m_internal`): Intermediate particles that were absorbed into this vertex during merging. These are generator-level particles (W bosons, virtual quarks) that don't correspond to a physically distinct vertex.
- **Out-parts** (`m_outparts`): The particles leaving the vertex. These are the visible decay products: the particles that either reach the detector as stable particles, or themselves decay at a different spatial location to form a child fat vertex.

Each FatVertex also stores:
- Whether it is the primary vertex (`m_is_pv`)
- A pointer to its parent FatVertex (`m_parent`)
- Pointers to its child FatVertices (`m_children`)

## Pipeline

The full pipeline runs inside `TruthVertexDecoratorAlg::execute()`. Here is a step-by-step walkthrough.

### Step 1: Primary Vertex Identification

The algorithm reads the `TruthPrimaryVertices` container, which contains exactly one entry but without particle links. It then searches the full `TruthVertices` container for vertices within 0.001 mm of the PV position. When multiple matches are found (which happens), the closest one is selected, with ties broken by the lowest absolute UID/barcode. This matched vertex carries the particle links needed for the recursive walk.

**Function**: `get_matched_primary_vertices()`

### Step 2: Initial Particle Collection

Not all particles in an event originate from the PV's outgoing particle list. ISR, multi-parton interactions, and other generator-level processes produce particles whose ancestry traces back to initial-state partons but may not pass through the PV vertex object. The algorithm traces every truth particle in the event back through its parent chain (following the highest-pT parent at each step when there are multiple parents) to find all unique initial-state ancestors. These are collected and passed to the fat vertex generator as additional PV in-parts.

**Function**: `get_initial_partial()`, `get_all_initial_particles()`

### Step 3: Recursive Fat Vertex Generation

Starting from the PV, the algorithm performs a breadth-first walk through the truth particle tree. For each particle in the `to_search` queue:

1. Call `generateFatVertex()` on the particle's decay vertex.
2. `generateFatVertex()` processes each outgoing particle from the truth vertex:
   - **Non-physical status particles** (status != 1 and != 2): Recursively absorbed as internal particles. These are generator intermediates.
   - **Particles with no valid children**: Added as out-parts (stable endpoints).
   - **Particles whose decay vertex is within `truthVertexMergeDistance`** of the first in-part's decay vertex: Recursively merged into the current fat vertex. This is the spatial merging step.
   - **Single-child same-PDG-ID particles**: Absorbed as internal (simple scattering with no reconstructable vertex).
   - **Single-child strange-to-strange particles**: Absorbed as internal (strange hadron oscillation chains like K0 -> K0bar).
   - **Everything else**: Added as an out-part (a particle that will seed its own fat vertex).
3. After building the fat vertex, all its in-parts and internals are marked as "searched" so they are not processed again. All its out-parts are added to the `to_search` queue to seed the next level of fat vertices.
4. Fat vertices with zero out-parts are discarded (they represent stable particles, not vertices).

**Functions**: `generateFatVertex()`, `generateFatVertices()`

### Step 4: Parent-Child Linking

After all fat vertices are generated, the algorithm iterates through the full list and establishes parent-child relationships. A fat vertex V2 is the parent of V1 if V1's first in-part appears in V2's out-parts. Conversely, V2 is a child of V1 if V2's first in-part appears in V1's out-parts.

These links are required by the type classification step, since some vertex types depend on the parent's type (e.g. `CHadronDecayFromBHadron` vs plain `CHadronDecay`).

**Function**: `FatVertex::doParentChildLinks()`

### Step 5: Vertex Type Classification

Each fat vertex is classified into a `DetailedVertexType` based on the properties of its in-parts, out-parts, and parent vertex. The classification logic in `FatVertex::getType()` proceeds through a priority-ordered decision tree:

#### Detailed Vertex Types

| Enum Value | Description |
|---|---|
| `PrimaryVertex` | The primary interaction vertex |
| `BHadronDecay` | In-part is a bottom hadron |
| `CHadronDecay` | In-part is a charm hadron (no B parent) |
| `CHadronDecayFromBHadron` | In-part is a charm hadron, parent vertex is a B decay |
| `TauDecay` | In-part is a tau lepton (no B parent) |
| `TauDecayFromBHadron` | In-part is a tau, parent vertex is a B decay |
| `StrangeDecay` | In-part is a strange hadron (no HF/tau parent) |
| `StrangeFromBHadron` | In-part is a strange hadron, parent is B decay |
| `StrangeFromCHadron` | In-part is a strange hadron, parent is C decay |
| `StrangeFromCFromBHadron` | In-part is a strange hadron, parent is C-from-B decay |
| `StrangeFromTau` | In-part is a strange hadron, parent is tau decay |
| `StrangeFromTauFromBHadron` | In-part is a strange hadron, parent is tau-from-B decay |
| `StrangeOscillation` | Strange hadron with single strange hadron output (e.g. K0 -> K0bar) |
| `PionDecay` | In-part is a charged or neutral pion (PDG ID 211 or 111) |
| `MaterialInteraction` | Energy difference > 100 MeV between in-part and sum of out-parts (hadronic interaction, see ATL-COM-PHYS-2016-058) |
| `MaybeMaterialInteraction` | Single valid out-part with energy difference > 10 MeV but < 100 MeV |
| `PhotoelectricEmission` | Photon in, electron out |
| `ComptonScattering` | Photon in, photon + electron out |
| `EPAnnihilation` | Positron in, one or two photons out |
| `Bremsstrahlung` | Charged lepton in, same-flavour lepton + photon out; or tau in with one child tau + one photon |
| `Conversion` | Photon in, lepton-antilepton pair out (gamma -> e+e-), including cases with broken truth chains |
| `OtherNInparts` | Vertex has multiple in-parts (anomalous) |
| `OtherNoOutparts` | Vertex has zero out-parts (anomalous) |
| `OtherSingleOutpart` | Single out-part that doesn't match any known pattern |
| `Other` | Catch-all for unclassified vertices |
| `OtherSecondaryVertex` | (Defined but assigned via the fallback classification for Geant4 simulation particles) |

#### Simple Vertex Types

The detailed types map to a reduced set of simple types for downstream use:

| Simple Type | Detailed Types Included |
|---|---|
| `PrimaryVertex` | `PrimaryVertex` |
| `BHadronDecay` | `BHadronDecay` |
| `CHadronDecay` | `CHadronDecay`, `CHadronDecayFromBHadron` |
| `TauDecay` | `TauDecay`, `TauDecayFromBHadron` |
| `StrangeDecay` | `StrangeDecay`, `StrangeOscillation`, `StrangeFromBHadron`, `StrangeFromCHadron`, `StrangeFromCFromBHadron`, `StrangeFromTau`, `StrangeFromTauFromBHadron` |
| `PionDecay` | `PionDecay` |
| `MaterialInteraction` | `MaterialInteraction`, `PhotoelectricEmission`, `ComptonScattering`, `EPAnnihilation`, `Bremsstrahlung`, `Conversion` |
| `OtherSecondaryVertex` | `OtherSecondaryVertex` |
| `Other` | `Other`, `OtherNInparts`, `OtherNoOutparts`, `OtherSingleOutpart` |

#### Special Integer Codes (not enum members)

| Value | Meaning | Context |
|---|---|---|
| `-1` | Invalid / not assigned | Default for particles not associated with any fat vertex |
| `-2` | Pileup | Tracks with no truth match, or tracks whose truth particle traces back to a pileup vertex |
| `-3` | Fake | Tracks whose truth match probability falls below `truthMatchProbabilityCut` |

### Step 6: Truth Particle Decoration

**Every truth particle in the container is decorated**, not just those associated with a vertex. The algorithm first initialises all particles with default values (type = -1, ID = -1, `ftagTPIsVertex` = 0, etc.), then overwrites the relevant fields for particles that belong to a fat vertex.

The `ftagTPIsVertex` flag distinguishes particles that *are* vertices from those that are not. A truth particle with `ftagTPIsVertex = 1` is the **in-part** (decaying particle) of a classified fat vertex — it has a decay vertex that was identified and classified by the algorithm. A truth particle with `ftagTPIsVertex = 0` is either a stable particle (no decay vertex), an out-part of another vertex that does not itself decay, or a particle that was not associated with any fat vertex. This flag is the primary way downstream consumers can filter to "only vertices" when the full truth particle collection is saved.

For each fat vertex (ordered by construction, starting from the PV), the algorithm assigns an integer vertex ID (a sequential counter starting at 0 for the PV). It then decorates:

- **In-part particles** (the decaying particle): vertex type, vertex ID, charged/neutral multiplicities of the vertex's out-parts, and `ftagTPIsVertex = 1`.
- **Internal particles**: vertex ID only (for debug traceability). These keep `ftagTPIsVertex = 0` since they are absorbed intermediates, not distinct vertices.
- **Out-part particles**: parent vertex ID (`ftagTPDecayParentID`), and if the particle has a decay vertex, the 3D distances from PV (`ftagTPDecayPVDistance`) and from production to decay vertex (`ftagTPDecayDistance`). Out-parts keep `ftagTPIsVertex = 0` unless they themselves appear as an in-part of a child fat vertex.

PV in-parts (protons, ISR gluons) are skipped — they are not physically meaningful particles to label.

Stable out-parts (those without a decay vertex) are optionally marked as valid based on the `alwaysKeepStableCharged` and `alwaysKeepStableNeutrals` configuration flags.

### Step 7: Track Decoration

Reconstructed tracks are matched to truth particles using the `InDetTrackTruthOriginTool`. For each track:

1. If no truth match exists, the track is labelled as pileup (type = -2).
2. If the truth match probability is below `truthMatchProbabilityCut` (default 0.5), the track is labelled as fake (type = -3).
3. Otherwise, the track inherits the vertex ID and vertex type from its truth-matched particle via UID/barcode lookup.
4. If the truth particle was an internal particle of a fat vertex rather than an out-part, the track still receives the correct vertex ID and type (with a debug message).
5. If the truth particle cannot be found in any fat vertex, the track is assigned vertex ID = -4 and type = -2 (pileup), as this indicates the particle likely originated from a pileup vertex that was close enough to the PV to be retained in the truth record.

Tracks also receive the PDG ID of their truth-matched particle and the PDG ID of that particle's parent.

A trackless stable charged particle filtering mechanism is also available: when `alwaysKeepTracklessStableCharged` is true, stable charged truth particles that have no associated reconstructed track are marked as valid for output. This ensures the training data includes truth information about charged particles that the detector missed.

### Step 8: Jet-Level Summary (JetTruthVertexSummaryDecoratorAlg)

This second algorithm reads the per-truth-particle decorations written by `TruthVertexDecoratorAlg` and counts vertices per jet. For each jet, it iterates over truth particles that have a valid vertex ID, checks delta-R matching to the jet, and increments type-specific counters. To avoid double-counting, it uses the vertex ID to ensure each unique vertex is only counted once per jet.

An optional inner-detector volume cut (`onlyID`) restricts counting to vertices whose production vertex falls within `Lxy < lxyIDThreshold` (default 1082 mm) and `|z| < zIDThreshold` (default 2710 mm).

## Output Variables

### Truth Particle Decorations

Written to **every** truth particle in the container (default: `TruthParticles`). All particles receive default values; only those belonging to a fat vertex have their fields overwritten with meaningful values.

| Variable | Type | Default | Description |
|---|---|---|---|
| `ftagTPIsVertex` | `int` | 0 | **1** if this particle is the in-part (decaying particle) of a classified fat vertex, **0** otherwise. Use this to filter to "only vertices" in downstream analysis. |
| `ftagTPValid` | `int` | 0 | Whether this particle is associated with a classified fat vertex (set to 1 for in-parts and for stable out-parts kept by `alwaysKeepStable*` flags) |
| `ftagTPDecayVertexType` | `int` | -1 | Detailed vertex type enum value (see table above). Only set for in-parts (`ftagTPIsVertex = 1`). |
| `ftagTPDecaySimpleVertexType` | `int` | -1 | Simple vertex type enum value. Only set for in-parts. |
| `ftagTPDecayVertexID` | `int` | -1 | Sequential vertex ID (0 = PV, 1+ = secondary vertices). Set for in-parts and internal particles. |
| `ftagTPDecayVertexParentID` | `int` | -1 | Vertex ID of the parent fat vertex. Set for out-parts. |
| `ftagTPDecayVertexNCharged` | `int` | -1 | Number of charged out-parts above `truthParticleMinimumPt`. Only set for in-parts. |
| `ftagTPDecayVertexNNeutral` | `int` | -1 | Number of neutral (non-neutrino) out-parts above `truthParticleMinimumPt`. Only set for in-parts. |
| `ftagTPDecayVertexAllNCharged` | `int` | -1 | Number of charged out-parts (no pT cut). Only set for in-parts. |
| `ftagTPDecayVertexAllNNeutral` | `int` | -1 | Number of neutral (non-neutrino) out-parts (no pT cut). Only set for in-parts. |
| `ftagTPDecayPVDistance` | `float` | NaN | 3D distance from PV to this particle's decay vertex [mm]. Set for out-parts that have a decay vertex. |
| `ftagTPDecayDistance` | `float` | NaN | 3D distance from production to decay vertex [mm]. Set for out-parts that have both production and decay vertices. |

### Track Decorations

Written to the track particle container (default: `InDetTrackParticles`).

| Variable | Type | Description |
|---|---|---|
| `ftagTrackDecayVertexID` | `int` | Vertex ID from truth-matched particle. -1 = unmatched to vertex, -4 = pileup (traced to non-PV origin) |
| `ftagTrackDecayVertexType` | `int` | Detailed vertex type. -1 = unassigned, -2 = pileup, -3 = fake |
| `ftagTrackDecaySimpleVertexType` | `int` | Simple vertex type. -1 = unassigned, -2 = pileup, -3 = fake |
| `trackPDGID` | `int` | PDG ID of the truth-matched particle, 0 if no match |
| `trackParentPDGID` | `int` | PDG ID of the truth-matched particle's parent, 0 if unavailable |

### Jet Decorations

Written to the jet container (default: `AntiKt4EMPFlowJets`).

| Variable | Type | Description |
|---|---|---|
| `ftagJetNumBVertices` | `int` | Number of B hadron decay vertices in the jet |
| `ftagJetNumCVertices` | `int` | Number of C hadron decay vertices in the jet |
| `ftagJetNumTauVertices` | `int` | Number of tau decay vertices in the jet |
| `ftagJetNumStrangeVertices` | `int` | Number of strange decay vertices in the jet |
| `ftagJetNumPionVertices` | `int` | Number of pion decay vertices in the jet |
| `ftagJetNumMaterialIntVertices` | `int` | Number of material interaction vertices in the jet |
| `ftagJetNumOtherVertices` | `int` | Number of other/unclassified vertices in the jet |
| `ftagJetNumVertices` | `int` | Total number of vertices in the jet |

## Configuration

### TruthVertexDecoratorAlg Parameters

| Parameter | Type | Default | Description |
|---|---|---|---|
| `truthContainer` | `string` | `"TruthParticles"` | Input truth particle container |
| `TruthPrimaryVertices` | `string` | `"TruthPrimaryVertices"` | Input PV container |
| `TruthVertices` | `string` | `"TruthVertices"` | Input truth vertex container |
| `trackContainer` | `string` | `"InDetTrackParticles"` | Input track container |
| `truthVertexMergeDistance` | `float` | `1.0` | Distance threshold [mm] for merging nearby truth vertices into a single fat vertex. Default 1.0 mm folds pi0 decays (ctau ~25 nm) into their parent vertex. |
| `truthParticleMinimumPt` | `float` | `0` | Minimum pT [MeV] for counting charged/neutral multiplicities in `NCharged`/`NNeutral`. Default 0 means `NCharged`/`NNeutral` match `AllNCharged`/`AllNNeutral` unless overridden. |
| `alwaysKeepStableNeutrals` | `bool` | `true` | Mark stable neutral out-parts as valid |
| `alwaysKeepStableCharged` | `bool` | `true` | Mark all stable charged out-parts as valid |
| `alwaysKeepTracklessStableCharged` | `bool` | `true` | Mark stable charged out-parts as valid only if they have no associated track |
| `truthMatchProbabilityAuxName` | `string` | `"truthMatchProbability"` | Name of the truth match probability auxiliary variable on tracks |
| `truthMatchProbabilityCut` | `float` | `0.5` | Tracks below this truth match probability are labelled as fakes |
| `useBarcode` | `bool` | `false` | Use legacy `barcode` instead of `uid` for particle identification |
| `trackTruthOriginTool` | `ToolHandle` | `InDetTrackTruthOriginTool` | Tool for track-to-truth matching |

### JetTruthVertexSummaryDecoratorAlg Parameters

| Parameter | Type | Default | Description |
|---|---|---|---|
| `truthContainer` | `string` | `"TruthParticles"` | Input truth particle container |
| `jetContainer` | `string` | `"AntiKt4EMPFlowJets"` | Input jet container |
| `drThreshold` | `float` | `0.4` | Delta-R cone for matching truth particles to jets |
| `onlyID` | `bool` | `false` | If true, only count vertices decaying within the inner detector volume |
| `lxyIDThreshold` | `float` | `1082.0` | Maximum transverse distance [mm] for ID volume cut |
| `zIDThreshold` | `float` | `2710.0` | Maximum longitudinal distance [mm] for ID volume cut |

## Broken Truth Chain Handling

The MC truth record is frequently incomplete. Geant4 does not preserve all particles, and the ATLAS truth thinning removes low-energy or short-lived intermediates. The algorithm handles several classes of broken truth chains:

### Missing Children

When a particle reports `nChildren() > 0` but some or all child pointers are null, the algorithm uses `num_valid_children()` and `has_valid_child()` to count only accessible children. If a particle has zero valid children despite claiming to have children, it is treated as a stable endpoint.

### Partial Decay Records

The type classifier explicitly handles cases where the truth record is incomplete:

- **Bremsstrahlung with missing lepton**: When a charged lepton has 1-2 reported children but only 1 valid child that is a photon, it is still classified as bremsstrahlung (the outgoing lepton was dropped from the record).
- **Conversion with missing positron**: When a photon has 2 reported children but only 1 valid child that is a charged lepton, it is classified as a conversion (gamma -> e+e- where one lepton was dropped).
- **Conversion with absorbed lepton**: When a photon produces one lepton as an internal particle and one as an out-part (because one lepton immediately interacted with material), it is still classified as a conversion.

### Geant4 Simulation Particles

The algorithm uses `HepMC::is_simulation_particle()` and `HepMC::SIM_REGENERATION_INCREMENT` to identify particles added by Geant4 simulation (as opposed to the generator). This is used in two ways:

1. **Material interaction fallback**: If a vertex has more reported children than valid children, and the first valid child is a simulation particle, the vertex is classified as a material interaction. This catches cases where Geant4 added secondary particles but some were subsequently thinned.
2. **Lepton re-scattering detection**: When a charged lepton's out-parts include a particle with the same base UID (modulo `SIM_REGENERATION_INCREMENT`), this indicates Geant4 re-used the particle with updated kinematics. This is classified as a material interaction rather than a decay.

### Ancestor Tracing

The `get_initial_partial()` function walks up the parent chain with a hard limit of 1000 iterations to prevent infinite loops in pathological truth records. When multiple parents exist at a vertex, it follows the highest-pT parent, which is a heuristic to follow the hard-scatter chain rather than pileup or underlying-event branches.
