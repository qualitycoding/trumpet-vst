# Deviations
- S-002 (DR-SECURITY): `pip-audit` reported 4 vulnerabilities (certifi 2026.7.22; filelock 4.0.12). Both are test-only
  tooling. Upgraded within the same major version: certifi 2026.10.19, filelock 4.2.1; lock and hashes regenerated;
  re-audit clean (`logs/S-002/pip-audit*.txt`).
- S-000: credential source is the gh keyring login (account qualitycoding), not a harness `GH_TOKEN`.
