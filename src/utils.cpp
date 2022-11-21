#include <esp_log.h>
#include <cinttypes>
#include <cstring>
#include <cstdio>

#include "config.hpp"

void dump_data(const char *tag, const uint8_t * data, int len) 
{
  esp_log_level_t log_level = esp_log_level_get(tag);

  if ((log_level != ESP_LOG_DEBUG) && (log_level != ESP_LOG_VERBOSE)) return;

  int pos = 0;
  static char buff[80];

  while (pos < len) {
    sprintf(buff, "%03XH:", pos);
    int k = 5;
    for (int i = 0; i < 16; i++) {
      if ((pos + i) < len) {
        sprintf(&buff[k], " %02X", data[pos + i]);
      }
      else {
        strcpy(&buff[k], "   ");
      }
      k += 3;
    }
    strcpy(&buff[k], "  |");
    k += 3;
    for (int i = 0; i < 16; i++) {
      buff[k++] = ((pos + i) < len) 
                    ? (((data[pos + i] >= ' ') && (data[pos + i] <= '~')) 
                        ? data[pos + i] 
                        : '.') 
                    : ' ';
    }
    strcpy(&buff[k], "|");
    ESP_LOGD(tag, "%s", buff);

    pos += 16;
  }
}

#define GET(c) if (ilen > 0) { c = *ibuff++; ilen--; } else { return false; }
#define PUT(c) if (olen > 0) { *obuff++ = c; olen--; } else { return false; }
#define SKIP_SPACES while (isspace(ch)) { PUT(ch); GET(ch); }

static char * obuff;
static char * ibuff;
static int    ilen;
static int    olen; 
static char   ch;

static bool do_item();

static bool do_object()
{
  if (ch != '{') return false;

  PUT(ch); SKIP_SPACES;

  if ((ch != '}') && (ch != ',')) {
    do {
      if (ch == ',') { PUT(ch); GET(ch); SKIP_SPACES; }
      if (!do_item()) return false;
    } while (ch == ',');
  }

  if (ch != '}') return false;
  PUT(ch); GET(ch); SKIP_SPACES;

  return true;
}

static bool do_string()
{
  if (ch == '"') {
    PUT(ch); GET(ch);
    while (ch != '"') {
      if (ch == '\\') {
        PUT(ch); GET(ch); PUT(ch);
      }
      else {
        PUT(ch); 
      }
      GET(ch);
    }
    PUT(ch); GET(ch);
  }
  else {
    PUT('"');
    while (!isspace(ch) && (ch != ':') && (ch != ',') && (ch != '}') && (ch != ']')) {
      PUT(ch); GET(ch);
    }
    PUT('"');
  }

  SKIP_SPACES;

  return true;
}

static bool do_value()
{
  if ((ilen > 5) && (strncmp("false", ibuff-1, 5) == 0)) {
    for (int i = 0; i < 5; i++) { PUT(ch); GET(ch); }
  }
  else if (ch == '"') {
    return do_string();
  }

  return true;
}

static bool do_item()
{
  if (!do_string()) return false;

  if (ch != ':') return false;
  PUT(ch); GET(ch); SKIP_SPACES;

  return do_value();
}

int to_json(uint8_t * in_buffer, int in_len, char * out_buffer, int out_len)
{
  if (out_len < in_len) return -1;

  obuff = out_buffer;
  ibuff = (char *) in_buffer;
  olen  = out_len;
  ilen  = in_len;

  GET(ch); SKIP_SPACES;

  return (!do_object()) ? out_len - olen : -1;
}