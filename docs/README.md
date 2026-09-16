# Watchy reorganization

## Programs
- clock-led: direct Raspberry Pi LED matrix clock demo.
- page-renderer: renders XML page definitions to SVG output.
- panel-runtime: schedules page updates and writes a composite output.
- simulator: renders a page through the real data/template pipeline and previews it as colored ASCII in the terminal.

## Dependencies
- pugixml, libcurl, rapidjson (headers only)
- macOS: `brew install pugixml curl rapidjson`
- Ubuntu/Debian: `apt install libpugixml-dev libcurl4-openssl-dev rapidjson-dev` (see `docker/Dockerfile`)

## Build
```bash
cmake -S . -B build
cmake --build build
```

## Run
```bash
./build/page-renderer configs/pages/weather.xml
./build/panel-runtime configs/pages/wordclock.xml
./build/simulator configs/pages/wordclock.xml
```
