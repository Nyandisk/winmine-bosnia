# winmine-bosnia
An internal DLL for WINMINE.EXE, that makes the game a massive gamble. Everything is a mine... apart from one tile.

## Usage
Inject DLL into WINMINE.EXE process, and toggle bosnia mode with `B`. (Flag will indicate the mod being active)
(for example this: https://archive.org/details/minesweeperxp)

## Compiling
Compile to 32 bit DLL in vs2022 (if you use the same project then Release x86 should do the trick)
Despite GitHub saying it is a C project, it is a C++ project, but due to MinHook also existin

## Libraries Used
 - MinHook (lib) v1.3.4 | License is in the ext folder.
