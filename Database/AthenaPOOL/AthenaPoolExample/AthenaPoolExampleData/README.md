# AthenaPoolExampleData

This package defines toy data classes and their containers for AthenaPool examples.

## Data Classes

### ExampleHit

Used for toy ESD, contains:
- Position (3D vector using CLHEP)
- Detector string identifier

Demonstrates basic data object storage and retrieval.

### ExampleTrack

Used for toy AOD, contains:
- Track parameters (pT, eta, phi)
- Detector string identifier
- Various navigational links to ExampleHits:
  - **ElementLink**: Single links to individual hits
  - **ElementLinkVector**: Collections of links
  - **Navigable**: Navigation support for associated hits

Demonstrates complex data objects with navigational relationships.

## Containers

- **ExampleHitContainer**: DataVector container for ExampleHit objects
- **ExampleTrackContainer**: DataVector container for ExampleTrack objects

## Additional Information

For more information on Athena I/O:
- [Athena I/O Documentation](https://atlas-software.docs.cern.ch/athena/io/)
