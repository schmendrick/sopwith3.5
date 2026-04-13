# Sopwith 3.5

This project is my working **Sopwith 3.5** codebase.

It starts from the Sopwith 3 source code published on SourceForge, which itself is based on the original 1984 Sopwith code, and continues from there with modern fixes and cleanup work.

## Why this repository exists

- Make the code compile reliably on... well... win11, as I will not test myself on other OS.
- Fix bugs found while rebuilding and testing the code.
- Preserve the gameplay baseline while improving reliability and maintainability.
- Extend the replay feature.
- In 2015, I also created Sopwith.NET (on SourceForge). It was an experiment to port Sopwith 3 C++ to C#. The game ran, but never got past alpha because I made the classic mistake: extending while porting.

## AI coding experiment + extensions

Another core goal of this project is to explore the practical potential of modern AI-assisted coding on a real legacy codebase. I do not plan to write much code myself (mostly). I want to see how much AI can really do.

In short, the roadmap focus is to:

- Build a stable C++ foundation.
- Make SDL and Allegro builds possible and documented (with the latter planned to be removed from `main` later).
- Make behavior deterministic and replay/test friendly.
- Add headless verification outputs for automated comparison.
- Create a reliable regression workflow.


See `sopwith3/docs/roadmap.md` for the full roadmap and phase details.

## Scope (current)

- In scope: compilability, bug fixes, deterministic replay/testing infrastructure, and tooling for reliable verification.
- Out of scope (for now): full gameplay rewrites and a possible future C# port (planned separately, yeah I kinda still have that in my head).

## Name

I refer to this project as **Sopwith 3.5** to distinguish it from the original Sopwith 3 distribution variants and to reflect the extension effort.

## Original authors and attribution

- **Sopwith (1984):** David L. Clark.
- **Sopwith 3 lineage:** Andrew Jenner and Jornand de Buisonjé (as credited in the Sopwith 3 source headers), based on David L. Clark's original Sopwith work.

This repository keeps those attributions explicit and preserves upstream notices. 

## Licensing

Yes, both license files should be acknowledged clearly.

The short version:

- `sopwith/license.txt` covers the original Sopwith source/license lineage.
- `sopwith3/copying.txt` is the GNU GPL v2 text used by the Sopwith 3 lineage/source tree.

If you redistribute binaries or source from this repo, keep both upstream license texts included and intact, and preserve all attribution/copyright notices.
