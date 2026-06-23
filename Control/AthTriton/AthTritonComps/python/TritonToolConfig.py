# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TritonToolCfg(
    flags,
    model_name: str,
    url: str,
    port: int = 8001,
    model_version: str = "",
    timeout: float = 0.0,
    ssl: bool = False,
    name="TritonTool",
    **kwargs,
):
    """Configure TritonTool in Control/AthOnnx/AthTritonComps/src"""

    acc = ComponentAccumulator()

    # Check if triton server is active
    from AthenaCommon.Logging import log as msg
    from urllib.parse import urlsplit

    split_url = urlsplit(url) if "//" in url else urlsplit(f"http://{url}")
    host = split_url.hostname
    if host == "":
        host = split_url.path
    if split_url.port is not None and split_url.port != port:
        raise RuntimeError(
            f"Triton URL is {url} and port is {port}"
            f" which differs from port in URL"
        )
    if port != 8001:
        msg.info(
            "Triton port is not set to 8001."
            " Can't assume HTTP port is 8000, not checking health."
        )
    elif ssl:
        msg.info("SSL is turned on for Triton, not checking health")
    else:
        import requests

        try:
            r = requests.get(f"http://{host}:8000/v2/health/live")
        except requests.exceptions.ConnectionError:
            raise RuntimeError(
                f"No running triton server found at http://{host}:8000/")
        if r.status_code != 200:
            raise RuntimeError(
                f"No running triton server found at http://{host}:8000/")
        if model_version != "":
            r = requests.get(
                f"http://{host}:8000/v2/models/{model_name}"
                f"/versions/{model_version}/ready"
            )
            model_str = f"{model_name} (version {model_version})"
        else:
            r = requests.get(
                f"http://{host}:8000/v2/models/{model_name}/ready")
            model_str = model_name
        if r.status_code != 200:
            raise RuntimeError(
                f"The {model_str} model is not ready for inference on {host}"
            )

    kwargs.setdefault("ModelName", model_name)
    kwargs.setdefault("URL", url)
    kwargs.setdefault("Port", port)
    kwargs.setdefault("ModelVersion", model_version)
    kwargs.setdefault("ClientTimeout", timeout)

    if port == 443:  # If the port is 443, that's typically used for HTTPS.
        ssl = True

    kwargs.setdefault("UseSSL", ssl)  # Default to not using SSL

    acc.setPrivateTools(CompFactory.AthInfer.TritonTool(name=name, **kwargs))
    return acc
