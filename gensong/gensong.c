#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "gzm.h"

#ifdef _WIN32
# include <io.h>
# include <fcntl.h>
#endif

_Bool gen_insn(uint32_t insn, int *dur, int *x, int *y)
{
  uint8_t dur_lo  = (insn >> 24) & 0xFF;
  uint8_t unk_0x4 = (insn >> 16) & 0xFF;
  uint8_t vib     = (insn >> 8)  & 0xFF;
  int8_t  bend    = (insn >> 0)  & 0xFF;
  if ((unk_0x4 & 0x57) != unk_0x4)
    return 0;
  if (vib > 0xF)
    return 0;
  if (bend > 0x3C || bend < -0x3C)
    return 0;
  uint8_t dur_hi = (dur_lo == 0 ? 3 : (dur_lo % 3) * 2);
  *dur = ((uint32_t)dur_hi << 8 | (uint32_t)dur_lo) / 3;
  if (vib > 0)
    vib = 7 + vib * 4;
  *x = vib;
  if (bend < 0)
    bend -= 7;
  else if (bend > 0)
    bend += 7;
  *y = bend;
  return 1;
}

static uint16_t note_button[5] =
{
  0x8000, 0x0004, 0x0008, 0x0002, 0x0008,
};

static uint16_t swap16(uint16_t v)
{
  return ((v << 8) & 0xFF00) | ((v >> 8) & 0x00FF);
}

static uint32_t swap32(uint32_t v)
{
  return ((v << 24) & 0xFF000000) | ((v << 8)  & 0x00FF0000) |
         ((v >> 8)  & 0x0000FF00) | ((v >> 24) & 0x000000FF);
}

int gen_gzm(FILE *fi, FILE *fo, _Bool verbose)
{
  gzm_start();
  z64_input_t zi;
  zi.raw.pad = 0;
  zi.raw.x = 0;
  zi.raw.y = 0;
  zi.x_diff = 0;
  zi.y_diff = 0;
  int note_prev = -1;
  int x_prev = 0;
  int y_prev = 0;
  while (1) {
    uint32_t v;
    if (fread(&v, sizeof(v), 1, fi) != 1) {
      if (ferror(fi)) {
        fprintf(stderr, "a read error ocurred\n");
        exit(1);
      }
      break;
    }
    v = swap32(v);
    int dur;
    int x;
    int y;
    if (gen_insn(v, &dur, &x, &y)) {
      int note;
      if (note_prev != -1 && x_prev == x && y_prev == y)
        note = (note_prev + 1 + rand() % 4) % 5;
      else
        note = rand() % 5;
      if (verbose)
        fprintf(stderr, "0x%08x: d %3i,  x %3i,  y %3i\n", v, dur, x, y);
      for (int i = 0; i < dur; ++i) {
        zi.raw_prev = zi.raw;
        zi.raw.pad = note_button[note];
        zi.raw.x = x;
        zi.raw.y = y;
        zi.pad_pressed = (zi.raw.pad ^ zi.raw_prev.pad) & zi.raw.pad;
        zi.pad_released = (zi.raw.pad ^ zi.raw_prev.pad) & ~zi.raw.pad;
        gzm_record_input(&zi);
      }
      note_prev = note;
      x_prev = x;
      y_prev = y;
    }
    else {
      fprintf(stderr, "impossible value: 0x%08x\n", v);
      exit(1);
    }
  }
  gzm_stop(fo);
  return 0;
}

int gen_raw(FILE *fi, FILE *fo, _Bool verbose)
{
  int note_prev = -1;
  int x_prev = 0;
  int y_prev = 0;
  while (1) {
    uint32_t v;
    if (fread(&v, sizeof(v), 1, fi) != 1) {
      if (ferror(fi)) {
        fprintf(stderr, "a read error ocurred\n");
        exit(1);
      }
      break;
    }
    v = swap32(v);
    int dur;
    int x;
    int y;
    if (gen_insn(v, &dur, &x, &y)) {
      int note;
      if (note_prev != -1 && x_prev == x && y_prev == y)
        note = (note_prev + 1 + rand() % 4) % 5;
      else
        note = rand() % 5;
      if (verbose)
        fprintf(stderr, "0x%08x: d %3i,  x %3i,  y %3i\n", v, dur, x, y);
      for (int j = 0; j < dur * 3; ++j) {
        z64_controller_t zc;
        zc.pad = swap16(note_button[note]);
        zc.x = x;
        zc.y = y;
        if (fwrite(&zc, sizeof(zc), 1, fo) != 1) {
          fprintf(stderr, "a write error ocurred\n");
          exit(1);
        }
      }
      note_prev = note;
      x_prev = x;
      y_prev = y;
    }
    else {
      fprintf(stderr, "impossible value: 0x%08x\n", v);
      exit(1);
    }
  }
  return 0;
}

void usage()
{
  fprintf(stderr,
          "usage: gensong [--gzm|--raw] [--verbose]"
          " [-i <infile='-'>] [-o <outfile='-'>]\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  _Bool gzm = 1;
  _Bool verbose = 0;
  const char *fn_i = NULL;
  const char *fn_o = NULL;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--gzm") == 0)
      gzm = 1;
    else if (strcmp(argv[i], "--raw") == 0)
      gzm = 0;
    else if (strcmp(argv[i], "--verbose") == 0)
      verbose = 1;
    else if (fn_i == NULL && strcmp(argv[i], "-i") == 0 && i + 1 < argc)
      fn_i = argv[++i];
    else if (fn_o == NULL && strcmp(argv[i], "-o") == 0 && i + 1 < argc)
      fn_o = argv[++i];
    else
      usage();
  }
  if (fn_i == NULL)
    fn_i = "-";
  if (fn_o == NULL)
    fn_o = "-";

  FILE *fi;
  if (strcmp(fn_i, "-") == 0) {
#ifdef _WIN32
    _setmode(_fileno(stdin), O_BINARY);
#endif
    fi = stdin;
  }
  else {
    fi = fopen(fn_i, "rb");
    if (fi == NULL) {
      fprintf(stderr, "failed to open '%s': %s\n", fn_i, strerror(errno));
      exit(1);
    }
  }

  FILE *fo;
  if (strcmp(fn_o, "-") == 0) {
#ifdef _WIN32
    _setmode(_fileno(stdout), O_BINARY);
#endif
    fo = stdout;
  }
  else {
    fo = fopen(fn_o, "wb");
    if (fo == NULL) {
      fprintf(stderr, "failed to open '%s': %s\n", fn_o, strerror(errno));
      exit(1);
    }
  }

  srand(time(NULL));

  int result;
  if (gzm)
    result = gen_gzm(fi, fo, verbose);
  else
    result = gen_raw(fi, fo, verbose);

  if (fi == stdin) {
#ifdef _WIN32
    _setmode(_fileno(stdin), O_TEXT);
#endif
  }
  else
    fclose(fi);

  if (fo == stdout) {
#ifdef _WIN32
    _setmode(_fileno(stdout), O_TEXT);
#endif
  }
  else
    fclose(fo);

  return result;
}
