#pragma once

// This is the structure used in a message  xQueue between the receiver and the main app task

struct Message {
  int    length;
  char * data;
};