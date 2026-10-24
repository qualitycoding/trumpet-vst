# Third-party notices

| Component | Version | Licence | URL | Use | Shipped |
|---|---|---|---|---|---|
| JUCE | 9.0.3 (be29c81492b6) | AGPLv3 or commercial; the binaries of this project are AGPLv3 works | https://github.com/juce-framework/JUCE | plugin framework | yes (linked into VST3/AU/Standalone) |
| VST3 SDK | 3.8 (bundled with JUCE) | MIT, Steinberg Media Technologies | https://github.com/steinbergmedia/vst3sdk | VST3 interfaces | yes (inside JUCE) |
| Catch2 | 3.16.0 | BSL-1.0 | https://github.com/catchorg/Catch2 | C++ tests | no |
| nlohmann/json | 3.12.0 | MIT | https://github.com/nlohmann/json | state and table parsing | yes |
| pluginval | 1.0.4 | GPLv3 | https://github.com/Tracktion/pluginval | plugin validation in CI | no (CI only) |
| TinySOL | 6.0 | CC BY 4.0 | https://doi.org/10.5281/zenodo.3685367 | reference recordings for realism tests, downloaded at test time | no |
| Fréour et al. 2022, Table 1 | — | CC BY 4.0 | https://doi.org/10.1051/aacus/2022004 | measured trumpet impedance poles used to validate and calibrate the bore model | data only (derived table) |
| ASIO SDK | — | not used (`JUCE_ASIO=0`) | — | — | no |

The licence texts shipped with release candidates are listed in `NOTICE` and staged by the release-candidate job (D-025).
See `ATTRIBUTION.md` for the data attributions.
