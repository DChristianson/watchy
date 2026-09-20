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

`<feed>` and `<image>` elements support an optional `ttl` attribute — an
ISO-8601 duration (`PT15M` = 15 minutes, `P1D` = 1 day, `PT24H` = 24 hours,
etc. — only day/hour/minute/second components are supported, since
year/month aren't a fixed number of seconds) controlling how long a fetched
copy is reused before a refresh is attempted:

```xml
<feed name="weather" href="https://api.openweathermap.org/..." ttl="PT15M"/>
<image href="https://openweathermap.org/img/wn/{icon}.png" ttl="P1D"/>
```

Defaults: 15 minutes for feeds, 24 hours for images (an icon URL always
points at the same static artwork, so caching it aggressively is free
correctness, not staleness risk).

`<feed>` also supports `format="rss"` for real RSS 2.0 feeds (e.g. BBC's
`http://feeds.bbci.co.uk/news/rss.xml`), which is what `news.xml` uses.
Without it, a feed's response is always parsed as JSON. RSS is converted
into the same JSON shape as any other feed, generically by tag name --
`<channel><item><title>...` becomes `/<feed-name>/channel/items/0/title`,
and this works for whatever fields a given feed's `<item>`s happen to have,
not just title/description/pubDate/link:

```xml
<feed name="news" href="http://feeds.bbci.co.uk/news/rss.xml" format="rss"/>
```

`<flip>` is a text box (same `x`/`y`/`width`/`height`/`wrap`/`overflow`/
`font`/`color`/`letter-spacing`/`line-offset` as `<text>`) that cycles
through the elements of a JSON array over time instead of showing fixed
content:

```xml
<flip x="0" y="0" width="64" height="64" wrap="word" overflow="clip"
      font="tom-thumb" color="white"
      path="/news/channel/items" period="PT8S">
  <tspan>{title}</tspan>
  <tspan>{description}</tspan>
</flip>
```

- `path`: a JSON pointer to the array to cycle through.
- `period`: an ISO-8601 duration (same format as `ttl`) for how long each item shows before advancing. Default 5 seconds.
- Inside a `<flip>`, a template path that **doesn't** start with `/` is relative to the currently-showing item (`{title}` above resolves against `path`'s current element) — an absolute path (`{/time/hh}`) still reaches anywhere in the document, unaffected by which item is showing.
- The first item shows immediately; if the array shrinks so the current index is out of range, it clamps back to item 0 rather than erroring.
- `scroll-speed` (optional, default 0/off, pixels per second): when an item's rendered text is taller than the flip's `height`, it scrolls upward at this speed instead of clipping the overflow, stopping once fully revealed (it doesn't loop). Each new item starts scrolled back to the top.

A page can declare an `<attribution>` — e.g. crediting an icon set's real
terms of use — shown when the page first loads, then true-cross-faded (see
"Fade transitions" below) into the page's actual `<display>` content:

```xml
<page>
    <attribution hold="PT3S" fade="PT2S" font="tom-thumb" color="white">
        <tspan>Icons: OpenWeatherMap</tspan>
    </attribution>
    <data>...</data>
    <display>...</display>
</page>
```

- Same `font`/`color`/`x`/`y`/`width`/`height`/`letter-spacing`/`line-offset`/`wrap`/`overflow` attributes and `<tspan>` children as `<text>`.
- `hold`: an ISO-8601 duration for how long the attribution shows before the fade starts. Default 3 seconds.
- `fade`: an ISO-8601 duration for how long the cross-fade itself takes. Default 2 seconds.
- Optional — a page with no `<attribution>` renders exactly as before this feature existed.

## Fade transitions
`FadeTransitionGraphic` (`libs/watchpanel/include/watchpanel/transition.h`) is
a generic graphic that shows one child, holds it, then does a true per-pixel
cross-fade into a second child, and stays on the second one afterward — a
one-shot hand-off, not a loop. `<attribution>` (below) is the one page-XML
feature built on it so far; it isn't otherwise exposed as its own element
(no `<fade>`) yet.

Since a real cross-fade needs both sides' actual colors at once, and a
`Raster` is write-only, `FadeTransitionGraphic` owns two off-screen
`PixelBuffer`s (one per side) with their own `GraphicsContext`s — child
graphics must be constructed against `FromContext()`/`ToContext()`, not the
page's real context, so their `Draw()` renders into the off-screen buffers
that get blended into the real one:

```cpp
FadeTransitionGraphic transition(context, x, y, width, height,
                                  holdSeconds, fadeSeconds, fontPath, cacheDir);
transition.SetFromGraphic(new TextGraphic(transition.FromContext(), ...));
transition.SetToGraphic(new TextGraphic(transition.ToContext(), ...));
```

## Fetch caching
All remote fetches (JSON feeds and images) go through `hamper`'s local
cache (`cache/` at the repo root by default — configurable via the
`cacheDir` parameter on `WatchPage`/`WatchPanel`/`GraphicsContext`, and
gitignored). A cache entry is only replaced by a *successful* fetch —
**a stale entry is still served if a refresh attempt fails**, so a flaky
network or a rate-limited API degrades to "showing slightly old data"
rather than a blank panel. Only a URL that has never been fetched
successfully at all (nothing to fall back to) results in a real failure.

## Deterministic time
`WatchPage`/`WatchPanel`/`DataImport`/`Updateable`'s `Update()` all take
`(now, deltaSeconds)` explicitly rather than reading the system clock
themselves. Each component applies its own policy against these (a feed
decides whether to refetch, `FlipGraphic` decides whether to advance,
`TimeData` derives the displayed clock from `now` directly) — nothing in
the update path reads `std::time()` internally. Only the real apps'
main loops read the actual system clock (once per iteration) and pass it
down; this is what makes the whole update pipeline testable with fixed,
hand-picked timestamps instead of real sleeps (see `flip_test.cc`,
`hamper_cache_test.cc`). `Draw()` is unaffected by this — image fetching
that happens during `GraphicsContext::DrawImage` still reads the real
clock directly, since draw-time isn't part of this change.

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
