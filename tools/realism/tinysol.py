# SPDX-License-Identifier: Apache-2.0
"""TinySOL download and note selection (D-017; C-072, C-096). STUB (D-018), implemented in S-015."""
RECORD_ID = 3685367
ARCHIVE_MD5 = "36030a7fe389da86c3419e5ee48e3b7f"
METADATA_MD5 = "a86c9bb115f69e61f2f25872e397fc4a"
CONCERT_RANGE = (54, 82)


def download(dest):
    """Idempotently download TinySOL_metadata.csv and TinySOL.tar.gz (Zenodo REST API) into dest, verify the md5s,
    extract only Brass/Trumpet_C/ordinario/*.wav. Returns the extraction root. Raises RuntimeError on md5 mismatch."""
    raise NotImplementedError("download")


def trumpet_notes(root):
    """Sorted list of dicts {path, concert, dyn} for TpC ordinario files with concert MIDI in CONCERT_RANGE, excluding the
    semitone-resampled files: the last '-' field of the file stem has a component matching (^|_)R<digits>(u|d), e.g.
    'R100u' or 'T23u_R100d' (4 files: E5 mf, G#3 ff, A#3 ff, B3 ff; C-096). Pitch from the file name."""
    raise NotImplementedError("trumpet_notes")
