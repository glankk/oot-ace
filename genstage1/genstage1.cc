#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <vector>

#ifdef _WIN32
# include <io.h>
# include <fcntl.h>
#endif

void usage()
{
  fprintf(stderr,
          "usage:  genstage1 <v0> <t2> <s2> <dst>"
          " [<infile='-'>] [<outfile='-'>]\n"
          "dst:    v0 | t2 | s2\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  if (argc < 5 || argc > 7)
    usage();

  int v0;
  int t2;
  int s2;
  const char *fn_i = "-";
  const char *fn_o = "-";
  if (sscanf(argv[1], "%i", &v0) != 1)
    usage();
  if (sscanf(argv[2], "%i", &t2) != 1)
    usage();
  if (sscanf(argv[3], "%i", &s2) != 1)
    usage();
  const char *dst = argv[4];
  if (strcmp(dst, "v0") != 0 &&
      strcmp(dst, "t2") != 0 &&
      strcmp(dst, "s2") != 0)
  {
    usage();
  }
  if (argc >= 6)
    fn_i = argv[5];
  if (argc >= 7)
    fn_o = argv[6];

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
  if (strcmp(fn_o, "-") == 0)
    fo = stdout;
  else {
    fo = fopen(fn_o, "w");
    if (fo == NULL) {
      fprintf(stderr, "failed to open '%s': %s\n", fn_o, strerror(errno));
      exit(1);
    }
  }

  std::map<uint8_t, std::vector<int>> byte_places;
  for (int i = 0, c = fgetc(fi); c != EOF; ++i) {
    if (i > 0x3C) {
      fprintf(stderr, "error: input too long\n");
      exit(1);
    }
    if (abs((int8_t)(c - v0)) > 0x3C &&
        abs((int8_t)(c - t2)) > 0x3C &&
        abs((int8_t)(c - s2)) > 0x3C)
    {
      fprintf(stderr, "error: impossible character 0x%02x at %i\n", c, i);
      exit(1);
    }
    byte_places[c].push_back(i);
    c = fgetc(fi);
  }

  fprintf(fo, "/* $v0: 0x%02x */\n", v0);
  fprintf(fo, "/* $t2: 0x%02x */\n", t2);
  fprintf(fo, "/* $s2: 0x%02x */\n", s2);
  fprintf(fo, "\n");
  fprintf(fo, "/* dst: $%s */\n", dst);
  fprintf(fo, "\n");

  fprintf(fo, ".set    noreorder\n");
  fprintf(fo, "\n");

  for (auto p : byte_places) {
    auto c = p.first;
    auto &v = p.second;
    fprintf(fo, "/* %02x */\n", c);
    if (abs((int8_t)(c - v0)) <= 0x3C)
      fprintf(fo, "addiu   $s7, $v0, 0x%04x\n", (uint8_t)(c - v0));
    else if (abs((int8_t)(c - t2)) <= 0x3C)
      fprintf(fo, "addiu   $s7, $t2, 0x%04x\n", (uint8_t)(c - t2));
    else if (abs((int8_t)(c - s2)) <= 0x3C)
      fprintf(fo, "addiu   $s7, $s2, 0x%04x\n", (uint8_t)(c - s2));
    for (auto i : v)
      fprintf(fo, "sb      $s7, 0x%04x($%s)\n", i, dst);
  }

  fprintf(fo, "\n");
  fprintf(fo, "addiu   $s7, $%s, 0x0000\n", dst);
  fprintf(fo, "jr      $%s\n", dst);
  fprintf(fo, "sll     $zero, $s7, 0\n");

  if (fi == stdin) {
#ifdef _WIN32
    _setmode(_fileno(stdin), O_TEXT);
#endif
  }
  else
    fclose(fi);

  if (fo != stdout)
    fclose(fo);

  return 0;
}
