# AthenaPoolUtilities

Utilities and data classes for POOL persistency in Athena, supporting both event data and conditions data storage.

## Main Classes

### TP-Separated Persistency

**TPObjRef** - Object reference for Athena's TP-separated persistent data model. Replaces transient pointers, inheritance, and embedding. TP converters use this to reference objects within a top-level persistent object.

**TPCnvTokenList_p1** - For extending top-level persistent objects. Extensions are written as separate POOL objects, with tokens stored in the principal object.

**TPIntegerVector_p2** - Storage and proxy for TP converters producing persistent representation as integer series.

### Conditions Data Classes

**CondAttrListCollection** - Collection of CORAL AttributeLists, each with a channel number. Used for multi-channel conditions data from COOL.

**CondAttrListVec** - Vector of AttributeLists with channel numbers and IOV ranges. Athena representation for CoraCool data.

**CondMultChanCollection** - Template for collections to be written/registered in COOL multichannel folders.

### Address Classes

Opaque addresses for POOL I/O:
- **AthenaAttrListAddress**
- **CondAttrListCollAddress**
- **CondAttrListVecAddress**

## Key Features

- **IOV Support**: Run/LumiBlock and timestamp-based intervals of validity
- **CORAL Integration**: Schema-less data storage via AttributeLists
- **Multi-channel**: Channel-based organization with per-channel IOVs
- **TP Separation**: Full support for transient-persistent separation

## Documentation

- [TP Separation](https://twiki.cern.ch/twiki/bin/view/AtlasComputing/TransientPersistentSeparation)
- [AthenaPOOL](https://twiki.cern.ch/twiki/bin/view/AtlasComputing/AthenaPool)
- [COOL Database](https://twiki.cern.ch/twiki/bin/view/AtlasComputing/CoolATLAS)
