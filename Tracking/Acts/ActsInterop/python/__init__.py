# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""
Wraps the ACTS unit constants loaded from a ROOT dictionary.
To use these values please use

```python
from ActsInterop import UnitConstants
value = 15*UnitConstants.mm
```
"""

# pull in the dictionary/headers with Acts Units
import ROOT
from RootUtils.LoadEnumHeader import loadEnumHeader
loadEnumHeader ('Acts/Definitions/Units.hpp')
from ROOT.Acts.UnitConstants import mm   # noqa: F401


# Forward to the enumeration.
UnitConstants = ROOT.Acts.UnitConstants
