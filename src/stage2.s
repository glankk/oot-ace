/* $s7: origin */
/* $sp: 0x8011F050 */

/* create a vi message queue */
addiu   $a0, $sp, 0x0010  /* mq */
addiu   $a1, $sp, 0x0028  /* msg */
li      $a2, 1
/* hijack vi event from audio thread */
sw      $a0, 0x1130($sp)  /* mq_list.mq for audio irq */
jal     0x80004220        /* osCreateMesgQueue(mq, msg, 1) */

0:
  /* wait for vertical retrace */
  addiu   $a0, $sp, 0x0010  /* mq */
  la      $a1, 0
  li      $a2, 1
  jal     0x80002030        /* osRecvMesg(mq, NULL, OS_MESG_BLOCK) */
  /* copy controller data */
  lw      $t0, -0x1920($sp) /* z64_input_direct[0].raw */
  lhu     $t1, -0x18EE($sp) /* z64_input_direct[2].raw.x / y */
  sh      $t0, 0x003C($s7)
  sh      $t1, 0x003E($s7)
  addiu   $s7, $s7, 0x0004
  bne     $t0, $zero, 0b
