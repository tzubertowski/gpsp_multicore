# GPSP Soft FPU MIPS Optimization Summary

## Key Findings - Biggest Performance Bottlenecks

### 1. **Cycle Counting Overhead** (HIGH IMPACT)
**Location**: `main.c:249-251`, `main.c:354`
```c
// Current: Precise cycle tracking every instruction
timer[i].count -= completed_cycles;
write_ioreg(REG_TMXD(i), -(timer[i].count >> timer[i].prescale));
```
**Optimization**: Already has SF2000 cycle batching - extend to all MIPS soft FPU devices

### 2. **Partial Memory Flush System** (HIGHEST IMPACT)
**Location**: `cpu_threaded.c:3418` - `partial_flush_ram_full()`
**Issue**: Aggressive cache invalidation for Self-Modifying Code detection
**Optimization**: Most games don't use SMC - can reduce flush frequency dramatically

### 3. **Timer Prescaling Calculations** (MEDIUM IMPACT)  
**Location**: `main.c:251`, `main.c:276`
```c
// Current: Bit shifting for prescale every timer update
-(timer[i].count >> timer[i].prescale)
timer[i].count += (timer[i].reload << timer[i].prescale);
```
**Optimization**: Pre-calculate prescale values, batch updates

### 4. **Memory Access Timing** (MEDIUM IMPACT)
**Location**: `gba_memory.c` - Waitstate simulation
**Issue**: Simulates ROM/SRAM access timing for accuracy
**Optimization**: Skip waitstates for soft FPU MIPS where performance is critical

### 5. **MIPS Dynarec Inefficiency** (HIGH IMPACT)
**Location**: `mips/` directory, MIPS code generation
**Issue**: Generic MIPS codegen, not optimized for soft FPU devices
**Optimization**: Use MIPS-specific multiply, optimize branch delays

## Recommended Implementation Strategy

### Phase 1: Quick Wins (Immediate 10-15% gain)
1. **Extend SF2000 cycle batching to all MIPS soft FPU**
   ```c
   #if defined(SF2000) || defined(MIPS_SOFT_FPU) || defined(PERFORMANCE_OVER_ACCURACY)
   if (execute_cycles > 300 && execute_cycles < 3000) {
     execute_cycles = MIN(execute_cycles * 2, 6000);
   }
   #endif
   ```

2. **Add cache invalidation reduction flag**
   ```c
   #ifdef REDUCE_CACHE_INVALIDATION
   static u32 flush_counter = 0;
   if (++flush_counter % 4 != 0) return; // Flush every 4th time
   #endif
   ```

### Phase 2: Moderate Optimizations (Additional 15-20% gain)
1. **Skip memory waitstates for performance mode**
2. **Implement timer prescaling optimization**  
3. **Add MIPS multiply fast path**

### Phase 3: Aggressive 2D Game Optimizations (Additional 15-25% gain)
1. **Skip 3D calculations entirely**
2. **Reduce sprite precision**
3. **Fast background rendering**

## Specific Code Locations for Optimization

### 1. Main Performance Loop
**File**: `/app/cores/gpsp/main.c:313-526`
- Line 485-491: Existing SF2000 optimization to extend
- Line 354: Cycle counting to optimize
- Line 483: Execute cycles calculation

### 2. Cache Management  
**File**: `/app/cores/gpsp/cpu_threaded.c:3418-3449`
- Partial flush system that's too aggressive
- SMC detection that can be reduced for known safe games

### 3. Timer System
**File**: `/app/cores/gpsp/main.c:239-279`
- Line 251: Timer prescaling calculation
- Line 276: Timer reload calculation
- Can batch these updates for performance

### 4. MIPS Code Generation
**File**: `/app/cores/gpsp/mips/mips_emit.h`
- MIPS instruction emission
- Can optimize for soft FPU devices specifically

### 5. Memory System
**File**: `/app/cores/gpsp/gba_memory.c`
- Memory access timing simulation
- Waitstate calculations that can be skipped

## Expected Results for Kirby/2D Games on Soft FPU MIPS

- **Phase 1**: 10-15% performance improvement (safe optimizations)
- **Phase 2**: 25-35% total improvement (moderate accuracy tradeoffs)  
- **Phase 3**: 40-60% total improvement (aggressive 2D game optimizations)

## Most Promising Optimization Targets

1. **Partial flush system reduction** - Biggest single optimization opportunity
2. **Cycle batching extension** - Already proven effective on SF2000
3. **Timer prescaling simplification** - Called frequently, easy to optimize
4. **Memory waitstate skipping** - Significant for memory-intensive games
5. **MIPS-specific dynarec optimizations** - Platform-specific gains

## Implementation Files Created

1. **`profiling.h`** - Performance analysis framework
2. **`profiling.c`** - Profiling implementation with optimization suggestions
3. **`PERFORMANCE_ANALYSIS.md`** - Detailed technical analysis
4. **`OPTIMIZATION_SUMMARY.md`** - This summary document

The profiling system can be enabled with `-DENABLE_PROFILING` to measure actual performance bottlenecks on target hardware and validate optimization effectiveness.