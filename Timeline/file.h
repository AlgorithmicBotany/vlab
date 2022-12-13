#ifndef __FILE_H__
#define __FILE_H__
#include <string>

class File {
protected:
  File(const char *fname, const char *mode);

public:
  ~File();
  bool Valid() const { return NULL != _fp; }
  operator FILE *() { return _fp; }
  FILE *fp() { return _fp; }
  std::string getname() { return name; }

protected:
  FILE *_fp;
  std::string name;
};

class ReadTextFile : public File {
public:
  ReadTextFile(const char *fname) : File(fname, "rt") {}
};

class WriteTextFile : public File {
public:
  WriteTextFile(const char *fname) : File(fname, "wt") {}
  void PrintF(const char *, ...);
};

#else
#error File already included
#endif
