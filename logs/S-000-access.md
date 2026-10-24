# S-000 access and environment
- Credential source: gh keyring login as qualitycoding (no harness GH_TOKEN present); scopes gist, read:org, repo, workflow. Token name/expiry: not stated. Deviation from plan wording, logged.
- Variant: no-sudo (sudo needs a password). System cmake 3.28.3, ninja 1.11.1, g++ 13.3.0, Python 3.12.3 with pip; JUCE Linux packages missing, so build/ is configured with TPT_BUILD_PLUGIN=OFF (plugin builds in CI).
- trumpet-vst: push=true, visibility=public (D-025 applies), actions runs readable (26), push dry-run OK.
- qualitycoding/knowledge and qualitycoding/Lessons: permissions.push=true, ls-remote OK.
- Hosts: see logs/S-000-hosts.txt (all reachable; 404 on objects/release-assets/files.pythonhosted roots is normal).
- verify_freeze: freeze OK.
