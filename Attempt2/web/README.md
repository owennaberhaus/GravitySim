# Web build

Compiles the same `src/` tree to WebAssembly + WebGL2. Nothing here is used by
the Visual Studio build; `platform_gl.h` is what keeps one source tree serving
both targets.

## Build

From WSL, once per shell:

    source ~/emsdk/emsdk_env.sh

Then:

    bash web/build.sh              # -> ~/gsbuild, for local testing
    bash web/build.sh --deploy     # -> the site repo's public/gravitysim/

Test the local build with:

    emrun --no_browser --port 8080 ~/gsbuild/gravitysim.html

and open <http://localhost:8080/gravitysim.html> in Windows Chrome. WSL has no
browser of its own, and `file://` will not work either — WebAssembly streaming
instantiation needs a real HTTP response with `Content-Type: application/wasm`.

## Deploying

`--deploy` writes `gravitysim.html`, `gravitysim.js` and `gravitysim.wasm` into
`D:\coding\personal-website\public\`. Astro copies `public/` into `dist/`
verbatim, so they land at `/gravitysim.*`. Override the location with
`SITE=/path/to/site bash web/build.sh --deploy`.

### Why the files are flat, not in a `gravitysim/` folder

`vercel.json` sets `cleanUrls: true` and `trailingSlash: false`, so the page is
served at `/gravitysim` — no trailing slash. A relative `<script src="index.js">`
on that URL resolves against the *parent* directory, giving `/index.js`, which
404s. The HTML renders and then sits on "loading" forever, because nothing ever
reports an error.

Flat names avoid it: from `/gravitysim`, the relative `gravitysim.js` resolves to
`/gravitysim.js`, which is where the file actually is.

**Never let `public/gravitysim/` and `public/gravitysim.html` both exist.** Which
one Vercel serves for `/gravitysim` is then ambiguous, and one of the two is
broken.

**Commit the site repo from Windows, not from WSL.** That repo stores LF in git
but has CRLF on disk. Git for Windows normalises on commit, so `git status` is
clean there. A WSL git with `core.autocrlf=false` sees every line of every file
as modified and would rewrite the whole repo — a 6,000 line diff around a
one-line change.

## Notes

- No `-pthread`. Threads would require `Cross-Origin-Opener-Policy` and
  `Cross-Origin-Embedder-Policy` headers on every response, and nothing in the
  sim is parallel.
- WebGL clamps `glLineWidth` to 1.0, so the orbit trails are always one pixel
  on the web no matter what `path.cpp` asks for.
- The wasm filename does not change between builds, so it must not be served
  with a long `immutable` cache. `vercel.json` pins the content type only and
  leaves revalidation to Vercel's default.
