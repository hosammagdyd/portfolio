# Hosam Magdy Lotfy — Portfolio

A personal portfolio built in two parts: a real-time Qt/C++/QML application compiled to
WebAssembly, and a static site that showcases it. The goal isn't just to describe
skills — it's to demonstrate them, live, in the browser.

```
portfolio/
├── qt-showcase/   Qt 6 / C++ / QML application (the live demo)
├── site/          Static HTML/CSS/JS portfolio site that embeds it
└── .github/       CI workflow (build + test on every push)
```

---

## qt-showcase — the live demo

A ventilator-monitoring dashboard, built from scratch to demonstrate the same
architecture patterns used professionally on a real medical device HMI — without
using or referencing any proprietary source.

**What it actually demonstrates:**

- **A real state machine, not `if`/`else`.** Device status (Normal / Warning /
  Critical) is driven by a `QStateMachine` with a custom `QSignalTransition`
  subclass that checks temperature thresholds, not a chain of conditionals.
- **Genuine multithreading, with a WASM-aware fallback.** Telemetry is generated
  on a worker thread (`QThread` + `moveToThread`), kept fully isolated from the
  UI objects it eventually updates via queued signal/slot connections. Qt's
  single-threaded WebAssembly build doesn't support real threads, so the same
  code path falls back to cooperative main-thread execution via
  `#if QT_CONFIG(thread)` — correct behavior on every platform, with real
  parallelism wherever the platform actually provides it.
- **GPU-rendered waveform charts, not Canvas.** A custom `QQuickItem` subclass
  (`WaveformChartItem`) builds triangulated line geometry directly on the Qt
  Quick scene graph (`QSGGeometryNode`, `QSGFlatColorMaterial`) for an
  oscilloscope-style sweep display — three channels (pressure, flow, volume)
  rendered with physiologically-shaped waveforms modeled on public
  Assisted Volume Control (VC-A) ventilation characteristics.
- **Clean architecture.** `core/` is pure C++ domain logic (devices, state
  machine, simulator) with zero QtQuick dependency; `app/` is the QML/UI layer.
  Both compile into a single QML module to avoid the fragility of cross-module
  QML plugin loading on a shared-Qt desktop build.
- **Responsive by design.** The layout switches between a side-by-side
  master/detail view and a stacked single-column view below 700px width,
  verified against real Chrome device emulation (iPhone 14, Galaxy S25).

### Tech stack

Qt 6.8.3 (Quick, Qml, StateMachine), C++17, CMake, Qt for WebAssembly
(`wasm_singlethread`), Emscripten 3.1.56.

### Building it yourself

**Desktop:**
```bash
qt-cmake -G Ninja -S qt-showcase -B build-desktop -DCMAKE_BUILD_TYPE=Debug
cmake --build build-desktop
ctest --test-dir build-desktop --output-on-failure
```

**WebAssembly** (requires the Qt for WebAssembly `wasm_singlethread` kit and a
matching Emscripten toolchain — see the CI workflow below for exact versions):
```bash
qt-cmake -G Ninja -S qt-showcase -B qt-showcase/build/wasm_Release -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build qt-showcase/build/wasm_Release
```
`MinSizeRel` is used deliberately for the WASM build (emscripten `-Oz`) to keep
the download size down — the Controls module was also dropped entirely
(replaced `ApplicationWindow` with plain `Window`) since no actual widgets are
used, trimming the binary further.

---

## site — the portfolio page

A single-page, framework-free HTML/CSS/JS site (no build step, no bundler) that
presents the usual portfolio sections — About, Skills, Experience, Live Demo,
Projects, Education, Contact — and embeds the Qt WebAssembly demo directly.

```
site/
├── index.html        All page content/sections
├── css/styles.css     Dark/light theme (CSS custom properties), responsive layout
├── js/
│   ├── main.js        Theme toggle, mobile nav, scroll-reveal, demo lazy-load
│   └── hero.js         Animated circuit-trace/waveform canvas background
├── assets/             Résumé, favicon
├── demo/               Compiled Qt WASM output (synced from qt-showcase/build)
└── scripts/
    └── sync-demo.sh    Copies a fresh WASM build into site/demo/
```

### Demo embedding

The demo is lazy-loaded — nothing in `demo/` (the `.wasm` binary is ~19MB)
downloads until a visitor actually clicks "Launch Live Demo." On desktop/tablet
(≥760px) it loads inline, in an `<iframe>` pointing at the Qt-generated
`demo/appQtShowcase.html`, reusing Qt's own loader logic exactly rather than
reimplementing `qtLoad(...)` in the main page.

On mobile (<760px), clicking the button opens the demo **full-screen in a new
tab** instead of embedding it inline. This isn't a shortcut — it avoids a real
problem: the Qt app has its own responsive mobile layout, and squeezing that
layout into a small embedded box *inside* an already-narrow mobile page
produces a box-within-a-box squeeze (clipped text, waveform charts cut off
with no way to scroll to them). Opening it full-screen gives the app the whole
viewport it was actually designed for.

### Refreshing the embedded demo

Whenever `qt-showcase` changes and you rebuild the WASM target, sync the new
build into the site:
```bash
./site/scripts/sync-demo.sh
```

### Running the site locally

**Don't open `index.html` directly as a file.** Browsers block the WebAssembly
`fetch()` call when a page is loaded via `file://` — you'll see a Qt
"NetworkError" message if you try. Serve it instead:
```bash
cd site
python3 -m http.server 8000
```
Then open `http://localhost:8000`.

### Design notes

- Dark theme by default (teal accent), with a light-mode toggle persisted via
  `localStorage`. Falls back to the visitor's OS color-scheme preference on
  first visit.
- Scroll-reveal animations and active-nav-link highlighting via
  `IntersectionObserver` — no animation library.
- All asset paths are relative, so the site works correctly whether it's
  deployed at a domain root or a subpath.

---

## CI (`.github/workflows/qt-showcase-ci.yml`)

Runs automatically on every push/PR touching `qt-showcase/`, plus manual
dispatch. Two jobs, both on `ubuntu-latest`:

| Job | What it does |
|---|---|
| `desktop-build-and-test` | Installs Qt 6.8.3 (desktop, `linux_gcc_64`, `qtscxml` module), configures with `qt-cmake`, builds, and runs the test suite via `ctest` (`QT_QPA_PLATFORM=offscreen`, since the runner has no display). |
| `wasm-build` | Installs desktop Qt (host tooling) + Qt for WebAssembly (`wasm_singlethread`), sets up Emscripten 3.1.56, builds in `MinSizeRel`, and uploads `build-wasm/app/` as a downloadable artifact (7-day retention). |

A few non-obvious things that cost real debugging time, documented here so
they don't get rediscovered:
- The aqt module for `QStateMachine` is **`qtscxml`**, not `qtstatemachine`
  (the latter doesn't exist as an installable package).
- WASM Qt packages are published under `host: all_os, target: wasm` —
  not `host: linux, target: desktop` — because they're platform-independent
  at the package-repository level.
- Headless `ctest` runs need `QT_QPA_PLATFORM=offscreen`; without it, Qt
  tries to connect to a display that doesn't exist on the runner and aborts.

There is currently no deployment step — CI builds and verifies the WASM
artifact, but publishing it to a live domain is a deliberate, separate step
not yet wired up.

---

## Status

- ✅ Qt WASM demo: built, tested, CI-verified on both Desktop and WebAssembly targets
- ✅ Portfolio site: built, responsive, tested across desktop and mobile (real device emulation)
- ⏳ Résumé: currently `.docx` only — a PDF version is still pending
- ⏳ Domain + live deployment: not yet set up

## License

MIT — see [LICENSE](LICENSE). The Qt WASM demo's waveform shapes are modeled
on publicly documented Assisted Volume Control ventilation characteristics;
no proprietary source was used or referenced in building it.
