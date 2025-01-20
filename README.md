## DirectX 11 overlay for FC2

This overlay is an alternative to the official FC2 kernel and legacy usermode overlays.

It creates a transparent window using DirectX 11 and moves it on top of the target window. Then it gets the current FC2 drawing requests via FC2T and displays them on the overlay using ImGui.

Only works on 64 bit Windows. Please use the forum thread for questions and support.

<span>Click here to watch a showcase video on Youtube:</span><br />
<a href="https://www.youtube.com/watch?v=1KvuR2Lcves" target="_blank">
	<img src="https://i.imgur.com/8yFLDcK.jpeg" alt="Showcase Video" width="560" height="315" />
</a>

#### Features

- Completely streamproof
    - Invisible in any kind of recording, screenshare or screenshot
    - Doesn't show up in the Windows task bar or the Alt+Tab program overview
- Low latency & high FPS
- Can draw on top of fullscreen windows
- If you play in windowed mode the overlay will reposition itself when you move the game window
- "Panic key" that instantly closes the overlay

#### How to use

1. Enable the Lua script on the Member's panel
2. Launch FC2 **as an administrator** and calibrate it with a supported game
3. Start the overlay **as an administrator**
4. The overlay will automatically move on top of the target window
5. Press the END key to stop the overlay process. It will also close by itself when you close the calibrated game

#### Config options

- Toggle streamproof mode (enabled by default)
- Target framerate (default value is 250 FPS)
- Debug mode
    - Draws a red rectangle around the target window client area
    - Displays a window with performance info of the overlay

## Credits

- [killtimer0](https://github.com/killtimer0/) - UIAccess PoC
- [adamhlt](https://github.com/adamhlt/) - inspiration/idea
- [ImGui](https://github.com/ocornut/imgui) - great graphics library
- [typedef](https://github.com/typedef-FC/) - FC2 & FC2T