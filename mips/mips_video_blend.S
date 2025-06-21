# MIPS port of ARM video_blend.S for SF2000 performance
# Input:
#   $a0 = screen_src_ptr
#   $a1 = screen_dest_ptr
#   $a2 = start
#   $a3 = end

.data
.align 2
blend_constants:
    .word io_registers
    .word palette_ram_converted
    .word 0x04000200                # combine test mask
    .word 0x07E0F81F                # clamp mask
    .word 0x000003FE                # palette index mask
    .word 0x08010020                # saturation mask

.text
.align 2
.globl expand_blend
.type expand_blend, @function

expand_blend:
    # Save registers
    addiu $sp, $sp, -32
    sw $s0, 0($sp)
    sw $s1, 4($sp)
    sw $s2, 8($sp)
    sw $s3, 12($sp)
    sw $s4, 16($sp)
    sw $s5, 20($sp)
    sw $s6, 24($sp)
    sw $ra, 28($sp)

    # screen_src_ptr += start * 4
    sll $t0, $a2, 2
    addu $a0, $a0, $t0
    
    # screen_dest_ptr += start * 2
    sll $t0, $a2, 1
    addu $a1, $a1, $t0
    
    # counter = end - start
    subu $s0, $a3, $a2

    # Load constants
    la $t0, blend_constants
    lw $s1, 0($t0)                  # io_registers
    lw $s2, 4($t0)                  # palette_ram_converted
    lw $s3, 8($t0)                  # combine test mask (0x04000200)
    lw $s4, 12($t0)                 # clamp mask (0x07E0F81F)
    lw $s5, 16($t0)                 # palette index mask (0x000003FE)
    lw $s6, 20($t0)                 # saturation mask (0x08010020)

    # Load bldalpha from io_registers[0x52]
    lhu $t1, 0x52($s1)
    
    # blend_a = bldalpha & 0x1F
    andi $t2, $t1, 0x1F
    
    # blend_b = (bldalpha >> 8) & 0x1F
    srl $t3, $t1, 8
    andi $t3, $t3, 0x1F
    
    # Clamp blend_a to 16
    slti $t4, $t2, 17
    beqz $t4, 1f
    nop
    li $t2, 16
1:
    # Clamp blend_b to 16
    slti $t4, $t3, 17
    beqz $t4, 2f
    nop
    li $t3, 16
2:
    # Check if (blend_a + blend_b) > 16 for saturation
    addu $t4, $t2, $t3
    slti $t5, $t4, 17
    beqz $t5, saturation_loop
    nop

# Main loop without saturation
main_loop:
    beqz $s0, loop_end
    nop
    
    # Load pixel pair
    lw $t0, 0($a0)
    addiu $a0, $a0, 4
    
    # Check if blending needed
    and $t1, $t0, $s3
    bne $t1, $s3, no_blend
    nop
    
    # Extract top pixel index
    sll $t4, $t0, 1
    and $t4, $t4, $s5
    addu $t4, $s2, $t4
    lhu $t4, 0($t4)                 # pixel_top
    
    # Dilate top pixel
    sll $t5, $t4, 16
    or $t4, $t4, $t5
    and $t4, $t4, $s4               # pixel_top_dilated
    
    # Extract bottom pixel index
    srl $t5, $t0, 15
    and $t5, $t5, $s5
    addu $t5, $s2, $t5
    lhu $t5, 0($t5)                 # pixel_bottom
    
    # Dilate bottom pixel
    sll $t6, $t5, 16
    or $t5, $t5, $t6
    and $t5, $t5, $s4               # pixel_bottom_dilated
    
    # Blend calculation: (pixel_top * blend_a) + (pixel_bottom * blend_b)
    mult $t4, $t2                   # pixel_top * blend_a
    mflo $t4
    mult $t5, $t3                   # pixel_bottom * blend_b
    mflo $t5
    addu $t4, $t4, $t5
    
    # Normalize and mask result
    srl $t4, $t4, 4
    and $t4, $t4, $s4
    srl $t5, $t4, 16
    or $t4, $t4, $t5
    
    # Store result
    sh $t4, 0($a1)
    addiu $a1, $a1, 2
    
    addiu $s0, $s0, -1
    b main_loop
    nop

no_blend:
    # Extract top pixel only
    sll $t4, $t0, 1
    and $t4, $t4, $s5
    addu $t4, $s2, $t4
    lhu $t4, 0($t4)
    
    # Store pixel
    sh $t4, 0($a1)
    addiu $a1, $a1, 2
    
    addiu $s0, $s0, -1
    b main_loop
    nop

# Saturation loop (similar to main loop but with saturation checks)
saturation_loop:
    beqz $s0, loop_end
    nop
    
    # Load pixel pair
    lw $t0, 0($a0)
    addiu $a0, $a0, 4
    
    # Check if blending needed
    and $t1, $t0, $s3
    bne $t1, $s3, no_blend_sat
    nop
    
    # Similar blend logic but with saturation checks
    # (Simplified for now - can be expanded)
    sll $t4, $t0, 1
    and $t4, $t4, $s5
    addu $t4, $s2, $t4
    lhu $t4, 0($t4)
    
    sh $t4, 0($a1)
    addiu $a1, $a1, 2
    
    addiu $s0, $s0, -1
    b saturation_loop
    nop

no_blend_sat:
    sll $t4, $t0, 1
    and $t4, $t4, $s5
    addu $t4, $s2, $t4
    lhu $t4, 0($t4)
    
    sh $t4, 0($a1)
    addiu $a1, $a1, 2
    
    addiu $s0, $s0, -1
    b saturation_loop
    nop

loop_end:
    # Restore registers
    lw $s0, 0($sp)
    lw $s1, 4($sp)
    lw $s2, 8($sp)
    lw $s3, 12($sp)
    lw $s4, 16($sp)
    lw $s5, 20($sp)
    lw $s6, 24($sp)
    lw $ra, 28($sp)
    addiu $sp, $sp, 32
    
    jr $ra
    nop

.size expand_blend, .-expand_blend