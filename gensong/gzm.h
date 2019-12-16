#ifndef GZM_H
#define GZM_H
#include <stdint.h>
#include <stdio.h>

typedef struct
{
  union
  {
    struct
    {
      uint16_t      a                   : 1;
      uint16_t      b                   : 1;
      uint16_t      z                   : 1;
      uint16_t      s                   : 1;
      uint16_t      du                  : 1;
      uint16_t      dd                  : 1;
      uint16_t      dl                  : 1;
      uint16_t      dr                  : 1;
      uint16_t                          : 2;
      uint16_t      l                   : 1;
      uint16_t      r                   : 1;
      uint16_t      cu                  : 1;
      uint16_t      cd                  : 1;
      uint16_t      cl                  : 1;
      uint16_t      cr                  : 1;
    };
    uint16_t        pad;                      /* 0x0000 */
  };
  int8_t            x;                        /* 0x0002 */
  int8_t            y;                        /* 0x0003 */
                                              /* 0x0004 */
} z64_controller_t;

typedef struct
{
  z64_controller_t  raw;                      /* 0x0000 */
  /* 0x0000: ok */
  /* 0x0800: device not present */
  /* 0x0400: transaction error */
  uint16_t          status;                   /* 0x0004 */
  z64_controller_t  raw_prev;                 /* 0x0006 */
  uint16_t          status_prev;              /* 0x000A */
  uint16_t          pad_pressed;              /* 0x000C */
  int8_t            x_diff;                   /* 0x000E */
  int8_t            y_diff;                   /* 0x000F */
  char              unk_0x10[0x0002];         /* 0x0010 */
  uint16_t          pad_released;             /* 0x0012 */
  int8_t            adjusted_x;               /* 0x0014 */
  int8_t            adjusted_y;               /* 0x0015 */
  char              unk_0x16[0x0002];         /* 0x0016 */
                                              /* 0x0018 */
} z64_input_t;

void gzm_start(void);
void gzm_stop(FILE *f);
void gzm_record_input(z64_input_t *zi);
void gzm_record_seed(uint32_t old_seed, uint32_t new_seed);
void gzm_record_oca_input(uint16_t pad, int8_t adjusted_x, int8_t adjusted_y);
void gzm_record_oca_sync(int audio_frames);
void gzm_record_room_load(void);
void gzm_record_reset(void);

#endif
