# ColumnarToolWrapperPython

Python bindings for ATLAS columnar CP tools.

## Setup

```bash
setupATLAS -q
asetup <your release>
python -m venv --system-site-packages venv
./venv/bin/pip install awkward uproot
```

TOML configuration support (Task 5) requires Python 3.11+ (`tomllib` stdlib) or `pip install tomli` for earlier versions.

---

## Quick Start

The simplest way to run a tool: use the `atlascp` submodule.

```python
from ColumnarToolWrapperPython import atlascp
import uproot

# Create a tool
tool = atlascp.MuonEfficiencyScaleFactors(
    "myTool",
    rename_containers={"EventInfo": "EventInfoAuxDyn", "Muons": "AnalysisMuonsAuxDyn"},
)

# Load events from a PHYSLITE file
with uproot.open("DAOD_PHYSLITE.root") as f:
    tree = f["CollectionTree"]
    input_fields = [c.name for c in tool.input_columns if not c.is_optional]
    events = tree.arrays(input_fields, entry_stop=100)

# Run the tool
result = tool(events)
print(result["AnalysisMuonsAuxDyn.sfOut"].to_list())
```

That's it. Three levels of interface are provided below if you need more control.

---

## Concepts

### API Levels

Three levels of interface are provided, from highest to lowest level:

| Level | Class / module | Best for |
|-------|---------------|----------|
| Analyzer | `atlascp` | Simplest and full of sugar |
| High-level | `Tool` | General non-expert developer access |
| Low-level | `PythonToolHandle` | Expert access, manual buffer control, custom integrations |

### Container Renaming

CP tools refer to data by canonical names (`"Muons"`, `"EventInfo"`). PHYSLITE files use prefixed branch names (`"AnalysisMuonsAuxDyn"`). The `rename_containers` parameter maps `{canonical_name: branch_prefix}` so that column names reported by the tool match your input array's field names.

For example, after setting `rename_containers={"EventInfo": "EventInfoAuxDyn", "Muons": "AnalysisMuonsAuxDyn"}`, the tool's `ColumnInfo.name` fields will use the prefixed names (`"EventInfoAuxDyn.runNumber"` instead of `"EventInfo.runNumber"`).

### Virtual Arrays and Lazy Loading

uproot's `virtual=True` returns lazily-loaded arrays — data is read from disk only when accessed. This avoids loading unused columns and improves performance:

```python
input_fields = [c.name for c in tool.input_columns if not c.is_optional]
events = tree.arrays(filter_name=input_fields, virtual=True)  # Lazy loading
```

Remove `virtual=True` and `filter_name` if you want eager loading of all columns into memory immediately.

---

## Using `atlascp`

The `atlascp` submodule lets you construct tools by name without writing the `CP::ToolType/instanceName` string manually.

```python
from ColumnarToolWrapperPython import atlascp

# Equivalent to Tool("CP::MuonEfficiencyScaleFactors/myTool")
tool = atlascp.MuonEfficiencyScaleFactors("myTool")

# Instance name can be omitted — a unique name is auto-generated
tool = atlascp.MuonEfficiencyScaleFactors()

# With properties and container renaming
tool = atlascp.MuonEfficiencyScaleFactors(
    "myTool",
    properties={"WorkingPoint": "Tight"},
    rename_containers={
        "EventInfo": "EventInfoAuxDyn",
        "Muons": "AnalysisMuonsAuxDyn",
    },
)
result = tool(events)
```

### Constructor Details

`instance_name` is positional-only. Passing it by keyword raises `TypeError`:

```python
# Wrong: raises TypeError
tool = atlascp.MuonEfficiencyScaleFactors(instance_name="myTool")

# Also wrong: positional-only is a hard constraint
tool = atlascp.MuonEfficiencyScaleFactors("myTool", None)

# Correct: name positional, properties/rename_containers as keywords
tool = atlascp.MuonEfficiencyScaleFactors("myTool", properties={...})
```

The `namespace` keyword (default `"CP"`) selects the C++ namespace:

```python
# Equivalent to Tool("columnar::VectorExampleTool/vecEx")
tool = atlascp.VectorExampleTool("vecEx", namespace="columnar")
```

### TOML Configuration

For frameworks managing multiple tools, a TOML file can configure all tools at once. Use `[[tool.TypeName]]` (array-of-tables) to define tool instances — multiple blocks of the same type create multiple instances with different properties:

```toml
# config.toml
[global.rename_containers]
EventInfo = "EventInfoAuxDyn"
Muons = "AnalysisMuonsAuxDyn"

[global.properties]
CalibTag = "251211_Preliminary"

# First instance of MuonEfficiencyScaleFactors
[[tool.MuonEfficiencyScaleFactors]]
properties.WorkingPoint = "Tight"

# Second instance with different working point — overrides global Muons container
[[tool.MuonEfficiencyScaleFactors]]
properties.WorkingPoint = "Medium"
rename_containers.Muons = "SelectedMuonsAuxDyn"
```

Tool-level settings override global settings for both `rename_containers` and `properties`.

Use `atlascp.configure()` to load and instantiate tools:

```python
from ColumnarToolWrapperPython import atlascp

corrections = atlascp.configure("config.toml")

# Access tools by type name — returns a list (multiple instances of same type allowed)
tight_tool, medium_tool = corrections["MuonEfficiencyScaleFactors"]

# Run all tools and merge outputs
result = corrections.apply(events)

# Iterate in config-file order
for type_name, tool in corrections.items():
    print(type_name, tool.properties)
```

**Note:** `config` is not re-exported from the top-level package. For advanced merging logic, import explicitly:

```python
from ColumnarToolWrapperPython.config import load_config, merge_tool_config

cfg = load_config("config.toml")
global_settings = cfg["global"]
for entry in cfg["tool"]["MuonEfficiencyScaleFactors"]:
    merged = merge_tool_config(global_settings, entry)
    # merged now has tool-specific overrides applied to global defaults
```

**TOML paths:** `atlascp.configure()` accepts a `str` or `pathlib.Path`.

---

## Using `Tool`

`Tool` is the high-level wrapper. It takes an awkward array of events and handles buffer extraction, column setting, calling, and output reconstruction automatically.

```python
import awkward as ak
import uproot
from ColumnarToolWrapperPython import Tool

tool = Tool(
    "CP::MuonEfficiencyScaleFactors/myTool",
    rename_containers={
        "EventInfo": "EventInfoAuxDyn",
        "Muons": "AnalysisMuonsAuxDyn",
    },
)

# Load events from a PHYSLITE file
with uproot.open("DAOD_PHYSLITE.root") as f:
    tree = f["CollectionTree"]
    input_fields = [c.name for c in tool.input_columns if not c.is_optional]
    events = tree.arrays(filter_name=input_fields, entry_stop=100, virtual=True)

# Returns an ak.Array with one field per output column
result = tool(events)
print(result["AnalysisMuonsAuxDyn.sfOut"].to_list())
```

### Inspecting Columns and Systematics

```python
# All columns (ColumnInfo objects)
for col in tool.columns:
    print(col.name, col.access_mode, col.is_optional)

# Input / output columns separately
print([c.name for c in tool.input_columns])
print([c.name for c in tool.output_columns])

# Properties set at construction time
print(tool.properties)

# Recommended systematic variations
print(tool.recommended_systematics)
```

### Systematic Variations

Apply systematic variations in two ways:

**Two-step form** (persistent state):

```python
for sys_name in tool.recommended_systematics:
    tool.apply_systematic_variation(sys_name)
    result = tool(events)
    print(sys_name, result["AnalysisMuonsAuxDyn.sfOut"].to_list())
    tool.apply_systematic_variation("")  # reset to nominal
```

**One-step form** (inline keyword, resets automatically):

```python
nominal = tool(events)
varied = tool(events, systematic="MUON_EFF_RECO_SYS__1up")

# Tool is back to nominal after the call — reset is automatic
back_to_nominal = tool(events)
```

The `systematic=` keyword temporarily applies the variation, runs the tool, then resets to nominal (`""`). The reset happens in a `finally` block, so the tool state is always restored even if the tool call raises an exception.

---

## The Same Task at Each Level

The following three snippets all do the same thing: run `MuonEfficiencyScaleFactors` on a PHYSLITE file and print the output scale factors. They differ only in how much boilerplate you write.

### Level 1 — `atlascp` (least boilerplate)

```python
import uproot
from ColumnarToolWrapperPython import atlascp

tool = atlascp.MuonEfficiencyScaleFactors(
    "myTool",
    properties={"WorkingPoint": "Tight"},
    rename_containers={"EventInfo": "EventInfoAuxDyn", "Muons": "AnalysisMuonsAuxDyn"},
)

with uproot.open("DAOD_PHYSLITE.root") as f:
    events = f["CollectionTree"].arrays(
        filter_name=[c.name for c in tool.input_columns if not c.is_optional],
        virtual=True,
    )

result = tool(events)
print(result["AnalysisMuonsAuxDyn.sfOut"].to_list())
```

### Level 2 — `Tool` (explicit type string)

```python
import uproot
from ColumnarToolWrapperPython import Tool

tool = Tool(
    "CP::MuonEfficiencyScaleFactors/myTool",
    properties={"WorkingPoint": "Tight"},
    rename_containers={"EventInfo": "EventInfoAuxDyn", "Muons": "AnalysisMuonsAuxDyn"},
)

with uproot.open("DAOD_PHYSLITE.root") as f:
    events = f["CollectionTree"].arrays(
        filter_name=[c.name for c in tool.input_columns if not c.is_optional],
        virtual=True,
    )

result = tool(events)
print(result["AnalysisMuonsAuxDyn.sfOut"].to_list())
```

### Level 3 — `PythonToolHandle` (full manual control)

```python
import numpy as np
import uproot
from ColumnarToolWrapperPython import PythonToolHandle

handle = PythonToolHandle()
handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/myTool")
handle.set_property("WorkingPoint", "Tight")
handle.initialize()
handle.rename_containers({"EventInfo": "EventInfoAuxDyn", "Muons": "AnalysisMuonsAuxDyn"})

# Load flat numpy arrays from uproot
input_names = [c.name for c in handle.columns if not c.is_offset and not c.is_optional]
with uproot.open("DAOD_PHYSLITE.root") as f:
    arrays = f["CollectionTree"].arrays(input_names, library="np")

# Allocate output buffers
n_muons = len(arrays["AnalysisMuonsAuxDyn.pt"])
sf_out    = np.zeros(n_muons, dtype=np.float32)
valid_out = np.zeros(n_muons, dtype=np.int8)

# Set offsets, inputs, and outputs — hold all references until after call()
event_info_offsets = np.array([0, len(arrays["AnalysisMuonsAuxDyn.pt"])], dtype=np.uint64)
muon_offsets       = np.arange(n_muons + 1, dtype=np.uint64)
handle["EventInfoAuxDyn"]             = event_info_offsets
handle["EventInfoAuxDyn.runNumber"]   = arrays["EventInfoAuxDyn.runNumber"]
handle["EventInfoAuxDyn.RandomRunNumber"] = arrays["EventInfoAuxDyn.RandomRunNumber"]
handle["EventInfoAuxDyn.eventTypeBitmask"] = arrays["EventInfoAuxDyn.eventTypeBitmask"]
handle["AnalysisMuonsAuxDyn"]         = muon_offsets
handle["AnalysisMuonsAuxDyn.pt"]      = arrays["AnalysisMuonsAuxDyn.pt"]
handle["AnalysisMuonsAuxDyn.eta"]     = arrays["AnalysisMuonsAuxDyn.eta"]
handle["AnalysisMuonsAuxDyn.phi"]     = arrays["AnalysisMuonsAuxDyn.phi"]
handle["AnalysisMuonsAuxDyn.muonType"] = arrays["AnalysisMuonsAuxDyn.muonType"]
handle.set_column_void("AnalysisMuonsAuxDyn.sfOut",    sf_out,    False)
handle.set_column_void("AnalysisMuonsAuxDyn.validOut", valid_out, False)

handle.call()
print(sf_out.tolist())
```

---

## Event Selection

You can select events before running the tool using boolean indexing:

```python
import numpy as np

selection = np.array([True, False, True, ...])
events_selected = events[selection]
result = tool(events_selected)  # Processes only selected events
```

The tool correctly handles any masking or filtering of the event array. This works transparently with the high-level `Tool` API.

---

## C++ Message Logging Integration

C++ tool messages route to Python's `logging` module automatically on import via the `"atlas"` logger. The default level is INFO and the default format closely matches the C++ `MessagePrinter` output:

```
ToolSvc.myTool           INFO    Efficiency type is = RECO
ToolSvc.myTool           INFO    JPsi based low pt SF will start to rock below 10 GeV!
ToolSvc.myTool           INFO    Trying to initialize, with working point Medium, ...
ToolSvc.myTool           INFO    Successfully initialized!
```

Set the level **before** importing to control what is shown:

```python
import logging
logging.getLogger("atlas").setLevel(logging.WARNING)  # suppress INFO messages

from ColumnarToolWrapperPython import atlascp
tool = atlascp.MuonEfficiencyScaleFactors("myTool")
# (no output — INFO suppressed)

tool2 = atlascp.MuonEfficiencyScaleFactors("myTool")
# Package.AsgTools.ToolStore WARNING asg::ToolStore::put: Tool with name "ToolSvc.myTool" already registered
# (WARNING still shown; INFO suppressed)
```

The level can also be changed after import:

```python
logging.getLogger("atlas").setLevel(logging.ERROR)  # suppress warnings too
logging.getLogger("atlas").setLevel(logging.DEBUG)   # show DEBUG and VERBOSE
```

To opt out of the Python bridge entirely and restore the raw C++ stdout printer:

```python
from ColumnarToolWrapperPython.logging_bridge import remove_printer
remove_printer()
# Output reverts to C++ default stdout format — identical appearance but
# no longer controllable via Python logging level or handlers.
```

### Custom Logging Configuration

The default handler is `logging.getLogger("atlas").handlers[0]`. To change its format in place:

```python
import logging
logging.getLogger("atlas").handlers[0].setFormatter(
    logging.Formatter("[%(tool)s] %(levelname)s %(message)s")
)
```

Output:
```
[ToolSvc.myTool] INFO Efficiency type is = RECO
[ToolSvc.myTool] INFO Successfully initialized!
[Package.AsgTools.ToolStore] WARNING asg::ToolStore::put: Tool with name "ToolSvc.myTool" already registered
```

To replace the handler entirely:

```python
import logging, sys
logger = logging.getLogger("atlas")
logger.handlers.clear()
handler = logging.StreamHandler(sys.stderr)
handler.setFormatter(logging.Formatter("[%(tool)s] %(levelname)s %(message)s"))
logger.addHandler(handler)
```

With timestamp:

```python
handler.setFormatter(logging.Formatter(
    "[%(asctime)s] [%(levelname)s] [%(tool)s] %(message)s",
    datefmt="%H:%M:%S"
))
```

### Rich Logging Integration (Optional)

For pretty colored output with Rich:

```python
from rich.logging import RichHandler

class ToolRichHandler(RichHandler):
    """RichHandler subclass that displays tool names in brackets."""
    def render_message(self, record, message):
        tool = getattr(record, "tool", None)
        if tool:
            message = f"[{tool}] {message}"
        return super().render_message(record, message)

# Use it
handler = ToolRichHandler()
logger.addHandler(handler)
```

### Message Levels

C++ `MSG::Level` is mapped to Python logging levels:

- `NIL` → `NOTSET`
- `VERBOSE` → 5 (custom level below logging.DEBUG)
- `DEBUG` → `DEBUG`
- `INFO` → `INFO`
- `WARNING` → `WARNING`
- `ERROR` → `ERROR`
- `FATAL` → `CRITICAL`

Control verbosity:

```python
logger = logging.getLogger("atlas")
logger.setLevel(logging.INFO)    # Hide DEBUG and VERBOSE
logger.setLevel(5)               # Show VERBOSE and above
logger.setLevel(logging.DEBUG)   # Show all including VERBOSE
```

### Auto-Cleanup

The bridge is automatically removed at Python interpreter shutdown via `atexit`. You do not need to call `remove_printer()` at exit — it is only needed if you want to revert to the C++ stdout printer during a session.

**Note:** `logging_bridge` is not re-exported from the top-level package. Import explicitly: `from ColumnarToolWrapperPython.logging_bridge import remove_printer`.

---

## Using `PythonToolHandle`

For direct control over column buffers — useful when your data is already in numpy arrays rather than awkward arrays, or when writing custom integrations.

```python
import numpy as np
from ColumnarToolWrapperPython import PythonToolHandle

handle = PythonToolHandle()
handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/myTool")
handle.initialize()

# Set properties before initialize()
handle = PythonToolHandle()
handle.set_type_and_name("CP::MuonCalibTool/myTool")
handle.set_property("IsRun3Geo", False)   # via set_property()
handle.calibMode = 0                       # or via attribute assignment
handle.initialize()
```

### Setting and Calling Columns

Input columns use `__setitem__`; output columns (written by the tool) require `set_column_void` with `is_const=False` so the tool can write into the buffer. **Hold references to all numpy arrays until after `call()`** — the handle stores raw pointers, so arrays must not be garbage-collected mid-call.

```python
handle = PythonToolHandle()
handle.set_type_and_name("columnar::SimpleSelectorExampleTool/myTool")
handle.initialize()

event_info   = np.array([0, 2],          dtype=np.uint64)   # offsets: [start, end]
particles    = np.array([0, 1, 3],       dtype=np.uint64)   # offsets: [0, 1, 2, 3]
particles_pt = np.array([10e5, 10e5, 1e3], dtype=np.float32)
selection    = np.array([0, 0, 0],       dtype=np.int8)     # output buffer

handle["EventInfo"]          = event_info
handle["Particles"]          = particles
handle["Particles.pt"]       = particles_pt
handle.set_column_void("Particles.selection", selection, False)  # mutable output

handle.call()
print(list(selection))  # [1, 1, 0]
```

### Reading Columns Back

You can read any column (input or output) via `__getitem__`:

```python
sf_arr = handle["Muons.sfOut"]    # returns a read-only numpy view
```

`dict(handle)` returns all columns as a `{name: array}` mapping. Unset columns return empty arrays; `call()` resets all columns back to empty:

```python
# After setting columns, before call():
cols = dict(handle)
print(cols["Muons.pt"])   # populated
print(cols["Muons.isLRT"])  # empty — optional, not set

handle.call()
cols = dict(handle)       # all empty again after call()
```

### Inspecting Column Metadata

```python
for col in handle.columns:
    print(col.name, col.access_mode, col.is_optional, col.is_offset)

print(handle.get_recommended_systematics())
```

---

## CutBookKeepers (Sum of Weights)

When normalising MC samples you need the sum of weights from before any event selection — this is stored as CutBookKeeper metadata in the `MetaData` TTree of PHYSLITE (and other xAOD) ROOT files, not in the event tree.

```python
import awkward as ak
from ColumnarToolWrapperPython import read_cutbookkeepers

# Single file
cbk = read_cutbookkeepers("DAOD_PHYSLITE.root")
print(cbk[0].nEventsProcessed)   # total events processed
print(cbk[0].sumOfWeights)       # sum of MC generator weights
print(cbk[0].sumOfWeightsSquared)

# Multiple files — one record per file, aggregate with ak.sum()
files = ["file1.root", "file2.root", "file3.root"]
cbk = read_cutbookkeepers(files)
total_sow = float(ak.sum(cbk.sumOfWeights))

# Data integrity check: nonzero means some files had incomplete bookkeepers
# (produced by crashed jobs — event counts may be unreliable)
if ak.any(cbk.nIncomplete > 0):
    print("Warning: incomplete CutBookkeepers found")
```

The result is an awkward record array with fields:

| Field | Type | Description |
|---|---|---|
| `nEventsProcessed` | int | Events processed before any selection |
| `sumOfWeights` | float | Sum of MC generator weights |
| `sumOfWeightsSquared` | float | Sum of squared MC generator weights |
| `nIncomplete` | int | Entries in `IncompleteCutBookkeepers` (data-integrity flag) |

### Stream selection

The `AllExecutedEvents` entry with the maximum skimming cycle is selected from any of the default allowed streams: `StreamAOD`, `StreamDAOD_PHYSLITE`, `StreamEVGEN`, `StreamEVNT`. This matches the logic in `CP::AsgCutBookkeeperAlg`.

To restrict to a specific stream:

```python
cbk = read_cutbookkeepers(files, input_stream="StreamAOD")
# or a list:
cbk = read_cutbookkeepers(files, input_stream=["StreamAOD", "StreamEVGEN"])
```

To look up a different named bookkeeper:

```python
cbk = read_cutbookkeepers(files, bookkeeper_name="PHYSLITEKernel")
```

---

## Advanced: `buffers` Submodule

The `buffers` submodule contains the internal functions that `Tool.__call__` uses to handle the boilerplate of buffer extraction, allocation, and reconstruction:

```python
from ColumnarToolWrapperPython.buffers import (
    classify_columns,
    resolve_optional_columns,
    extract_buffers,
    allocate_outputs,
    reconstruct_output,
)
```

These functions are useful for custom integrations or when building your own wrapper around `PythonToolHandle`. Refer to `python/buffers.py` and `test/test_wrapper.py` for detailed usage examples.

---

## Nested Vectors

Tools that read `std::vector<std::vector<T>>` columns (e.g. `VectorExampleTool` reading `NumTrkPt500`) are fully supported by the high-level `Tool` API. The wrapper auto-detects nested-vector inputs from `ColumnInfo` metadata and extracts the inner offsets and data buffers automatically.

Use `Tool.required_input_branch_names` to get the deduplicated list of uproot branch names to load. For nested-vector inputs the trailing `.data` suffix is stripped so the names match the actual ROOT branches:

```python
import uproot
from ColumnarToolWrapperPython import Tool

tool = Tool(
    "columnar::VectorExampleTool/vecEx",
    rename_containers={"Particles": "AnalysisJetsAuxDyn"},
)

with uproot.open("DAOD_PHYSLITE.root") as f:
    events = f["CollectionTree"].arrays(tool.required_input_branch_names, entry_stop=10)

result = tool(events)
print(result["AnalysisJetsAuxDyn.selection"].to_list())
```

At the `PythonToolHandle` level, nested-vector columns can still be set manually using the `.offset` / `.data` naming convention:

```python
handle["Met.name.offset"] = np.array([0, 9, 14], dtype=np.uint64)
handle["Met.name.data"]   = np.array([ord(c) for c in "InvisibleFinal"], dtype=np.int8)
```

**Note:** Nested-vector *outputs* (`std::vector<std::vector<T>>` written by the tool) are not yet supported by `Tool.__call__` — `allocate_outputs` raises `NotImplementedError` if it encounters one.
