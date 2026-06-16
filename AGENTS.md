# Agent Notes for Commander Keen Amiga RTG Port

## Build Commands

### Amiga cross-compilation (m68k-amigaos):
```bash
cd amiga && make -f Makefile.amiga
```

### Check code with lint-like checks:
```bash
cd amiga && make -f Makefile.amiga lint
```

## Project Structure

- `amiga/` - Amiga RTG port code
  - `rtg_driver.c/h` - RTG graphics backend (Picasso96/Cybergraphics)
  - `id_vw.c` - Video manager - framebuffer management, blits
  - `id_rf.c` - Refresh/rendering engine (C reimplementation of asm)
  - `id_in.c` - Input via Amiga input.device
  - `id_sd.c` - Sound via AHI
  - `id_mm.c` - Plain C memory manager
  - `id_ca.c` - Cache manager
  - `id_us.c` - User interface
  - `gelib.c` - EGA shape loading
  - `soft.c` - File loading/decompression
  - `jam_io.c` - AmigaDOS file I/O
  - `id_us_s.c` - User interface strings
- `*.c`, `*.h` (root) - Original game logic (kd_*.c, id_*.c, etc.)

## Key Porting Notes

- All DOS-specific inline assembly (`asm {}`) replaced with C
- EGA planar video memory emulated via RTG framebuffer
- VGA register access replaced with RTG API calls
- BIOS interrupts replaced with AmigaOS calls
- File I/O uses AmigaDOS instead of open/read/write
- Memory model: no far/heap distinctions; plain malloc
- Original game data files (KDR) remain unchanged

## Key Differences from DOS Original

- `GRMODE` always `EGAGR` (RTG handles mode)
- `_seg`, `far`, `huge` macros expand to nothing
- `geninterrupt()` calls removed
- `screenseg` replaced with RTG framebuffer pointer
- `linewidth` = 64 (EGA standard)
- Timing via Amiga timer.device instead of PIT
