# FlavorTagJetDecorators

Athena algorithms that pre-compute flavour-tagging variables at derivation
time. These run while the full AOD is still available, so the source
containers (truth leptons, primary vertices, electrons, etc.) can be dropped
from the derived output.

Developed for the FTAG1LITE derivation format; usable by any derivation that
needs these decorations.
