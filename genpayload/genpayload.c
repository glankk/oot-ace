#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
# include <io.h>
# include <fcntl.h>
#endif


void usage()
{
  fprintf(stderr, "usage: genpayload [-i <infile='-'>] [-o <outfile='-'>]\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  const char *fn_i = NULL;
  const char *fn_o = NULL;
  for (int i = 1; i < argc; ++i) {
    if (fn_i == NULL && strcmp(argv[i], "-i") == 0 && i + 1 < argc)
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

  uint8_t start[] =
  {
    0x82, 0x2A, 0x20, 0xAC, 0x08, 0x04, 0x69, 0x7D,
  };
  for (int i = 0; i < 9; ++i) /* 2 frames, plus 1 to be lag safe */
    fwrite(start, sizeof(start), 1, fo);

  uint8_t word[4];
  while (fread(word, sizeof(word), 1, fi) == 1) {
    uint8_t pad[] = {0x00, 0x01};
    fwrite(pad, sizeof(pad), 1, fo);
    fwrite(&word[0], 2, 1, fo);
    fwrite(pad, sizeof(pad), 1, fo);
    fwrite(&word[2], 2, 1, fo);
  }

  uint8_t stop[] =
  {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };
  fwrite(stop, sizeof(stop), 1, fo);

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

  return 0;
}
