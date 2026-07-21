"""Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.

Helpers for fetching HGTD calibration values directly from CREST.

This is an interim path for Phase 1 testing on lxplus:

CREST -> Python config -> TOABinSize property

It avoids switching the whole IOVDbSvc backend to CREST before the
HGTD-specific folder wiring is finalized with Andrea/Evgeny.
"""

from __future__ import annotations

import json
import subprocess
import urllib.parse
import urllib.request


def _crest_request_json(crest_api_url: str, endpoint: str) -> dict:
    url = f"{crest_api_url.rstrip('/')}/{endpoint.lstrip('/')}"
    request = urllib.request.Request(url, headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(request) as response:
        body = response.read().decode()
    return json.loads(body) if body else {}


def _extract_toa_bin_size(payload: object) -> float:
    if isinstance(payload, dict):
        if "toa_bin_size" in payload:
            return float(payload["toa_bin_size"])
        data = payload.get("data")
        if isinstance(data, list):
            for entry in data:
                if isinstance(entry, dict) and "toa_bin_size" in entry:
                    return float(entry["toa_bin_size"])
    if isinstance(payload, list):
        for entry in payload:
            if isinstance(entry, dict) and "toa_bin_size" in entry:
                return float(entry["toa_bin_size"])
    raise RuntimeError(
        "Could not find 'toa_bin_size' in CREST payload. "
        f"Payload content was: {payload}"
    )


def fetch_hgtd_toa_bin_size_from_crest(crest_api_url: str | None, tag_name: str) -> float:
    """Fetch the latest HGTD toa_bin_size value from CREST."""
    if not crest_api_url:
        raise RuntimeError(
            "CREST API URL is not set. Export CREST_API_URL or pass CrestApiUrl explicitly."
        )
    if not tag_name:
        raise RuntimeError("CREST tag name is empty.")

    endpoint = (
        "iovs?tagname="
        f"{urllib.parse.quote(tag_name, safe='')}"
        "&page=0&size=1&sort=id.since:DESC"
    )
    iov_result = _crest_request_json(crest_api_url, endpoint)
    resources = iov_result.get("resources", [])
    if not resources:
        raise RuntimeError(
            "No IOVs were found in CREST for HGTD tag "
            f"{tag_name} at {crest_api_url}."
        )

    payload_hash = resources[0].get("payloadHash")
    if not payload_hash:
        raise RuntimeError(
            f"Latest IOV for tag {tag_name} did not contain a payloadHash: {resources[0]}"
        )

    result = subprocess.run(
        ["crestCmd", "get", "payload", "--hash", payload_hash],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(
            "crestCmd failed while fetching payload "
            f"{payload_hash}: {result.stderr or result.stdout}"
        )

    payload_text = (result.stdout or "").strip()
    if not payload_text:
        raise RuntimeError(f"crestCmd returned an empty payload for hash {payload_hash}.")

    try:
        payload = json.loads(payload_text)
    except json.JSONDecodeError as exc:
        raise RuntimeError(
            "CREST payload was not valid JSON. "
            f"Hash: {payload_hash}, payload: {payload_text[:200]}"
        ) from exc

    return _extract_toa_bin_size(payload)
