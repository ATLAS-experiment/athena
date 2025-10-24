# AthenaPoolExampleAlgorithms

This package contains example algorithms for writing and reading data objects using AthenaPool.

## Overview

The algorithms in this package demonstrate writing and reading data, tags, and conditions via AthenaPool.

## Key Algorithms

### Event Data I/O
- **WriteData**: Creates ExampleHits in an ExampleHitContainer and records them into StoreGate
- **ReadData**: Reads event data objects (ExampleHits, ExampleTracks) and demonstrates navigation through ElementLinks and Navigables
- **ReWriteData**: Reads ExampleHits and processes them into ExampleTracks with navigational relations (ElementLinks, ElementLinkVector, Navigable, WeightedNavigable)

### xAOD I/O
- **WriteExampleElectron**: Writes xAOD::ExampleElectronContainer with decorations
- **ReadExampleElectron**: Reads xAOD::ExampleElectronContainer and demonstrates selective decoration reading

### Conditions Data I/O
- **WriteCond**: Writes conditions data objects to the detector store
- **ReadCond**: Reads conditions data objects from the detector store

### Metadata I/O
- **WriteTag**: Creates and writes AthenaAttributeList for event tagging and collections
- **ReadMeta**: Reads file metadata (EventStreamInfo, EventBookkeeperCollection) using IMetaDataTool interface
- **QueryTag**: Selector tool for filtering events based on tag metadata

### Filtering
- **PassNoneFilter**: Simple filter algorithm that rejects all events (demonstrates filtering mechanism)

## Example Tests

### Simple Writing
Test writing of EventData and InFile MetaData with multiple streams and tag writing.

```bash
checkFile.py SimplePoolFile1.root
```

Expected output:
```
## opening file [SimplePoolFile1.root]...
## importing ROOT...
## importing ROOT... [DONE]
## opening file [OK]
File:SimplePoolFile1.root
Size:       34.836 kb
Nbr Events: 20

================================================================================
     Mem Size       Disk Size        Size/Evt      MissZip/Mem  items  (X) Container Name (X=Tree|Branch)
================================================================================
      72.382 kb        0.000 kb        0.000 kb        1.000       20  (T) DataHeader
--------------------------------------------------------------------------------
      46.367 kb        0.000 kb        0.000 kb        1.000       20  (B) EventInfo_p2_McEventInfo
      29.958 kb        0.000 kb        0.000 kb        1.000       20  (B) ExampleHitContainer_p1_MyHits
       9.207 kb        0.000 kb        0.000 kb        1.000        1  (B) EventStreamInfo_p1_Stream1
      50.704 kb        0.000 kb        0.000 kb        1.000        1  (T) MetaDataHdrDataHeader
================================================================================
     208.618 kb        0.000 kb        0.000 kb        0.000       20  TOTAL (POOL containers)
================================================================================
## Bye.
```

```bash
checkFile.py SimplePoolFile2.root
```

Expected output:
```
## opening file [SimplePoolFile2.root]...
## importing ROOT...
## importing ROOT... [DONE]
## opening file [OK]
File:SimplePoolFile2.root
Size:       29.271 kb
Nbr Events: 20

================================================================================
     Mem Size       Disk Size        Size/Evt      MissZip/Mem  items  (X) Container Name (X=Tree|Branch)
================================================================================
      67.502 kb        0.000 kb        0.000 kb        1.000       20  (T) DataHeader
--------------------------------------------------------------------------------
      46.367 kb        0.000 kb        0.000 kb        1.000       20  (B) EventInfo_p2_McEventInfo
       9.207 kb        0.000 kb        0.000 kb        1.000        1  (B) EventStreamInfo_p1_Stream2
      50.704 kb        0.000 kb        0.000 kb        1.000        1  (T) MetaDataHdrDataHeader
================================================================================
     173.780 kb        0.000 kb        0.000 kb        0.000       20  TOTAL (POOL containers)
================================================================================
## Bye.
```

### Appending
Test appending EventData and InFile MetaData, and tag writing in update mode.

**Note**: Appending InFile MetaData does not work in the current framework.

After appending, `checkFile.py SimplePoolFile2.root` should show:
```
## opening file [SimplePoolFile2.root]...
## importing ROOT...
## importing ROOT... [DONE]
## opening file [OK]
File:SimplePoolFile2.root
Size:       43.342 kb
Nbr Events: 40

================================================================================
     Mem Size       Disk Size        Size/Evt      MissZip/Mem  items  (X) Container Name (X=Tree|Branch)
================================================================================
      84.822 kb        0.000 kb        0.000 kb        1.000       40  (T) DataHeader
--------------------------------------------------------------------------------
      52.327 kb        0.000 kb        0.000 kb        1.000       40  (B) EventInfo_p2_McEventInfo
       9.207 kb        0.000 kb        0.000 kb        1.000        1  (B) EventStreamInfo_p1_Stream2
       9.207 kb        0.000 kb        0.000 kb        1.000        1  (B) EventStreamInfo_p1_Stream1
      51.562 kb        0.000 kb        0.000 kb        1.000        2  (T) MetaDataHdrDataHeader
================================================================================
     207.125 kb        0.000 kb        0.000 kb        0.000       40  TOTAL (POOL containers)
================================================================================
## Bye.
```

### Reading and Writing
Test reading EventData (SimplePoolFile1.root) and writing EventData with navigational relations (ElementLinks, ElementLinkVector, Navigable) to upstream EventData.

```bash
checkFile.py SimplePoolFile3.root
```

Expected output:
```
## opening file [SimplePoolFile3.root]...
## importing ROOT...
## importing ROOT... [DONE]
## opening file [OK]
File:SimplePoolFile3.root
Size:       39.700 kb
Nbr Events: 20

================================================================================
     Mem Size       Disk Size        Size/Evt      MissZip/Mem  items  (X) Container Name (X=Tree|Branch)
================================================================================
      79.462 kb        0.000 kb        0.000 kb        1.000       20  (T) DataHeader
--------------------------------------------------------------------------------
      46.367 kb        0.000 kb        0.000 kb        1.000       20  (B) EventInfo_p2_McEventInfo
      49.346 kb        0.000 kb        0.000 kb        1.000       20  (B) ExampleTrackContainer_p1_MyTracks
       9.207 kb        0.000 kb        0.000 kb        1.000        1  (B) EventStreamInfo_p1_Stream1
      50.704 kb        0.000 kb        0.000 kb        1.000        1  (T) MetaDataHdrDataHeader
================================================================================
     235.086 kb        0.000 kb        0.000 kb        0.000       20  TOTAL (POOL containers)
================================================================================
## Bye.
```

### Reading with Navigation
Test reading EventData with navigation and InFile MetaData, including event skipping.

Files read:
- SimplePoolFile1.root: EventInfo, Hits
- SimplePoolFile2.root: EventInfo
- SimplePoolFile3.root: EventInfo, Hits (via Navigation), Tracks

## Additional Information

For more information on Athena I/O:
- [Athena I/O Documentation](https://atlas-software.docs.cern.ch/athena/io/)
