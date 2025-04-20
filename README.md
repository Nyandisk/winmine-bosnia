# winmine-bosnia
An internal DLL for WINMINE.EXE, that makes the game impossible. Every tile you click on, gets set to a mine before the game reveals it, therefore everything you now click on, will be a mine.

## Why?
good question

## Usage
Compile to x86 (32bit) DLL, and inject into running WINMINE process. You can press B for the toggle.
MinHook (x86) is required for compiling, link against libMinHook.x86.lib.
