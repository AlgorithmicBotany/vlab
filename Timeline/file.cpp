#include <cassert>
#include <cstdio>
#include <cstdarg>

#if defined(__APPLE__) || defined(linux)
#include <sys/file.h>
#endif

#include "file.h"

File::File(const char *fname, const char *mode) {
  _fp = fopen(fname, mode);
  name = std::string(fname);
#if defined(__APPLE__) || defined(linux)
  if (_fp) {
    flock(fileno(_fp), LOCK_EX);
  }
#endif
}

File::~File() {
#if defined(__APPLE__) || defined(linux)
  if (_fp) {
    flock(fileno(_fp), LOCK_UN);
  }
#endif
  if (_fp)
    fclose(_fp);
}

void WriteTextFile::PrintF(const char *format, ...) {
  assert(Valid());
  va_list args;
  va_start(args, format);
  vfprintf(_fp, format, args);
  va_end(args);
}
