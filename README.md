# winmine-bosnia
An internal DLL for WINMINE.EXE, that makes the game impossible. Every tile you click on, gets set to a mine before the game reveals it, therefore everything you now click on, will be a mine.

## Usage
Inject DLL into WINMINE.EXE process, and toggle bosnia mode with `B`.

## Compiling
Compile to x86 (32bit) DLL.
MinHook (x86) is required for compiling, link against libMinHook.x86.lib.
