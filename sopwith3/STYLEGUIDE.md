# Sopwith 3 — observed source layout

This file **only** records conventions that already appear in `sopwith3/src`. It is **not** a list of goals or policies for future edits.

## Indentation

- C/C++ sources use **2 spaces** per indent level (e.g. `bird.cpp`, `def.h`).
- **`Makefile.msys2`** uses **tabs** in recipes (make convention).

## Braces

- **Functions / methods**: opening `{` on the line after the signature (e.g. `bird.cpp`: `Bird::Bird`, `Bird::update`, `main.cpp`: `main`).
- **`if` / `for` / `while`**: opening `{` often on the **same line** as the keyword (e.g. `bird.cpp`).
- **Small inline functions** in anonymous namespaces sometimes put `{` on the same line as the name (e.g. `def.h`: `screen_center_x`, `tostring`).

## Spacing

- **Commas** often have **no space** after them in parameters and calls (e.g. `main.cpp`: `int argc,char* argv[]`, `start(argc,argv)`).
- **Enum / constant style** often uses **no spaces** around `=` (e.g. `def.h`: `KEYBOARD=1`, `MAX_X=3000`).
- **Operators** are often written **tight** (e.g. `bird.cpp`: `life==-1`, `framecounter&7`).
- **Pointers**: `Type* name` (space before `*`; e.g. `bird.cpp`, `main.cpp`).

## Files

- Sources carry the **standard GPL header block** (same shape across `.cpp` / `.h`).
- **Include guards** use `#ifndef SOPWITH_…` / `#define SOPWITH_…` / `#endif /* SOPWITH_… */`.
- **Include order** varies by file; examples: `bird.cpp` includes `bird.h` first then other project headers; `def.h` pulls system headers after the guard.

## Names (as they appear)

- **Types / classes**: `CamelCase` (e.g. `Bird`, `Gamemode` in `def.h`).
- **Functions**: mixed (`start`, `spritedata`, `instanceof`, member names like `update`, `docollision`).
- **Enum lists** may be **dense** (no space after commas inside braces): e.g. `def.h` `enum Gamemode {NO_GAMEMODE,SINGLE,...}`.

## Control flow

- **`else`** is sometimes placed without extra braces on the following line (e.g. `bird.cpp` `update()`).
- **`dynamic_cast` / `static_cast` / `reinterpret_cast`** appear where the code already uses them.

## String literals and macros

- Help text in `sopwith.cpp` builds a string with **`__DATE__` / `__TIME__`** using **adjacent string literals** (spaces between `"…"`, `__DATE__`, etc.).

## Tooling (separate from code style)

- **[`.editorconfig`](.editorconfig)** — editor defaults for this repo.
