/* gameplaySP Profiling System Implementation
 * Aggressive Performance Optimization for Soft FPU MIPS Devices
 * 
 * Copyright (C) 2025 Performance Analysis Extension
 */

#include "profiling.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#ifdef ENABLE_PROFILING

bool profiling_enabled = false;

// Initialize profiling counters with names
prof_counter_t prof_counters[PROF_MAX_CATEGORIES] = {
    {0, 0, 0, 0, 0, "Cycle Counting"},
    {0, 0, 0, 0, 0, "Memory Access Timing"},
    {0, 0, 0, 0, 0, "Instruction Decode"},
    {0, 0, 0, 0, 0, "Cache Invalidation"},
    {0, 0, 0, 0, 0, "Timer Prescaling"},
    {0, 0, 0, 0, 0, "Video Sync"},
    {0, 0, 0, 0, 0, "Sound Processing"},
    {0, 0, 0, 0, 0, "Dynarec Overhead"},
    {0, 0, 0, 0, 0, "Register Allocation"},
    {0, 0, 0, 0, 0, "Branch Prediction"}
};

// Aggressive optimization settings
aggressive_opts_t aggressive_opts = {
    .skip_accurate_cycle_counting = false,
    .batch_cycle_updates = false,
    .simplify_timer_prescaling = false,
    .ignore_waitstates = false,
    .cache_memory_regions = false,
    .skip_alignment_checks = false,
    .cache_decoded_instructions = false,
    .skip_condition_code_updates = false,
    .simplified_addressing_modes = false,
    .reduce_cache_invalidation = false,
    .lazy_cache_validation = false,
    .enlarged_cache_blocks = false,
    .use_mips_multiply_fast = false,
    .skip_fpu_accuracy = false,
    .mips_branch_delay_optimize = false,
    .skip_3d_calculations = false,
    .reduce_sprite_precision = false,
    .fast_background_rendering = false
};

void profiling_init(void) {
    profiling_enabled = true;
    profiling_reset();
    printf("[PROFILING] Performance analysis enabled for MIPS soft FPU optimization\n");
}

void profiling_reset(void) {
    for (int i = 0; i < PROF_MAX_CATEGORIES; i++) {
        prof_counters[i].cycle_count = 0;
        prof_counters[i].call_count = 0;
        prof_counters[i].total_time_us = 0;
        prof_counters[i].min_time_us = 0;
        prof_counters[i].max_time_us = 0;
    }
}

u64 get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (u64)tv.tv_sec * 1000000 + tv.tv_usec;
}

void profiling_print_report(void) {
    if (!profiling_enabled) return;
    
    printf("\n=== GPSP PERFORMANCE ANALYSIS REPORT ===\n");
    printf("Platform: MIPS Soft FPU Device\n");
    printf("Target: Aggressive Accuracy vs Performance Tradeoffs\n\n");
    
    printf("%-25s %12s %12s %12s %12s %12s\n", 
           "Category", "Calls", "Total (us)", "Avg (us)", "Min (us)", "Max (us)");
    printf("--------------------------------------------------------------------------------\n");
    
    u64 total_time = 0;
    for (int i = 0; i < PROF_MAX_CATEGORIES; i++) {
        prof_counter_t *c = &prof_counters[i];
        total_time += c->total_time_us;
        
        if (c->call_count > 0) {
            u64 avg_time = c->total_time_us / c->call_count;
            printf("%-25s %12llu %12llu %12llu %12llu %12llu\n",
                   c->name, c->call_count, c->total_time_us, avg_time, c->min_time_us, c->max_time_us);
        }
    }
    
    printf("--------------------------------------------------------------------------------\n");
    printf("Total profiled time: %llu us\n\n", total_time);
    
    // Analyze bottlenecks and suggest optimizations
    analyze_performance_bottlenecks();
}

void analyze_performance_bottlenecks(void) {
    printf("=== PERFORMANCE BOTTLENECK ANALYSIS ===\n\n");
    
    u64 total_time = 0;
    for (int i = 0; i < PROF_MAX_CATEGORIES; i++) {
        total_time += prof_counters[i].total_time_us;
    }
    
    if (total_time == 0) {
        printf("No performance data collected yet.\n");
        return;
    }
    
    // Identify top bottlenecks
    printf("Top Performance Bottlenecks:\n");
    for (int i = 0; i < PROF_MAX_CATEGORIES; i++) {
        if (prof_counters[i].total_time_us > 0) {
            double percentage = (double)prof_counters[i].total_time_us / total_time * 100.0;
            if (percentage > 5.0) {
                printf("  %s: %.1f%% of total time\n", prof_counters[i].name, percentage);
                suggest_category_optimizations(i, percentage);
            }
        }
    }
    
    printf("\n=== AGGRESSIVE OPTIMIZATION RECOMMENDATIONS ===\n");
    suggest_mips_optimizations();
    suggest_2d_game_optimizations();
    suggest_accuracy_tradeoffs();
}

void suggest_category_optimizations(int category, double percentage) {
    printf("    -> ");
    switch (category) {
        case PROF_CYCLE_COUNTING:
            printf("Consider: batch cycle updates, skip micro-cycle accuracy\n");
            break;
        case PROF_MEMORY_ACCESS_TIMING:
            printf("Consider: ignore waitstates, cache region lookups\n");
            break;
        case PROF_INSTRUCTION_DECODE:
            printf("Consider: cache decoded instructions, simplify addressing\n");
            break;
        case PROF_CACHE_INVALIDATION:
            printf("Consider: reduce flush frequency, lazy validation\n");
            break;
        case PROF_TIMER_PRESCALING:
            printf("Consider: simplified timer math, batch timer updates\n");
            break;
        case PROF_DYNAREC_OVERHEAD:
            printf("Consider: larger translation blocks, reduced invalidation\n");
            break;
        default:
            printf("Category-specific optimizations available\n");
            break;
    }
}

void suggest_mips_optimizations(void) {
    printf("\nMIPS Soft FPU Specific Optimizations:\n");
    printf("  1. Enable fast multiply operations (skip accuracy checks)\n");
    printf("  2. Optimize branch delay slot handling\n");
    printf("  3. Use MIPS-specific instruction combining\n");
    printf("  4. Reduce soft FPU precision for non-critical operations\n");
    printf("  5. Use larger dynarec cache blocks for MIPS\n");
}

void suggest_2d_game_optimizations(void) {
    printf("\n2D Game Specific Optimizations (e.g., Kirby):\n");
    printf("  1. Skip 3D transformation calculations\n");
    printf("  2. Reduce sprite positioning precision\n");
    printf("  3. Use fast background rendering modes\n");
    printf("  4. Simplify alpha blending calculations\n");
    printf("  5. Cache frequently used tile patterns\n");
}

void suggest_accuracy_tradeoffs(void) {
    printf("\nAccuracy vs Performance Tradeoffs:\n");
    printf("  Level 1 (Safe): Batch cycle updates, cache memory regions\n");
    printf("  Level 2 (Moderate): Skip waitstates, simplify timers\n");
    printf("  Level 3 (Aggressive): Skip condition flags, reduce cache invalidation\n");
    printf("  Level 4 (Extreme): Skip alignment checks, simplified addressing\n");
    printf("\nRecommendation: Start with Level 2 for 2D games on soft FPU MIPS\n");
}

void profiling_set_aggressive_opts(int level) {
    switch (level) {
        case 1: // Safe optimizations
            aggressive_opts.batch_cycle_updates = true;
            aggressive_opts.cache_memory_regions = true;
            aggressive_opts.cache_decoded_instructions = true;
            break;
            
        case 2: // Moderate optimizations
            profiling_set_aggressive_opts(1);
            aggressive_opts.ignore_waitstates = true;
            aggressive_opts.simplify_timer_prescaling = true;
            aggressive_opts.reduce_cache_invalidation = true;
            aggressive_opts.use_mips_multiply_fast = true;
            break;
            
        case 3: // Aggressive optimizations
            profiling_set_aggressive_opts(2);
            aggressive_opts.skip_condition_code_updates = true;
            aggressive_opts.lazy_cache_validation = true;
            aggressive_opts.mips_branch_delay_optimize = true;
            aggressive_opts.fast_background_rendering = true;
            break;
            
        case 4: // Extreme optimizations
            profiling_set_aggressive_opts(3);
            aggressive_opts.skip_accurate_cycle_counting = true;
            aggressive_opts.skip_alignment_checks = true;
            aggressive_opts.simplified_addressing_modes = true;
            aggressive_opts.skip_fpu_accuracy = true;
            aggressive_opts.skip_3d_calculations = true;
            aggressive_opts.reduce_sprite_precision = true;
            break;
    }
    
    printf("[PROFILING] Applied optimization level %d\n", level);
}

void analyze_cycle_counting_overhead(void) {
    printf("\n=== CYCLE COUNTING ANALYSIS ===\n");
    printf("Current cycle counting calls: %llu\n", prof_counters[PROF_CYCLE_COUNTING].call_count);
    printf("Average cycle counting time: %llu us\n", 
           prof_counters[PROF_CYCLE_COUNTING].call_count > 0 ? 
           prof_counters[PROF_CYCLE_COUNTING].total_time_us / prof_counters[PROF_CYCLE_COUNTING].call_count : 0);
    
    if (prof_counters[PROF_CYCLE_COUNTING].call_count > 10000) {
        printf("RECOMMENDATION: High cycle counting overhead detected!\n");
        printf("  - Consider batching cycle updates\n");
        printf("  - Skip micro-cycle accuracy for 2D games\n");
        printf("  - Use simplified timing for soft FPU MIPS\n");
    }
}

void analyze_memory_access_patterns(void) {
    printf("\n=== MEMORY ACCESS ANALYSIS ===\n");
    printf("Memory access timing calls: %llu\n", prof_counters[PROF_MEMORY_ACCESS_TIMING].call_count);
    
    if (prof_counters[PROF_MEMORY_ACCESS_TIMING].total_time_us > 0) {
        printf("RECOMMENDATION: Memory access timing overhead detected!\n");
        printf("  - Skip ROM/SRAM waitstate simulation\n");
        printf("  - Cache memory region type lookups\n");
        printf("  - Skip alignment check penalties\n");
    }
}

void analyze_cache_invalidation_frequency(void) {
    printf("\n=== CACHE INVALIDATION ANALYSIS ===\n");
    printf("Cache invalidation calls: %llu\n", prof_counters[PROF_CACHE_INVALIDATION].call_count);
    
    if (prof_counters[PROF_CACHE_INVALIDATION].call_count > 1000) {
        printf("RECOMMENDATION: Excessive cache invalidation detected!\n");
        printf("  - Reduce partial flush frequency\n");
        printf("  - Use lazy cache validation\n");
        printf("  - Enlarge cache block sizes\n");
        printf("  - Skip SMC detection for ROM areas\n");
    }
}

void analyze_dynarec_efficiency(void) {
    printf("\n=== DYNAREC EFFICIENCY ANALYSIS ===\n");
    printf("Dynarec overhead calls: %llu\n", prof_counters[PROF_DYNAREC_OVERHEAD].call_count);
    
    if (prof_counters[PROF_DYNAREC_OVERHEAD].total_time_us > 0) {
        printf("RECOMMENDATION: Dynarec overhead detected!\n");
        printf("  - Increase translation cache sizes\n");
        printf("  - Use larger basic block sizes\n");
        printf("  - Reduce cache invalidation frequency\n");
        printf("  - Optimize MIPS code generation\n");
    }
}

void apply_kirby_specific_optimizations(void) {
    printf("[PROFILING] Applying Kirby-specific optimizations\n");
    
    // Kirby games are 2D, so we can be very aggressive
    aggressive_opts.skip_3d_calculations = true;
    aggressive_opts.fast_background_rendering = true;
    aggressive_opts.reduce_sprite_precision = true;
    aggressive_opts.ignore_waitstates = true;
    aggressive_opts.simplify_timer_prescaling = true;
    
    printf("  - Disabled 3D calculations\n");
    printf("  - Enabled fast 2D rendering\n");
    printf("  - Reduced sprite precision\n");
    printf("  - Simplified timing accuracy\n");
}

void apply_mips_soft_fpu_optimizations(void) {
    printf("[PROFILING] Applying MIPS soft FPU optimizations\n");
    
    aggressive_opts.use_mips_multiply_fast = true;
    aggressive_opts.skip_fpu_accuracy = true;
    aggressive_opts.mips_branch_delay_optimize = true;
    aggressive_opts.batch_cycle_updates = true;
    
    printf("  - Enabled fast MIPS multiply\n");
    printf("  - Reduced soft FPU accuracy\n");
    printf("  - Optimized branch delay slots\n");
    printf("  - Batched cycle counting\n");
}

void suggest_optimizations_for_platform(const char* platform) {
    printf("\n=== PLATFORM-SPECIFIC OPTIMIZATIONS ===\n");
    printf("Platform: %s\n", platform);
    
    if (strstr(platform, "mips") || strstr(platform, "MIPS")) {
        suggest_mips_optimizations();
        
        if (strstr(platform, "soft") && strstr(platform, "fpu")) {
            printf("\nSoft FPU detected - additional optimizations:\n");
            printf("  - Reduce floating point precision\n");
            printf("  - Use integer approximations where possible\n");
            printf("  - Skip FPU state saves for simple operations\n");
        }
    }
    
    printf("\nFor 2D games like Kirby, consider applying Level 3-4 optimizations\n");
}

#endif // ENABLE_PROFILING