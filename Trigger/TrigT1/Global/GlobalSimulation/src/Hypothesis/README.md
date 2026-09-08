Hypothesis algs for GlobalSimulation
===

This holds source code for simulation of the Global Trigger hypothesis block.

TIP Writers
---

Global Algorithms that perform the final selection of TOBs write to the TIP word;
a 1024b word which is sent to the CTP. The bits added to the TIP indicate the number of TOBs
passing the selection. Each TIP writer writes a fixed number of bits, in a specified bit position
dependent upon the current menu configuration.

Each TIP writer is implemented as an Athena `AlgTool`, and executed by an instance
of `GlobalSimulationAlg`, and must extend the `ITIPWriterAlgTool` interface.

### Implementing a TIP writer

For an example implementation, see [`CommonMultAlgTool`](./CommonMultAlgTool.h).

It is recommended to derive from the `TIPWriterAlgTool` base class, which handles
common operations relating to the TIP word. In this case, the derived class should
obey the following rules:

```c++
/// Derive from the relevant base class and interface:
class CommonMultAlgTool: public extends<TIPWriterAlgTool,ITIPWriterAlgTool> {
...
}
```

Use the common `TIPWriterAlgTool::updateTIP()` method and implement `ITIPWriterAlgTool::countPassingTOBs()`:
```c++
/// Public methods
using TIPWriterAlgTool::updateTIP;

virtual StatusCode countPassingTOBs(const EventContext&, unsigned int& N_pass_tobs) const override;
```

Be sure to call the base class `initialize()` method in the derived class `initialize()`:
```c++
StatusCode CommonMultAlgTool::initialize() {

  ATH_CHECK( TIPWriterAlgTool::initialize() );

  // Any other initialisation here

  return StatusCode::SUCCESS;
}
```

Selectors
---

Selection logic, especially where it may be shared between different classes
selecting on the same type of TOB, may be implemented in helper classes
such as [`CommonSelector`](./CommonSelector.h), that are held as members of the
TIP writer AlgTool.
