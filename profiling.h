/* gameplaySP Profiling System for Aggressive Performance Optimization
 * Designed to identify performance bottlenecks for soft FPU MIPS devices
 * 
 * Copyright (C) 2025 Performance Analysis Extension
 */

#ifndef PROFILING_H
#define PROFILING_H

#include "common.h"

#ifdef ENABLE_PROFILING

// Profiling categories for different performance bottlenecks
typedef enum {
    PROF_CYCLE_COUNTING = 0,
    PROF_MEMORY_ACCESS_TIMING,
    PROF_INSTRUCTION_DECODE,
    PROF_CACHE_INVALIDATION,
    PROF_TIMER_PRESCALING,
    PROF_VIDEO_SYNC,
    PROF_SOUND_PROCESSING,
    PROF_DYNAREC_OVERHEAD,
    PROF_REGISTER_ALLOCATION,
    PROF_BRANCH_PREDICTION,
    PROF_MAX_CATEGORIES
} prof_category_t;

// Performance counters
typedef struct {
    u64 cycle_count;
    u64 call_count;
    u64 total_time_us;
    u64 min_time_us;
    u64 max_time_us;
    const char* name;
} prof_counter_t;

// Aggressive optimization flags for different accuracy levels
typedef struct {
    // Cycle counting optimizations
    bool skip_accurate_cycle_counting;    // Skip cycle-accurate timing
    bool batch_cycle_updates;             // Update cycles in batches
    bool simplify_timer_prescaling;       // Use simplified timer math
    
    // Memory access optimizations  
    bool ignore_waitstates;               // Skip memory waitstate timing
    bool cache_memory_regions;            // Cache memory region lookups
    bool skip_alignment_checks;           // Skip memory alignment checks
    
    // Instruction decode optimizations
    bool cache_decoded_instructions;      // Cache instruction decode results
    bool skip_condition_code_updates;     // Skip some condition flag updates
    bool simplified_addressing_modes;     // Use simpler addressing mode calc
    
    // Cache optimizations
    bool reduce_cache_invalidation;       // Less aggressive cache flushing
    bool lazy_cache_validation;           // Validate cache entries lazily
    bool enlarged_cache_blocks;           // Use larger cache block sizes
    
    // MIPS-specific optimizations
    bool use_mips_multiply_fast;          // Use fast multiply on MIPS
    bool skip_fpu_accuracy;               // Skip soft FPU accuracy checks
    bool mips_branch_delay_optimize;      // Optimize branch delay slots
    
    // 2D game specific optimizations
    bool skip_3d_calculations;            // Skip 3D math for 2D games
    bool reduce_sprite_precision;         // Reduce sprite positioning precision
    bool fast_background_rendering;       // Use fast background rendering
} aggressive_opts_t;

extern prof_counter_t prof_counters[PROF_MAX_CATEGORIES];
extern aggressive_opts_t aggressive_opts;

// Profiling macros for minimal overhead
#define PROF_START(category) \
    u64 prof_start_##category = 0; \
    if (profiling_enabled) prof_start_##category = get_time_us()

#define PROF_END(category) \
    do { \
        if (profiling_enabled) { \
            u64 elapsed = get_time_us() - prof_start_##category; \
            prof_counters[category].call_count++; \
            prof_counters[category].total_time_us += elapsed; \
            if (elapsed < prof_counters[category].min_time_us || prof_counters[category].min_time_us == 0) \
                prof_counters[category].min_time_us = elapsed; \
            if (elapsed > prof_counters[category].max_time_us) \
                prof_counters[category].max_time_us = elapsed; \
        } \
    } while(0)

#define PROF_COUNTER_INC(category) \
    if (profiling_enabled) prof_counters[category].call_count++

extern bool profiling_enabled;

// Function declarations
void profiling_init(void);
void profiling_reset(void);
void profiling_print_report(void);
void profiling_set_aggressive_opts(int level);
u64 get_time_us(void);

// Specific bottleneck analysis functions
void analyze_cycle_counting_overhead(void);
void analyze_memory_access_patterns(void);
void analyze_cache_invalidation_frequency(void);
void analyze_dynarec_efficiency(void);

// Optimization suggestion system
void suggest_optimizations_for_platform(const char* platform);
void apply_kirby_specific_optimizations(void);
void apply_mips_soft_fpu_optimizations(void);

#else
// No-op macros when profiling is disabled
#define PROF_START(category)
#define PROF_END(category)
#define PROF_COUNTER_INC(category)
#define profiling_init()
#define profiling_reset()
#define profiling_print_report()
#endif

#endif // PROFILING_H