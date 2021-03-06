// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Example of a clock. This is very similar to the text-example,
// except that it shows the time :)
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"
#include "graphics.h"
#include "utf8-internal.h"

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <vector>
#include <string>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

const char *words_hours12[12] = { 
    "Twelve",
    "One",
    "Two",
    "Three",
    "Four",
    "Five",
    "Six",
    "Seven",
    "Eight",
    "Nine",
    "Ten",
    "Eleven"
};

const char *words_minutes20[20] = { 
    "O\' Clock",
    "O\' One",
    "O\' Two",
    "O\' Three",
    "O\' Four",
    "O\' Five",
    "O\' Six",
    "O\' Seven",
    "O\' Eight",
    "O\' Nine",
    "Ten",
    "Eleven",
    "Twelve",
    "Thirteen",
    "Fourteen",
    "Fifteen",
    "Sixteen",
    "Seventeen",
    "Eighteen",
    "Nineteen"
};

const char *words_tens[12] = { 
    "Twenty",
    "Thirty",
    "Forty",
    "Fifty"
};

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Reads text from stdin and displays it. "
          "Empty string: clear screen\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr,
          "\t-d <time-format>  : Default '%%H:%%M'. See strftime()\n"
          "\t                    Can be provided multiple times for multiple "
          "lines\n"
          "\t-f <font-file>    : Use given font.\n"
          "\t-x <x-origin>     : X-Origin of displaying text (Default: 0)\n"
          "\t-y <y-origin>     : Y-Origin of displaying text (Default: 0)\n"
          "\t-s <line-spacing> : Extra spacing between lines when multiple -d given\n"
          "\t-S <spacing>      : Extra spacing between letters (Default: 0)\n"
          "\t-C <r,g,b>        : Color. Default 255,255,0\n"
          "\t-B <r,g,b>        : Background-Color. Default 0,0,0\n"
          "\t-O <r,g,b>        : Outline-Color, e.g. to increase contrast.\n"
          "\n"
          );
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

static int displayWidth(const Font &font, const char *utf8_text, int extra_spacing) {
    int x = 0;
    while (*utf8_text) {
        if (x > 0) {
            x += extra_spacing;
        }
        const uint32_t cp = utf8_next_codepoint(utf8_text);
        x += font.CharacterWidth(cp);
    }
    return x;
}

static bool parseColor(Color *c, const char *str) {
  return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
}

static bool FullSaturation(const Color &c) {
  return (c.r == 0 || c.r == 255)
    && (c.g == 0 || c.g == 255)
    && (c.b == 0 || c.b == 255);
}



int main(int argc, char *argv[]) {
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  // We accept multiple format lines

  std::vector<std::string> format_lines;
  Color color(255, 255, 0);
  Color bg_color(0, 0, 0);
  Color outline_color(0,0,0);

  const char *bdf_font_file = NULL;
  int x_orig = 0;
  int y_orig = 0;
  int letter_spacing = 0;
  int line_spacing = 0;
  bool center = false;

  int opt;
  while ((opt = getopt(argc, argv, "x:y:f:C:B:O:s:S:d:c")) != -1) {
    switch (opt) {
    case 'x': x_orig = atoi(optarg); break;
    case 'y': y_orig = atoi(optarg); break;
    case 'f': bdf_font_file = strdup(optarg); break;
    case 's': line_spacing = atoi(optarg); break;
    case 'c': center = true; break;
    case 'S': letter_spacing = atoi(optarg); break;
    case 'C':
      if (!parseColor(&color, optarg)) {
        fprintf(stderr, "Invalid color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    case 'B':
      if (!parseColor(&bg_color, optarg)) {
        fprintf(stderr, "Invalid background color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    default:
      return usage(argv[0]);
    }
  }

  if (bdf_font_file == NULL) {
    fprintf(stderr, "Need to specify BDF font-file with -f\n");
    return usage(argv[0]);
  }

  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  rgb_matrix::Font font;
  if (!font.LoadFont(bdf_font_file)) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    return 1;
  }

  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL)
    return 1;

  const bool all_extreme_colors = (matrix_options.brightness == 100)
    && FullSaturation(color)
    && FullSaturation(bg_color)
    && FullSaturation(outline_color);

  if (all_extreme_colors)
    matrix->SetPWMBits(1);

  int x = x_orig;
  int y = y_orig;

  FrameCanvas *offscreen = matrix->CreateFrameCanvas();

  struct timespec next_time;
  next_time.tv_sec = time(NULL);
  next_time.tv_nsec = 0;
  struct tm tm;

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  while (!interrupt_received) {
    offscreen->Fill(bg_color.r, bg_color.g, bg_color.b);
    localtime_r(&next_time.tv_sec, &tm);

    format_lines.clear();
    format_lines.push_back("The time is");
    format_lines.push_back("");
    int hour = tm.tm_hour;
    bool pm = hour > 11;
    if (pm) {
        hour = hour - 12;
    }
    format_lines.push_back(words_hours12[hour]);
    int minute = tm.tm_min;
    if (minute < 20) {
        format_lines.push_back(words_minutes20[minute]);
    } else {
        int tens = (minute / 10) - 2;
        format_lines.push_back(words_tens[tens]);
        int ones = minute % 10;
        if (ones > 0) {
            format_lines.push_back(words_hours12[ones]);
        }
    }
   
    int line_offset = 0;
    for (const std::string &line : format_lines) {
        if (center) {
            int line_width = displayWidth(font, line.c_str(), letter_spacing);
            x = (64 - line_width) / 2;
        }
        rgb_matrix::DrawText(offscreen, font,
                            x, y + font.baseline() + line_offset,
                            color, NULL, line.c_str(),
                            letter_spacing);
        line_offset += font.height() + line_spacing;
    }

    // Wait until we're ready to show it.
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_time, NULL);

    // Atomic swap with double buffer
    offscreen = matrix->SwapOnVSync(offscreen);

    next_time.tv_sec += 1;
  }

  // Finished. Shut down the RGB matrix.
  delete matrix;

  write(STDOUT_FILENO, "\n", 1);  // Create a fresh new line after ^C on screen
  return 0;
}

