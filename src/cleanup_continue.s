/* cleanup routine that continues execution as normal */

/* restore audio irq */
la      $t0, 0x80120180
la      $t1, 0x80120CEC
sw      $t1, 0x0000($t0)

/* restore saved registers */
la      $s0, 0x801F33B0
la      $s1, 0x801C84A0
la      $s2, 0x801DAA30
la      $s3, 0x00000000
la      $s4, 0x801D9C44
la      $s5, 0x00000000
la      $s6, 0xFEFFFFFF
la      $s7, 0x00000001
addiu   $s8, $s1, 0x0810

/* restore stack */
la      $ra, 0x80027264
sw      $ra, 0x0014($sp)
sw      $s0, 0x0028($sp)

/* restore clobbered memory */
la      $t0, 0x000002FF
sw      $t0, 0x0000($s2)

/* restore jump table and return */
la      $t0, 0x80200000
la      $t6, 0x801F8A08
sw      $t6, -0x5F50($t0)
jr      $t6
