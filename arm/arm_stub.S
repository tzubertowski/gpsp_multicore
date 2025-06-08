#include "../gpsp_config.h"

#define defsymbl(symbol) \
.align 2;                \
.type symbol, %function ;\
.global symbol ;         \
.global _##symbol ;      \
symbol:                  \
_##symbol:

.text
.align 2

#define REG_R0            (0 * 4)
#define REG_R1            (1 * 4)
#define REG_R2            (2 * 4)
#define REG_R3            (3 * 4)
#define REG_R4            (4 * 4)
#define REG_R5            (5 * 4)
#define REG_R6            (6 * 4)
#define REG_R7            (7 * 4)
#define REG_R8            (8 * 4)
#define REG_R9            (9 * 4)
#define REG_R10           (10 * 4)
#define REG_R11           (11 * 4)
#define REG_R12           (12 * 4)
#define REG_R13           (13 * 4)
#define REG_R14           (14 * 4)
#define REG_SP            (13 * 4)
#define REG_LR            (14 * 4)
#define REG_PC            (15 * 4)
#define REG_CPSR          (16 * 4)
#define CPU_MODE          (17 * 4)
#define CPU_HALT_STATE    (18 * 4)

#define REG_BUS_VALUE     (19 * 4)
#define REG_N_FLAG        (20 * 4)
#define REG_Z_FLAG        (21 * 4)
#define REG_C_FLAG        (22 * 4)
#define REG_V_FLAG        (23 * 4)
#define REG_SLEEP_CYCLES  (24 * 4)
#define OAM_UPDATED       (25 * 4)

#define CPU_ALERT_HALT    (1 << 0)
#define CPU_ALERT_SMC     (1 << 1)
#define CPU_ALERT_IRQ     (1 << 2)

#define reg_a0            r0
#define reg_a1            r1
#define reg_a2            r2

#define reg_base          r11
#define reg_flags         r9

#define reg_cycles        r12

#define reg_x0            r3
#define reg_x1            r4
#define reg_x2            r5
#define reg_x3            r6
#define reg_x4            r7
#define reg_x5            r8


#define MODE_SUPERVISOR   0x13
#define SUPERVISOR_OFFSET 0x03

@ Memory offsets from reg_base to the different buffers
#define IWRAM_OFF      -0xA8000
#define VRAM_OFF       -0x98000
#define EWRAM_OFF      -0x80000
#define SPSR_RAM_OFF      0x100
#define STORE_TBL_OFF     0x118
#define REGMODE_RAM_OFF   0x400
#define OAM_RAM_OFF       0x500
#define PAL_RAM_OFF       0x900
#define RDMAP_OFF         0xD00
#define IOREG_OFF        0x8D00
#define PAL_CONV_OFF     0x9100


#if __ARM_ARCH >= 6
  #define extract_u16(rd, rs) \
    uxth rd, rs
#else
  #define extract_u16(rd, rs) \
    bic rd, rs, #0xff000000;  \
    bic rd, rd, #0x00ff0000
#endif

@ Will load the register set from memory into the appropriate cached registers.
@ See arm_emit.h for listing explanation.

#define load_registers_arm()                                                 ;\
  ldr reg_x0, [reg_base, #REG_R0]                                            ;\
  ldr reg_x1, [reg_base, #REG_R1]                                            ;\
  ldr reg_x2, [reg_base, #REG_R6]                                            ;\
  ldr reg_x3, [reg_base, #REG_R9]                                            ;\
  ldr reg_x4, [reg_base, #REG_R12]                                           ;\
  ldr reg_x5, [reg_base, #REG_R14]                                           ;\

#define load_registers_thumb()                                               ;\
  ldm reg_base, {reg_x0, reg_x1, reg_x2, reg_x3, reg_x4, reg_x5}


@ Will store the register set from cached registers back to memory.

#define store_registers_arm()                                                ;\
  str reg_x0, [reg_base, #REG_R0]                                            ;\
  str reg_x1, [reg_base, #REG_R1]                                            ;\
  str reg_x2, [reg_base, #REG_R6]                                            ;\
  str reg_x3, [reg_base, #REG_R9]                                            ;\
  str reg_x4, [reg_base, #REG_R12]                                           ;\
  str reg_x5, [reg_base, #REG_R14]                                           ;\

#define store_registers_thumb()                                              ;\
  stm reg_base, {reg_x0, reg_x1, reg_x2, reg_x3, reg_x4, reg_x5}

#define store_registers_cond()                                               ;\
  stmne reg_base, {reg_x0, reg_x1, reg_x2, reg_x3, reg_x4, reg_x5}           ;\
  bne 80f                                                                    ;\
  str reg_x0, [reg_base, #REG_R0]                                            ;\
  str reg_x1, [reg_base, #REG_R1]                                            ;\
  str reg_x2, [reg_base, #REG_R6]                                            ;\
  str reg_x3, [reg_base, #REG_R9]                                            ;\
  str reg_x4, [reg_base, #REG_R12]                                           ;\
  str reg_x5, [reg_base, #REG_R14]                                           ;\
  80:

@ Returns an updated persistent cpsr with the cached flags register.
@ Uses reg as a temporary register and returns the CPSR here.

#define collapse_flags_no_update(reg)                                        ;\
  ldr reg, [reg_base, #REG_CPSR]          /* reg = cpsr                    */;\
  bic reg, reg, #0xF0000000               /* clear ALU flags in cpsr       */;\
  and reg_flags, reg_flags, #0xF0000000   /* clear non-ALU flags           */;\
  orr reg, reg, reg_flags                 /* update cpsr with ALU flags    */;\

@ Updates cpsr using the above macro.

#define collapse_flags(reg)                                                  ;\
  collapse_flags_no_update(reg)                                              ;\
  str reg, [reg_base, #REG_CPSR]                                             ;\

@ Loads the saved flags register from the persistent cpsr.

#define extract_flags()                                                      ;\
  ldr reg_flags, [reg_base, #REG_CPSR]                                       ;\
  msr cpsr_f, reg_flags                                                      ;\


#define save_flags()                                                         ;\
  mrs reg_flags, cpsr                                                        ;\

#define restore_flags()                                                      ;\
  msr cpsr_f, reg_flags                                                      ;\

@ Align the stack to 64 bits (ABIs that don't require it, still recommend so)
#define call_c_saved_regs r2, r3, r12, lr

@ Calls a C function - reloads the stack pointer and saves all caller save
@ registers which are important to the dynarec.

#define call_c_function(function)                                            ;\
  stmdb sp!, { call_c_saved_regs }                                           ;\
  bl function                                                                ;\
  ldmia sp!, { call_c_saved_regs }                                           ;\


@ Update the GBA hardware (video, sound, input, etc)

@ Input:
@ r0: current PC

#define return_straight()   bx lr
#define return_add()        add pc, lr, #4

#define load_pc_straight()  ldr r0, [lr, #-12]
#define load_pc_add()       ldr r0, [lr]

#define cycles_straight()  mvn r0, reg_cycles
#define cycles_add()       mov r0, #0

#define arm_update_gba_builder(name, mode, return_op)                        ;\
                                                                             ;\
defsymbl(arm_update_gba_##name)                                              ;\
  load_pc_##return_op()                                                      ;\
  str r0, [reg_base, #REG_PC]             /* write out the PC              */;\
                                                                             ;\
  save_flags()                                                               ;\
  collapse_flags(r0)                      /* update the flags              */;\
                                                                             ;\
  store_registers_##mode()                /* save out registers            */;\
  cycles_##return_op()                    /* remaining cycles in arg0      */;\
  call_c_function(update_gba)             /* update GBA state              */;\
                                                                             ;\
  cmp r0, #0                              /* check MSB for frame completion*/;\
  blt return_to_main                                                         ;\
                                                                             ;\
  bic reg_cycles, r0, #0xF0000000         /* clear MSB, not part of count  */;\
  mvn reg_cycles, reg_cycles              /* we count negative to zero     */;\
                                                                             ;\
  tst r0, #0x40000000                     /* set if PC changed             */;\
  bne 1f                                  /* go jump/translate             */;\
                                                                             ;\
  load_registers_##mode()                 /* reload registers              */;\
  restore_flags()                                                            ;\
  return_##return_op()                    /* continue, no PC change        */;\
                                                                             ;\
1:                                                                           ;\
  ldr r1, [reg_base, #REG_CPSR]           /* r1 = flags                    */;\
  ldr r0, [reg_base, #REG_PC]             /* load new PC                   */;\
  tst r1, #0x20                           /* see if Thumb bit is set       */;\
  bne 2f                                  /* if so load Thumb PC           */;\
                                                                             ;\
  load_registers_arm()                    /* load ARM regs                 */;\
  call_c_function(block_lookup_address_arm)                                  ;\
  restore_flags()                                                            ;\
  bx r0                                   /* jump to new ARM block         */;\
2:                                                                           ;\
  load_registers_thumb()                  /* load Thumb regs               */;\
  call_c_function(block_lookup_address_thumb)                                ;\
  restore_flags()                                                            ;\
  bx r0                                   /* jump to new ARM block         */;\
.size arm_update_gba_##name, .-arm_update_gba_##name

arm_update_gba_builder(arm, arm, straight)
arm_update_gba_builder(thumb, thumb, straight)

arm_update_gba_builder(idle_arm, arm, add)
arm_update_gba_builder(idle_thumb, thumb, add)


@ Cheat hooks for master function
@ This is called whenever PC == cheats-master-function
@ Just calls the C function to process cheats

#define cheat_hook_builder(mode)                                             ;\
defsymbl(mode##_cheat_hook)                                                  ;\
  save_flags()                                                               ;\
  store_registers_##mode()                                                   ;\
  call_c_function(process_cheats)                                            ;\
  load_registers_##mode()                                                    ;\
  restore_flags()                                                            ;\
  bx lr                                                                      ;\

cheat_hook_builder(arm)
cheat_hook_builder(thumb)


@ These are b stubs for performing indirect branches. They are not
@ linked to and don't return, instead they link elsewhere.

@ Input:
@ r0: PC to branch to

defsymbl(arm_indirect_branch_arm)
  save_flags()
  call_c_function(block_lookup_address_arm)
  restore_flags()
  bx r0

defsymbl(arm_indirect_branch_thumb)
  save_flags()
  call_c_function(block_lookup_address_thumb)
  restore_flags()
  bx r0

defsymbl(arm_indirect_branch_dual_arm)
  save_flags()
  tst r0, #0x01                           @ check lower bit
  bne 1f                                  @ if set going to Thumb mode
  add r0, #2                              @ two LSB are cleared after
  call_c_function(block_lookup_address_arm)
  restore_flags()
  bx r0                                   @ keep executing arm code
1:
  store_registers_arm()                   @ save out ARM registers
  ldr r1, [reg_base, #REG_CPSR]           @ load cpsr
  load_registers_thumb()                  @ load in Thumb registers
  orr r1, r1, #0x20                       @ set Thumb mode
  str r1, [reg_base, #REG_CPSR]           @ store flags
  call_c_function(block_lookup_address_thumb)
  restore_flags()
  bx r0       
.size arm_indirect_branch_dual_arm, .-arm_indirect_branch_dual_arm

defsymbl(arm_indirect_branch_dual_thumb)
  save_flags()
  tst r0, #0x01                           @ check lower bit
  beq 1f                                  @ if set going to ARM mode
  call_c_function(block_lookup_address_thumb)
  restore_flags()
  bx r0                                   @ keep executing thumb code
1:
  store_registers_thumb()                 @ save out Thumb registers
  ldr r1, [reg_base, #REG_CPSR]           @ load cpsr
  load_registers_arm()                    @ load in ARM registers
  bic r1, r1, #0x20                       @ clear Thumb mode
  str r1, [reg_base, #REG_CPSR]           @ store flags
  add r0, #2                              @ two LSB are cleared after
  call_c_function(block_lookup_address_arm)
  restore_flags()
  bx r0
.size arm_indirect_branch_dual_thumb, .-arm_indirect_branch_dual_thumb

@ Update the cpsr.

@ Input:
@ r0: new cpsr value
@ [lr]:   bitmask (user mode)
@ [lr+4]: bitmask (privileged mode)
@ [lr+8]: current PC

defsymbl(execute_store_cpsr)
  save_flags()
  ldr r1, [reg_base, #CPU_MODE]           @ r1 = cpu_mode
  lsr r1, r1, #4                          @ Load privilege bit
  ldr r1, [lr, r1, lsl #2]                @ Load mask

  and reg_flags, r0, r1                   @ reg_flags = new_cpsr & store_mask
  ldr r0, [reg_base, #REG_CPSR]           @ r0 = cpsr
  bic r0, r0, r1                          @ r0 = cpsr & ~store_mask
  orr reg_flags, reg_flags, r0            @ reg_flags = new_cpsr | cpsr

  mov r0, reg_flags                       @ also put new cpsr in r0

  store_registers_arm()                   @ save ARM registers
  ldr r2, [lr, #8]                        @ r2 = pc
  call_c_function(execute_store_cpsr_body)
  load_registers_arm()                    @ restore ARM registers

  cmp r0, #0                              @ check new PC
  bne 1f                                  @ if it's zero, resume

  restore_flags()
  add pc, lr, #12                         @ return (skip data)

1:
  call_c_function(block_lookup_address_arm)
  restore_flags()
  bx r0                                   @ return to PC ARM address
.size execute_store_cpsr, .-execute_store_cpsr

@ Restore the cpsr from the mode spsr and mode shift.

@ Input:
@ r0: current pc

defsymbl(execute_spsr_restore)
  save_flags()
  ldr r2, [reg_base, #CPU_MODE]           @ r2 = cpu_mode
  ands r2, r2, #0xF                       @ Ignore privilege bits
  beq 2f                                  @ if user/system mode no side effects

  add r1, reg_base, #SPSR_RAM_OFF         @ r1 = spsr
  ldr r1, [r1, r2, lsl #2]                @ r1 = spsr[cpu_mode] (new cpsr)
  str r1, [reg_base, #REG_CPSR]           @ update cpsr
  mov reg_flags, r1                       @ also, update shadow flags

  @ This function call will pass r0 (address) and return it.
  store_registers_arm()                   @ save ARM registers
  call_c_function(execute_spsr_restore_body)

  ldr r1, [reg_base, #REG_CPSR]           @ r1 = cpsr
  tst r1, #0x20                           @ see if Thumb mode is set
  bne 1f                                  @ if so handle it

  load_registers_arm()                    @ restore ARM registers
2:
  call_c_function(block_lookup_address_arm)
  restore_flags()
  bx r0

1:
  load_registers_thumb()                  @ load Thumb registers
  call_c_function(block_lookup_address_thumb)
  restore_flags()
  bx r0


@ Setup the mode transition work for calling an SWI.

@ Input:
@ r0: current pc

#define execute_swi_builder(mode)                                            ;\
                                                                             ;\
defsymbl(execute_swi_##mode)                                                 ;\
  save_flags()                                                               ;\
  add r1, reg_base, #REGMODE_RAM_OFF      /* r1 = reg_mode                 */;\
  /* REG_MODE(MODE_SUPERVISOR)[6] = pc                                     */;\
  ldr r0, [lr]                            /* load PC                       */;\
  str r0, [r1, #((SUPERVISOR_OFFSET * (7 * 4)) + (6 * 4))]                   ;\
  collapse_flags_no_update(r0)            /* r0 = cpsr                     */;\
  add r1, reg_base, #SPSR_RAM_OFF         /* r1 = spsr                     */;\
  str r0, [r1, #(SUPERVISOR_OFFSET * 4)]  /* spsr[MODE_SUPERVISOR] = cpsr  */;\
  bic r0, r0, #0x3F                       /* clear mode flag in r0         */;\
  orr r0, r0, #(0x13 | 0x80)              /* supervisor mode + disable IRQ */;\
  str r0, [reg_base, #REG_CPSR]           /* update cpsr                   */;\
                                                                             ;\
  mov r0, #MODE_SUPERVISOR                                                   ;\
                                                                             ;\
  store_registers_##mode()                /* store regs for mode           */;\
  call_c_function(set_cpu_mode)           /* set the CPU mode to svsr      */;\
  load_registers_arm()                    /* load ARM regs                 */;\
  ldr r0, =0xe3a02004                     /* Update open BUS value         */;\
  str r0, [reg_base, #REG_BUS_VALUE]                                         ;\
                                                                             ;\
  restore_flags()                                                            ;\
  add pc, lr, #4                          /* return                        */;\

execute_swi_builder(arm)
execute_swi_builder(thumb)


@ Start program execution. Normally the mode should be Thumb and the
@ PC should be 0x8000000, however if a save state is preloaded this
@ will be different.

@ Input:
@ r0: initial value for cycle counter

@ Uses sp as reg_base; must hold consistently true.

defsymbl(execute_arm_translate_internal)

  @ save the registers to be able to return later
  stmdb sp!, { r4, r5, r6, r7, r8, r9, r10, r11, r12, lr }

  mov reg_base, r1                        @ init base_reg

  mvn reg_cycles, r0                      @ load cycle counter

  @ Check whether the CPU is sleeping already, we should just wait for IRQs
  ldr r1, [reg_base, #CPU_HALT_STATE]
  cmp r1, #0
  bne cpu_sleep_loop
  b lookup_pc


@ Epilogue to return to the main thread (whatever called execute_arm_translate)

return_to_main:
  @ restore the saved regs and return
  ldmia sp!, { r4, r5, r6, r7, r8, r9, r10, r11, r12, lr }
  bx lr

@ Prepares stores for external write function (align + zero extend value)
#define store_align_8()   and r1, r1, #0xff
#define store_align_16()  bic r0, r0, #0x01; extract_u16(r1, r1)
#define store_align_32()  bic r0, r0, #0x03

#define mask_addr_8(nbits)                                                   ;\
  mov r0, r0, lsl #(32 - nbits)            /* isolate bottom n bits in top */;\
  mov r0, r0, lsr #(32 - nbits)            /* high bits are now clear      */;\

#define mask_addr_16(nbits)                                                  ;\
  mov r0, r0, lsl #(32 - nbits)            /* isolate bottom n bits in top */;\
  mov r0, r0, lsr #(32 - nbits + 1)        /* high bits are now clear      */;\
  mov r0, r0, lsl #1                       /* LSB is also zero             */;\

#define mask_addr_32(nbits)                                                  ;\
  mov r0, r0, lsl #(32 - nbits)            /* isolate bottom n bits in top */;\
  mov r0, r0, lsr #(32 - nbits + 2)        /* high bits are now clear      */;\
  mov r0, r0, lsl #2                       /* 2 LSB are also zero          */;\

@ Vram, OAM and palette memories can only be accessed at a 16 bit boundary
#define mask_addr_bus16_32(nbits) mask_addr_32(nbits)
#define mask_addr_bus16_16(nbits) mask_addr_16(nbits)
#define mask_addr_bus16_8(nbits)  mask_addr_16(nbits)

#define dup8(reg)  bic r1, r1, #0xff00; orr r1, r1, lsl #8;
#define dup16(reg)
#define dup32(reg)

@ Write out to memory.

@ Input:
@ r0: address
@ r1: value
@
@ The instruction at LR is not an inst but a u32 data that contains the PC
@ Used for SMC. That's why return is essentially `pc = lr + 4`

#define execute_store_builder(store_type, str_op, str_op16, load_op)         ;\
                                                                             ;\
ext_store_u##store_type:                                                     ;\
  save_flags()                                                               ;\
  ldr r2, [lr]                            /* load PC                       */;\
  str r2, [reg_base, #REG_PC]             /* write out PC                  */;\
  store_align_##store_type()                                                 ;\
  call_c_function(write_memory##store_type)                                  ;\
  restore_flags()                                                            ;\
  add pc, lr, #4                          /* return                        */;\
                                                                             ;\
ext_io_store_u##store_type:                                                  ;\
  save_flags()                                                               ;\
  ldr r2, [lr]                            /* load PC                       */;\
  str r2, [reg_base, #REG_PC]             /* write out PC                  */;\
                                                                             ;\
  mask_addr_##store_type(10)              /* Mask to IO memory (+align)    */;\
  call_c_function(write_io_register##store_type)                             ;\
                                                                             ;\
  cmp r0, #0                                                                 ;\
  bne write_epilogue                      /* handle additional write stuff */;\
  restore_flags()                                                            ;\
  add pc, lr, #4                          /* return                        */;\
                                                                             ;\
ext_store_iwram_u##store_type:                                               ;\
  save_flags()                                                               ;\
  mask_addr_##store_type(15)              /* Mask to mirror memory (+align)*/;\
  add r2, reg_base, #(IWRAM_OFF+0x8000)   /* r2 = iwram base               */;\
  load_op r2, [r0, r2]                    /* Load current value first      */;\
  cmp r2, r1                              /* Compare with new value        */;\
  beq restore_and_return                  /* Skip if same - no actual change*/;\
  add r2, reg_base, #(IWRAM_OFF+0x8000)   /* r2 = iwram base               */;\
  str_op r1, [r0, r2]                     /* store data                    */;\
  sub r2, r2, #0x8000                     /* r2 = iwram smc base           */;\
  load_op r1, [r0, r2]                    /* r1 = SMC sentinel             */;\
  cmp r1, #0                              /* Check value, should be zero   */;\
  bne 3f                                  /* if so perform smc write       */;\
  restore_flags()                                                            ;\
  add pc, lr, #4                          /* return                        */;\
                                                                             ;\
ext_store_ewram_u##store_type:                                               ;\
  save_flags()                                                               ;\
  mask_addr_##store_type(18)              /* Mask to mirror memory (+align)*/;\
  add r2, reg_base, #EWRAM_OFF            /* r2 = ewram base               */;\
  load_op r2, [r0, r2]                    /* Load current value first      */;\
  cmp r2, r1                              /* Compare with new value        */;\
  beq restore_and_return                  /* Skip if same - no actual change*/;\
  add r2, reg_base, #EWRAM_OFF            /* r2 = ewram base               */;\
  str_op r1, [r0, r2]                     /* store data                    */;\
  add r2, r2, #0x40000                    /* r2 = ewram smc base           */;\
  load_op r1, [r0, r2]                    /* r1 = SMC sentinel             */;\
  cmp r1, #0                              /* Check value, should be zero   */;\
  bne 4f                                  /* if so perform smc write       */;\
  restore_flags()                                                            ;\
  add pc, lr, #4                          /* return                        */;\
                                                                             ;\
ext_store_vram_u##store_type:                                                ;\
  save_flags()                                                               ;\
  mask_addr_bus16_##store_type(17)        /* Mask to mirror memory (+align)*/;\
  dup##store_type(r1)                     /* Duplicate byte if necessary   */;\
  cmp r0, #0x18000                        /* Check if exceeds 96KB         */;\
  subcs r0, r0, #0x8000                   /* Mirror to the last bank       */;\
  add r2, reg_base, #VRAM_OFF             /* r2 = vram base                */;\
  restore_flags()                                                            ;\
  str_op16 r1, [r0, r2]                   /* store data                    */;\
  add pc, lr, #4                          /* return                        */;\
                                                                             ;\
ext_store_oam_ram_u##store_type:                                             ;\
  mask_addr_bus16_##store_type(10)        /* Mask to mirror memory (+align)*/;\
  dup##store_type(r1)                     /* Duplicate byte if necessary   */;\
  add r2, reg_base, #OAM_RAM_OFF          /* r2 = oam ram base             */;\
  str_op16 r1, [r0, r2]                   /* store data                    */;\
  str r2, [reg_base, #OAM_UPDATED]        /* write non zero to signal      */;\
  add pc, lr, #4                          /* return                        */;\
                                                                             ;\
3: /* Flush RAM cache and "resume" execution via re-compile iwram */         ;\
  add r0, r0, #0x3000000                  /* Convert to full address       */;\
  ldr r1, [lr]                            /* load PC                       */;\
  str r1, [reg_base, #REG_PC]             /* write out PC                  */;\
  call_c_function(partial_flush_ram_full)                                    ;\
  b resume_pc                             /* continue execution            */;\
                                                                             ;\
4: /* Flush RAM cache and "resume" execution via re-compile ewram */         ;\
  add r0, r0, #0x2000000                  /* Convert to full address       */;\
  ldr r1, [lr]                            /* load PC                       */;\
  str r1, [reg_base, #REG_PC]             /* write out PC                  */;\
  call_c_function(partial_flush_ram_full)                                    ;\
  b resume_pc                                                                ;\