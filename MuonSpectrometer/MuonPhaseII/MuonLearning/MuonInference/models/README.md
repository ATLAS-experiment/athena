# MuonInference models

Place your ONNX models here for local testing. The official models are stored on ATLAS EOS
and are resolved at runtime via PathResolver in Athena.

Typical usage:
- Name your model files `*.onnx`.
- It will locate them with `PathResolverFindDataFile("MuonInference/models/<your>.onnx")`.
