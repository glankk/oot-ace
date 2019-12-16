.macro  init_pic
  la      $t9, 9f
  bal     9f
  9:
  subu    $t9, $ra, $t9
.endm

.macro  la_pic reg, addr
  la      \reg, \addr
  addu    \reg, $t9
.endm


_start:
  init_pic

  /* relocate */
  la_pic  $a0, code_patch_tbl + 0x0004
  bal     reloc_R_MIPS_26
  addiu   $a0, $a0, 0x0008
  bal     reloc_R_MIPS_26
  addiu   $a0, $a0, 0x0008
  bal     reloc_R_MIPS_26
  addiu   $a0, $a0, 0x0008
  bal     reloc_R_MIPS_26
  addiu   $a0, $a0, 0x0008
  bal     reloc_R_MIPS_26
  addiu   $a0, $a0, 0x0008
  bal     reloc_R_MIPS_26
  la_pic  $a0, ovl_patch_tbl + 0x0004
  bal     reloc_R_MIPS_26
  addiu   $a0, $a0, 0x0008
  bal     reloc_R_MIPS_26
  la_pic  $a0, rom_patch_tbl + 0x0008
  bal     reloc_R_MIPS_32

  /* apply patches */
  bal     PatchCode
  la      $t0, 0x800FE480
  lw      $a0, 0x0028($t0)
  lw      $a1, 0x002C($t0)
  lw      $a2, 0x001C($t0)
  bal     PatchOverlay

  /* FrankerZ on B */
  la      $t0, 0x8011A5D0
  li      $t1, 2 /* Z64_ITEM_BOMB */
  sb      $t1, 0x0068($t0)
  la      $a0, 0x801C84A0
  li      $a1, 0 /* Z64_ITEMBTN_B */
  jal     0x8006FB50 /* z64_UpdateItemButton */

  /* cleanup and continue */
  .include        "cleanup_continue.s"


/* $a0: ptr */
/* $t9: pic offset */
reloc_R_MIPS_26:
  lw      $t0, 0x0000($a0)
  srl     $t1, $t0, 26
  sll     $t1, $t1, 26
  sll     $t2, $t0, 2
  addu    $t2, $t9, $t2
  sll     $t2, $t2, 4
  srl     $t2, $t2, 6
  or      $t0, $t1, $t2
  sw      $t0, 0x0000($a0)
  jr      $ra


/* $a0: ptr */
/* $t9: pic offset */
reloc_R_MIPS_32:
  lw      $t0, 0x0000($a0)
  addu    $t0, $t9, $t0
  sw      $t0, 0x0000($a0)
  jr      $ra


/* $t9: pic offset */
PatchCode:
  la_pic  $t0, code_patch_tbl
  0:
    lw      $t1, 0x0000($t0)
    beqz    $t1, 1f
    lw      $t2, 0x0004($t0)
    sw      $t2, 0x0000($t1)
    cache   0x19, 0x0000($t1) /* pd hit writeback */
    cache   0x10, 0x0000($t1) /* pi hit invalidate */
    addiu   $t0, $t0, 0x0008
    b       0b
  1:
  jr      $ra


/* $a0: vram_start */
/* $a1: vram_end */
/* $a2: ptr */
/* $t9: pic offset */
PatchOverlay:
  la_pic  $t0, ovl_patch_tbl
  0:
    lw      $t1, 0x0000($t0)
    beqz    $t1, 2f
    blt     $t1, $a0, 1f
    bge     $t1, $a1, 1f
      lw      $t2, 0x0004($t0)
      subu    $t1, $t1, $a0
      addu    $t1, $a2, $t1
      sw      $t2, 0x0000($t1)
      cache   0x19, 0x0000($t1) /* pd hit writeback */
      cache   0x10, 0x0000($t1) /* pi hit invalidate */
    1:
    addiu   $t0, $t0, 0x0008
    b       0b
  2:
  jr      $ra


LoadOverlay_hook:
  addiu   $sp, $sp, -0x0018
  sw      $ra, 0x0014($sp)
  sw      $a2, 0x0020($sp)
  sw      $a3, 0x0024($sp)
  lw      $t0, 0x0028($sp)
  sw      $t0, 0x0010($sp)

  jal     0x800CCBB8 /* z64_LoadOverlay */

  init_pic
  lw      $a0, 0x0020($sp)
  lw      $a1, 0x0024($sp)
  lw      $a2, 0x0028($sp)
  bal     PatchOverlay

  lw      $ra, 0x0014($sp)
  addiu   $sp, $sp, 0x0018
  jr      $ra


GetFile_hook:
  sw      $a1, 0x0034($sp)
  sw      $a2, 0x0038($sp)
  sw      $a3, 0x003C($sp)
  li      $s0, 0
  init_pic
  la_pic  $t0, rom_patch_tbl
  addu    $t6, $a2, $a3
  0:
    lw      $t1, 0x0000($t0)
    beqz    $t1, 2f
    lw      $t2, 0x0004($t0)
    addu    $t4, $t1, $t2
    blt     $t4, $a2, 1f
    bge     $t1, $t6, 1f
      li      $s0, 1
      b       2f
    1:
    addiu   $t0, 0x000C
    b       0b
  2:

  sw      $a2, 0x0000($a0)
  sw      $a1, 0x0004($a0)
  sw      $a3, 0x0008($a0)
  sw      $zero, 0x00014($a0)
  lw      $t0, 0x0044($sp)
  sw      $t0, 0x0018($a0)
  lw      $t1, 0x0048($sp)
  sw      $t1, 0x001C($a0)

  move    $a1, $a0
  la      $a0, 0x80007D40
  li      $a2, 1 /* OS_MESG_BLOCK */
  jal     0x80001E20 /* osSendMesg */

  beqz    $s0, 4f
    lw      $a0, 0x0044($sp)
    la      $a1, 0
    li      $a2, 1 /* OS_MESG_BLOCK */
    jal     0x80002030 /* osRecvMesg */
    lw      $a0, 0x0044($sp)
    lw      $a1, 0x0048($sp)
    li      $a2, 1 /* OS_MESG_BLOCK */
    jal     0x80001E20 /* osSendMesg */

    lw      $a1, 0x0034($sp)
    lw      $a2, 0x0038($sp)
    lw      $a3, 0x003C($sp)

    init_pic
    la_pic  $t0, rom_patch_tbl
    addu    $t6, $a2, $a3
    0:
      lw      $t1, 0x0000($t0)
      beqz    $t1, 3f
      lw      $t2, 0x0004($t0)
      lw      $t3, 0x0008($t0)
      addu    $t4, $t1, $t2
      move    $t5, $a1
      subu    $t5, $t5, $a2
      addu    $t5, $t5, $t1

      subu    $t7, $t1, $a2
      bgez    $t7, 1f
        subu    $t3, $t3, $t7
        subu    $t5, $t5, $t7
        addu    $t2, $t2, $t7
      1:

      subu    $t7, $t6, $t4
      bgez    $t7, 1f
        addu    $t2, $t2, $t7
      1:

      blez    $t2, 2f
      1:
        addiu   $t2, $t2, -1
        lb      $t7, 0x0000($t3)
        sb      $t7, 0x0000($t5)
        addiu   $t3, $t3, 1
        addiu   $t5, $t5, 1
        bgtz    $t2, 1b

      2:
      addiu   $t0, 0x000C
      b       0b
    3:
  4:

  li      $v0, 0
  lw      $ra, 0x002C($sp)
  lw      $s0, 0x0028($sp)
  addiu   $sp, $sp, 0x0030
  jr      $ra


/* $a0: object id */
LoadObject:
  la        $t0, 0x801D9C44 /* z64_obj_ctxt */
  la        $t1, 0x800F8FF8 /* z64_object_table */
  /* check that the object is not loaded already, or marked for loading */
  subu      $t2, $zero, $a0
  li        $t3, 0x0000
  lbu       $t4, 0x0008($t0)
  addiu     $t5, $t0, 0x000C
  0:
    beq     $t3, $t4, 0f
    lh      $t6, 0x0000($t5)
    beq     $t6, $a0, 1f
    beq     $t6, $t2, 1f
    addiu   $t3, $t3, 0x0001
    addiu   $t5, $t5, 0x0044
    b       0b
  0:
  /* look up the object size */
  sll       $t6, $a0, 0x0003
  addu      $t6, $t1, $t6
  lw        $t7, 0x0000($t6)
  lw        $t8, 0x0004($t6)
  subu      $t6, $t8, $t7
  /* mark object for loading */
  sh        $t2, 0x0000($t5)
  sw        $zero, 0x0008($t5)
  lw        $t3, 0x0004($t5)
  addu      $t3, $t3, $t6
  sw        $t3, 0x0048($t5)
  /* increment object count */
  addiu     $t4, $t4, 0x0001
  sb        $t4, 0x0008($t0)
  1:
  jr        $ra


SpawnAndAttachActor_hook:
  li      $t0, 0x0010
  bne     $a3, $t0, 0f
    sw      $ra, -0x0004($sp)
    sw      $a0, 0x0000($sp)
    li      $a0, 0x016B
    bal     LoadObject
    lw      $a0, 0x0000($sp)
    lw      $ra, -0x0004($sp)
    li      $a3, 0x019B
    li      $t0, 0x0003
    sh      $t0, 0x002A($sp)
  0:

  j       0x800253F0 /* z64_SpawnAndAttachActor */


dog_hitbox_hook:
  la      $t0, 0x801DAA30
  lw      $t0, 0x011C($t0)
  beq     $t0, $s0, 0f
    j       0x8004C130
  0:
  jr      $ra


.set    noreorder

code_patch_tbl:
  .word   0x80000D30
  j       GetFile_hook
  .word   0x8001B508
  jal     LoadOverlay_hook
  .word   0x8002520C
  jal     LoadOverlay_hook
  .word   0x80066E40
  jal     LoadOverlay_hook
  .word   0x80099C8C
  jal     LoadOverlay_hook
  .word   0x800CCD74
  jal     LoadOverlay_hook
  .word   0

ovl_patch_tbl:
  .word   0x808318CC
  jal     SpawnAndAttachActor_hook
  .word   0x80B4AC0C
  jal     dog_hitbox_hook
  /* ammo hack */
  .word   0x80834180
  bne     $zero, $zero, . + 0x0018
  /* bomb actor count hack */
  .word   0x80834190
  beq     $zero, $zero, . + 0x0018
  /* multiple dogs hack */
  .word   0x80B4A398
  beq     $zero, $zero, . + 0x0020
  .word   0

rom_patch_tbl:
  .word   0x007BF000
  .word   0x00001000
  .word   FrankerZ.png
  .word   0

FrankerZ.png:
  .incbin "FrankerZ.png.bin"
