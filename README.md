# dune3d-addons

Downloadable addons for Dune 3D.

## Layout

- `addons/index.json`
- `addons/<addon-id>/addon.json`
- `addons/<addon-id>/...`

Dune 3D reads `addons/index.json` as the preferred addon catalog and falls back to scanning addon folders if it is missing.

These addons are intended to be installed into Dune 3D from the Addons window.
