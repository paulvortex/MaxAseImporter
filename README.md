3ds Max ASE import plugin was originally created by Christer Janson at Kinetix Development as part of 3D Studio Max 7 SDK.
With several fixups and tweaks it can be compiled into a working ASE Import plugin.

Can import
------
- Triangle, normal and UV data
- Object positions, rotation, pivot
- Position/rotation/scale controllers (animation)
- Material list
- Vertex colors
- Lights
- Cameras
- Shapes
- Helpers

Whats fixed
------
- Several code cleanups to make it work with Max 2012
- Can import Quake 3 material names by detecting "textures/", "models/", "particles/", "gfx/" (base path should be supplied)
- Fixed smoothing groups and U/V import

Bugs
------
- Edges sometimes are not imported correctly
- Vertex morph animation is not supported

Build
------
To build ASE Importer you should get 3DS Max SDK and put the appropriate lib/ and include/ folders into sdk folder:
- for Max 7: sdk/SDK_Max7
- for Max 2011: sdk/SDK_Max11 (for x64: sdk/SDK_Max11/x64)
- for Max 2012: sdk/SDK_Max12 (for x64: sdk/SDK_Max12/x64)

--------------------------------------------------------------------------------
 Version History + Changelog
--------------------------------------------------------------------------------

1.01
------
- Untested support for Max 2011

1.0
------
- Initial release. Supports Max 7 and Max 2012 (x86 and x64).