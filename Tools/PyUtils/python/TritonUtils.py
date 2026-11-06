# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Utils.unixtools import find_calibfile
import re
import hashlib
from pathlib import Path

def tritonModelName(path: str | Path, prefix_length: int = 12) -> str:
    """
    Convert a file path into a unique string identifier

    The function expects the value of the path argument to be relative
    to one of the CALIBPATH entries. The full path is resolved by
    using the find_calibfile() function. If the provided path cannot
    be resolved to a full path, the function returns an empty string.
    """

    p = find_calibfile(Path(path).as_posix())
    if p is None:
        return ""

    # Use SHA256 to generate a stable, unique hash
    hash_val = hashlib.sha256(p.encode("utf-8")).hexdigest()
    parts = p.split("/")
    try:
        group_index = parts.index("GroupData")
        group_name = parts[group_index + 1]
    except (ValueError, IndexError):
        group_name = "UnknownGroup"

    # Optionally include the last path component for interpretability
    model_name = Path(p).stem[:20]    # limit length for readability
    base_name = f"{group_name}_{model_name}_{hash_val[:prefix_length]}"
    # clean up
    safe_name = re.sub(r"[^A-Za-z0-9_-]", "_", base_name)
    return safe_name
