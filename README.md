# watchy

Watchy is a simple framework for displaying "pages" of content on an LED
matrix panel — the time, the weather, a news ticker, whatever you like —
defined with a small XML-based markup language rather than code. A page
declares where its data comes from (a JSON API, an RSS feed, the clock) and
how to lay it out (text, images, shapes), and watchy handles fetching,
caching, templating, and rendering.

The same page definition renders identically to three different targets, so
you can build and preview pages on a laptop before ever touching real
hardware:

- **Real LED hardware** (`clock-led`), via [rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix).
- **SVG**, either a one-shot file (`page-renderer`) or a live auto-refreshing
  web preview (`panel-runtime`).
- **A terminal**, as colored ASCII art (`simulator`) — handy for a quick
  look without opening a browser.

## Programs

- `page-renderer` — renders one page's XML to an SVG file. Good for quick
  layout iteration.
- `panel-runtime` — loads a page and keeps rendering it on a timer (driving
  `<flip>` cycling, feed refreshes, and page rotation), serving a live
  auto-refreshing preview at `http://localhost:8080`.
- `simulator` — renders a page through the real data/template pipeline and
  previews it as colored ASCII in the terminal.
- `clock-led` — the real Raspberry Pi entry point, driving an actual LED
  matrix (see [Running on real LED hardware](#running-on-real-led-hardware)).

## Getting started

**Dependencies:** pugixml, libcurl, rapidjson (headers only).

```bash
# macOS
brew install pugixml curl rapidjson

# Ubuntu/Debian
apt install libpugixml-dev libcurl4-openssl-dev rapidjson-dev   # see docker/Dockerfile
```

**Build:**

```bash
cmake -S . -B build
cmake --build build
```

**Configuration:** pages read shared values from `configs/runtime/config.json`
(tracked in git — non-secret defaults like your city for weather) and
`configs/runtime/secrets.json` (gitignored — API keys). `secrets.json` isn't
checked in; create your own alongside `config.json`:

```json
{
    "openweather_appid": "your-openweathermap-api-key"
}
```

**Run:**

```bash
./build/page-renderer configs/pages/weather.xml
./build/simulator configs/pages/wordclock.xml

# Runs continuously, re-rendering on the page's own timers, and serves a
# live auto-refreshing preview at http://localhost:8080
./build/panel-runtime configs/pages/wordclock.xml [port]
```

## Writing pages

A page is an XML file with two parts: `<data>` (what to fetch) and
`<display>` (what to draw with it):

```xml
<page>
    <data>
        <feed name="weather" href="https://api.openweathermap.org/data/2.5/weather?..."/>
    </data>
    <display>
        <text x="0" y="0" font="tom-thumb">{/weather/main/temp:0}</text>
    </display>
</page>
```

### Templating

Anything in `{curly braces}` inside an attribute or text is a **JSON
pointer** into the page's data, resolved fresh on every update:

- `{/weather/main/temp}` — an absolute path, starting with `/`, reaches
  anywhere in the page's data. Built-in namespaces are always available:
  `/time` (the current clock — `hh`, `MM`, etc.), `/config` (your
  `config.json`), `/secrets` (your `secrets.json`), plus one namespace per
  `<feed name="...">`.
- Inside a `<flip>` (below), a path **without** a leading `/` is relative to
  the item currently showing.
- Add `:N` to round a numeric value to `N` decimal places instead of its raw
  formatting — e.g. `{/weather/main/temp:0}` renders `288.19` as `288`.

### Data sources

`<feed>` fetches a URL and makes its response available under
`/<name>/...`:

```xml
<feed name="weather" href="https://api.openweathermap.org/..." ttl="PT15M"/>
<feed name="news" href="http://feeds.bbci.co.uk/news/rss.xml" format="rss"/>
```

- `ttl` — an ISO-8601 duration (`PT15M` = 15 minutes, `P1D` = 1 day,
  `PT24H` = 24 hours — only day/hour/minute/second components are
  supported) for how long a fetched copy is reused before a refresh is
  attempted. Default 15 minutes. A failed refresh always falls back to the
  last successfully-fetched copy, however stale, rather than showing
  nothing — only a URL that has never once succeeded is a real failure.
- `format="rss"` parses the response as RSS 2.0 instead of the default JSON
  — converted generically by tag name, so `<channel><item><title>` becomes
  `/news/channel/items/0/title`. This works no matter what fields a given
  feed's items happen to have.

`<image>` accepts the same `ttl` (default 24 hours, since an icon URL
usually points at fixed artwork).

### Display elements

**`<text>`** draws one or more `<tspan>` lines:

```xml
<text x="0" y="0" width="64" height="64" wrap="word" overflow="clip"
      font="tom-thumb" color="white" letter-spacing="1" line-offset="7">
  <tspan>A headline that's longer than the panel is wide</tspan>
</text>
```

- `width` / `height` (optional, default unbounded) — the layout box for wrapping/clipping.
- `wrap` — `none` (default, one line per `<tspan>`) or `word` (wraps each `<tspan>` to fit `width`, using real glyph widths from the font).
- `overflow` — `visible` (default, draws past the box edges) or `clip` (hard-clips outside the box).
- `font`, `color`, `letter-spacing`, `line-offset` — self-explanatory; omitting all of the above renders as plain unwrapped text.

**`<image>`** decodes and letterboxes a PNG/JPEG into a box:

```xml
<image x="30" y="10" width="20" height="20" href="https://openweathermap.org/img/wn/{/weather/weather/0/icon}.png"/>
```

**`<rect>`** draws a filled box — `x`, `y`, `width`, `height`, `fill`.

**`<flip>`** cycles through the elements of a JSON array over time — a
ticker for a list of news items, forecast days, whatever varies in length:

```xml
<flip x="0" y="0" width="64" height="64" wrap="word" overflow="clip"
      font="tom-thumb" color="white" scroll-speed="10"
      path="/news/channel/items" period="PT8S">
  <tspan>{title}</tspan>
  <tspan>{description}</tspan>
</flip>
```

Takes the same attributes as `<text>`, plus:

- `path` — a JSON pointer to the array to cycle through.
- `period` — an ISO-8601 duration for how long each item shows before advancing. Default 5 seconds.
- `scroll-speed` (optional, default off, pixels per second) — if an item's text is taller than the box, scroll it upward at this speed instead of clipping, stopping once fully revealed. Each new item starts scrolled back to the top.
- The first item shows immediately; if the array shrinks, it clamps back to item 0 rather than erroring.

**`<attribution>`** shows a credit line when the page first loads (e.g. an
icon set's terms of use), then smoothly cross-fades into the page's real
`<display>` content:

```xml
<page>
    <attribution hold="PT3S" fade="PT2S" font="tom-thumb" color="white">
        <tspan>Icons: OpenWeatherMap</tspan>
    </attribution>
    <data>...</data>
    <display>...</display>
</page>
```

Takes the same attributes and `<tspan>`s as `<text>`, plus `hold` and
`fade` (both ISO-8601 durations, default 3s/2s) for how long it shows before
fading and how long the fade itself takes. Optional — a page with no
`<attribution>` renders exactly as if the feature didn't exist.

## Running on real LED hardware

Note: Currently this path can only be built and tested on the Pi itself.

`clock-led` runs the same page-rendering pipeline as the other apps, but by
default it just rasterizes without emitting to hardware (safe to build and
run anywhere, including your laptop). To actually drive an LED matrix:

1. Build [rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix)
   on the Pi, e.g. checked out at `/opt/rpi-rgb-led-matrix`.
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

## Hardware (bill of materials)

Parts used in the original build:

**Electronics**
- [64x64 RGB LED Matrix — 2.5mm pitch, 1/32 scan](https://www.adafruit.com/product/3649)
- [Adafruit RGB Matrix Bonnet for Raspberry Pi](https://www.adafruit.com/product/3211)
- [Male DC power adapter — 2.1mm plug to screw terminal block](https://www.adafruit.com/product/369)
- A 5V power supply, wired into the DC power adapter above
- [Raspberry Pi Zero WH (Zero W with headers)](https://www.adafruit.com/product/3708)
- A micro SD card (32GB+) for the Pi's OS

See Adafruit's own [RGB Matrix Bonnet guide](https://learn.adafruit.com/adafruit-rgb-matrix-bonnet-for-raspberry-pi/)
for wiring and setup.

## Architecture notes

A few things worth knowing if you're modifying watchy itself rather than
just writing pages:

- **Fetch caching** — every remote fetch (feeds and images) goes through a
  local cache (`cache/` at the repo root by default, gitignored). A cache
  entry is only replaced by a *successful* fetch, so a flaky network or a
  rate-limited API degrades to "showing slightly old data," never a blank
  panel.
- **Deterministic time** — every `Update()` in the pipeline takes
  `(now, deltaSeconds)` explicitly instead of reading the system clock
  itself; only the real apps' main loops read the actual clock, once per
  iteration. This is what makes the whole update pipeline (feed refresh
  timing, `<flip>` advancing, fades) testable with fixed, hand-picked
  timestamps instead of real sleeps.
- **Fade transitions** — `FadeTransitionGraphic` (used by `<attribution>`)
  does a true per-pixel cross-fade between two graphics, which needs both
  sides' actual pixel colors at once. Since the real display target is
  write-only, it renders each side into its own off-screen buffer first and
  blends the two into the real one.
