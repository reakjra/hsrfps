# hsrfps

Sets Honkai: Star Rail's frame cap to 120 by writing the game's own graphics settings value in a Wine prefix.

HSR stores its graphics settings in `HKCU\Software\Cognosphere\Star Rail` under `GraphicsSettings_Model_h2986158309`, a `REG_BINARY` holding UTF-8 JSON with one trailing NUL byte. The `FPS` field only accepts `30`, `60` and `120` as far as im aware

this reads that value, replaces the digits after `"FPS":` with `120`, and writes it back. every other graphics setting is preserved. If the entry does not exist yet (e.g, a prefix where the graphics settings were never edited and/or a new prefix) it writes a default one.

it must run as a Windows process in the same prefix as the game, and it must run *before* the game starts.

## Building

needs a mingw-w64 cross compiler:

```sh
make # crazy hard
```

## License

GPL-3.0.
