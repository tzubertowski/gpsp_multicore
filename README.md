
gpSP for SF2000 - Highly Optimized Fork
========================================

This is a specialized fork of libretro-gpsp, specifically optimized for the SF2000 handheld console and similar MIPS32 devices with software floating-point units (soft FPU).

**Repository Purpose:**
- Provide maximum performance on SF2000's MIPS32 CPU with soft FPU
- Implement device-specific optimizations for low-power embedded systems
- Maintain compatibility while pushing performance boundaries

**Original Project:**
This fork is based on notaz's gpSP (https://github.com/notaz/gpsp) for libretro
frontends, with contributions from davidgfnet (https://github.com/davidgfnet/gpsp).

Major SF2000 Optimizations & Features
======================================

This fork includes extensive optimizations specifically for MIPS32 soft FPU devices:

**Performance Enhancements:**
- **Selective Translation Cache Invalidation** - Intelligent cache management that only flushes affected code blocks
- **Frequency-Reduced Memory Flush System** - Optimized IWRAM/EWRAM flush operations to minimize performance overhead
- **MIPS-Specific Arithmetic Optimizations** - Replaced expensive operations with bit shifts and constants where possible
- **Soft FPU Optimizations** - Special handling for devices without hardware floating-point units
- **Enhanced Dynarec Block Management** - Increased block sizes and optimized exit limits for better performance
- **Intelligent Palette Caching** - Reduces redundant palette lookups in blend operations

**Video & Rendering:**
- **Optimized Tile Rendering** - Fast paths for common tile operations using MIPS-friendly arithmetic
- **Affine Sprite Optimizations** - Streamlined affine transformation calculations
- **Blend Operation Optimizations** - Context-aware blend skipping and caching
- **Window Processing Improvements** - Better performance for gradient and window effects
- **Cached Register Reads** - Minimizes memory access in rendering hot paths

**Audio Improvements:**
- **Conservative Sound Processing** - Balanced optimizations that maintain audio quality
- **Cycle Batching** - Extended to all soft FPU MIPS devices for consistent performance

**Device-Specific Features:**
- **Fake RTC System** - Software RTC implementation for devices without hardware clocks
- **Dynamic Speed Control** - Configurable performance/quality trade-offs
- **Partial Flush System** - Selective rendering updates for better efficiency
- **SF2000 Safe Mode Optimizations** - Device-specific safety checks to prevent crashes

**Memory Management:**
- **DMA Flush Optimizations** - Targeted flushing for DMA operations
- **Dynamic Translation Gates** - Adaptive code generation limits
- **SMC (Self-Modifying Code) Handling** - Efficient detection and recompilation

Feature list
============

gpSP features a dynamic recompiler that makes it quite fast (compared to other
emulators at least). It supports x86/x64, ARMv6/7 and ARMv8 and MIPS (32 and 64
bits), for other platforms an interpreter is available (albeit slower). Both
little and big endian systems are supported. Some supported platforms are PSP,
PS2, GameCube/Wii, Nintendo 3DS and Switch, Dingux/OpenDingux and of course
PC and Android.

At the moment this emulator lacks a native UI and must be played using some
libretro frontend (we recommend Retroarch). A list of available frontends can
be found at https://docs.libretro.com/development/frontends/

Many new features (compared to the original release) are:

 - Wireless Adapter networked multiplayer!
 - Rumble support (including Gameboy Player emulation)
 - New video renderer, fixes many graphical bugs & adds many effects (mosaic).
 - Many long standing issues have been fixed.
 - Slightly better performance (for some games at least!)
 - Better audio (fixed many audio related bugs).
 - Ships an opensource BIOS replacement,we recommend using the original though.

Planned features (aka the TODO list)
====================================

Some features I'd like to see (in loose priority order):

 - GBA link emulation (for some games, perhaps with patches).
 - Improve RFU (Wireless Adapter) emulation through research.
 - Bringing back the native UI for PC, PSP and perhaps PS2/3DS/Wii.
 - A native UI with Multiplayer support for portable devices with wifi support.
 - A better BIOS emulation and perhaps a newer better open BIOS.
 - Dynarec rewrite: make it easier to add new drcs and share more code.
 - Adding some funny DRCs like PowerPC or SH4.


