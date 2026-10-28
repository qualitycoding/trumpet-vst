# SPDX-License-Identifier: Apache-2.0
"""TinySOL download and note selection (D-017; C-072, C-096). Dataset: Cella et al., Zenodo 10.5281/zenodo.3685367, CC BY 4.0."""
import hashlib
import json
import pathlib
import re
import tarfile
import urllib.request

RECORD_ID = 3685367
ARCHIVE_MD5 = "36030a7fe389da86c3419e5ee48e3b7f"
METADATA_MD5 = "a86c9bb115f69e61f2f25872e397fc4a"
CONCERT_RANGE = (54, 82)
MARKER = f".extracted-{ARCHIVE_MD5}"
API = f"https://zenodo.org/api/records/{RECORD_ID}"
PICK = re.compile(r"(^|/)Brass/Trumpet_C/ordinario/[^/]+\.wav$")
NAME = re.compile(r"TpC-ord-([A-G]#?)(\d)-(pp|mf|ff)-")
PC = {"C": 0, "C#": 1, "D": 2, "D#": 3, "E": 4, "F": 5, "F#": 6, "G": 7, "G#": 8, "A": 9, "A#": 10, "B": 11}
RESAMPLED = re.compile(r"(^|_)R\d+[ud]$")


def _md5(path):
    h = hashlib.md5()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def _file_links():
    with urllib.request.urlopen(urllib.request.Request(API, headers={"Accept": "application/json"}), timeout=60) as r:
        record = json.load(r)
    return {f["key"]: f["links"]["self"] for f in record["files"]}


def _fetch(url, part, total=None):
    """Resumable streamed download into `part` (HTTP Range); returns when the file is complete."""
    have = part.stat().st_size if part.exists() else 0
    headers = {"Range": f"bytes={have}-"} if have else {}
    with urllib.request.urlopen(urllib.request.Request(url, headers=headers), timeout=60) as r:
        if have and r.status != 206:      # the server ignored the range: start again
            have = 0
        with open(part, "ab" if have else "wb") as f:
            for block in iter(lambda: r.read(1 << 20), b""):
                f.write(block)


def download(dest):
    """Idempotently download TinySOL_metadata.csv and TinySOL.tar.gz (Zenodo REST API) into dest, verify the md5s,
    extract only Brass/Trumpet_C/ordinario/*.wav. Returns the extraction root. Raises RuntimeError on md5 mismatch."""
    dest = pathlib.Path(dest)
    dest.mkdir(parents=True, exist_ok=True)
    if (dest / MARKER).exists():
        return dest
    links = _file_links()
    meta = dest / "TinySOL_metadata.csv"
    if not meta.exists() or _md5(meta) != METADATA_MD5:
        part = dest / "TinySOL_metadata.csv.part"
        _fetch(links["TinySOL_metadata.csv"], part)
        if _md5(part) != METADATA_MD5:
            part.unlink()
            raise RuntimeError("TinySOL_metadata.csv: md5 mismatch")
        part.replace(meta)
    archive, part = dest / "TinySOL.tar.gz", dest / "TinySOL.tar.gz.part"
    if not archive.exists():
        _fetch(links["TinySOL.tar.gz"], part)
        if _md5(part) != ARCHIVE_MD5:
            part.unlink()
            raise RuntimeError("TinySOL.tar.gz: md5 mismatch")
        part.replace(archive)
    elif _md5(archive) != ARCHIVE_MD5:
        archive.unlink()
        raise RuntimeError("TinySOL.tar.gz: md5 mismatch")
    with tarfile.open(archive, "r|gz") as tar:
        for member in tar:
            if member.isfile() and PICK.search(member.name):
                tar.extract(member, dest, filter="data")
    (dest / MARKER).write_text("extracted Brass/Trumpet_C/ordinario from TinySOL.tar.gz (md5 verified)\n")
    archive.unlink()
    return dest


def trumpet_notes(root):
    """Sorted list of dicts {path, concert, dyn} for TpC ordinario files with concert MIDI in CONCERT_RANGE, excluding the
    semitone-resampled files: the last '-' field of the file stem has a component matching (^|_)R<digits>(u|d), e.g.
    'R100u' or 'T23u_R100d' (4 files: E5 mf, G#3 ff, A#3 ff, B3 ff; C-096). Pitch from the file name."""
    notes = []
    for p in sorted(pathlib.Path(root).glob("**/TpC-ord-*.wav")):
        m = NAME.search(p.name)
        if not m:
            continue
        concert = 12 * (int(m.group(2)) + 1) + PC[m.group(1)]
        if not CONCERT_RANGE[0] <= concert <= CONCERT_RANGE[1]:
            continue
        if RESAMPLED.search(p.stem.split("-")[-1]):
            continue
        notes.append({"path": str(p), "concert": concert, "dyn": m.group(3)})
    notes.sort(key=lambda n: (n["concert"], n["dyn"], n["path"]))
    return notes
