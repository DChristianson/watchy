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
./build/simulator configs/pages/wordclock.xml

# Runs continuously, re-rendering on WatchPanel's update/page-flip timers,
# and serves a live auto-refreshing preview at http://localhost:8080
./build/panel-runtime configs/pages/wordclock.xml [port]
```

## Page schema
`<text>` elements support optional word-wrapping and overflow clipping within
a declared box:

```xml
<text x="0" y="0" width="64" height="64" wrap="word" overflow="clip" color="white">
  <tspan>A headline that's longer than the panel is wide</tspan>
</text>
```

- `width` / `height` (optional, default unbounded): the layout box for wrapping/clipping.
- `wrap`: `none` (default) — one line per `<tspan>`, no wrapping — or `word` — wraps each `<tspan>`'s text to fit `width`, using real glyph widths from the BDF font.
- `overflow`: `visible` (default) — draw past the box edges — or `clip` — hard-clip anything outside `x..x+width, y..y+height`.

Omitting all of these renders exactly as before this feature existed.

## Running on the Raspberry Pi (real LED hardware)
`clock-led` runs the same WatchPanel/LedRaster pipeline as the other apps, but
by default it just rasterizes without emitting to hardware (safe to build and
run anywhere). To actually drive an LED matrix:

1. Build [rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix)
   on the Pi, e.g. checked out at `/opt/rpi-rgb-led-matrix` (matching the
   old project `Makefile`'s convention).
2. Configure with the hardware flag on:
   ```bash
   cmake -S . -B build -DWATCHY_ENABLE_RGB_MATRIX=ON \
     -DRGB_MATRIX_ROOT=/opt/rpi-rgb-led-matrix   # only needed if not at the default path
   cmake --build build
   ```
3. Run it, passing any `--led-*` hardware flags the library understands
   (gpio mapping, rows/cols, daemon mode, etc.) plus the page to display:
   ```bash
   sudo ./build/clock-led configs/pages/wordclock.xml \
     --led-gpio-mapping=adafruit-hat-pwm --led-rows=64 --led-cols=64 --led-daemon
   ```

This hardware path can only be built and tested on the Pi itself — the
default (`WATCHY_ENABLE_RGB_MATRIX=OFF`) build is what's exercised in CI/Docker.
