/* cleanup routine that restarts the game context */

/* restore registers */
la      $s0, 0x801C84A0
la      $s1, 0x8011F290
la      $s2, 0x800F13D0
la      $s3, 0x800F13D0
la      $s4, 0x80108DDC
la      $sp, 0x8011F250

/* restore audio irq */
la      $t0, 0x80120000
la      $t1, 0x80120CEC
sw      $t1, 0x0180($t0)

/* setup context */
/* prevent the context dtor from running
   because the clobbered heap will cause it to crash */
sw      $zero, 0x0008($s0)      /* state_dtor */
la      $t0, 0x8009A750
sw      $t0, 0x000C($s0)        /* next_ctor */
la      $t1, 0x00012518
sw      $t1, 0x0010($s0)        /* next_size */
sw      $zero, 0x0098($s0)      /* state_continue */

/* jump back to main loop */
la      $t0, 0x800A19C8
jr      $t0
