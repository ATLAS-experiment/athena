# AthenaPoolExampleConverter

This package defines the persistent state representations and converters for ExampleHitContainer and ExampleTrackContainer.

## Components

### Persistent State Representations

- **ExampleHitContainer_p1**, **ExampleHit_p1**: Persistent representations for ExampleHit classes
- **ExampleTrackContainer_p1**, **ExampleTrack_p1**: Persistent representations for ExampleTrack classes

These contain corresponding data members but do not need many of the transient classes' methods and inheritance. Pointers (especially in top-level DataVector) can be replaced with data members by value, allowing better ROOT browsability.

When the transient class changes beyond what ROOT can handle automatically, a new persistent class (_p2, _p3, etc.) is introduced, and older definitions are kept to allow reading of old data.

### T-P Converters

Conversion between transient and persistent class objects:
- **ExampleHitContainerCnv_p1**, **ExampleHitCnv_p1**: Converters for ExampleHit classes
- **ExampleTrackContainerCnv_p1**, **ExampleTrackCnv_p1**: Converters for ExampleTrack classes

When a transient class changes requiring a new persistent version, a new T-P converter is created, and existing converters may be updated to create the new transient class objects.

### Gaudi Converters

The Gaudi converters for top-level classes are called when reading/writing objects:
- **Reading**: Uses the POOL class ID to dispatch the correct T-P converter
- **Writing**: Uses the latest T-P converter

## Additional Information

For more information on Athena I/O:
- [Athena I/O Documentation](https://atlas-software.docs.cern.ch/athena/io/)
