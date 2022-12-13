#include <cassert>
#include <fstream>
#include <string>

#include "scramble.h"

#include <windows.h>

const char VerSign[5] = "F\xE4M\xA0";

class FFile {
public:
  FFile(const char *fname) { _hFind = FindFirstFile(fname, &_fd); }
  ~FFile() {
    if (_hFind != INVALID_HANDLE_VALUE)
      FindClose(_hFind);
  }
  bool Found() const { return INVALID_HANDLE_VALUE != _hFind; }
  bool IsDirectory() const {
    return (_fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  }
  const char *FileName() const {
    assert(Found());
    return _fd.cFileName;
  }
  bool FindNext() {
    assert(Found());
    if (0 == FindNextFile(_hFind, &_fd)) {
      FindClose(_hFind);
      _hFind = INVALID_HANDLE_VALUE;
      return false;
    } else
      return true;
  }

private:
  HANDLE _hFind;
  WIN32_FIND_DATA _fd;
};

FileScrambler::FileScrambler(const char *fname)
    : _output(fname, std::ios_base::out | std::ios_base::trunc |
                         std::ios_base::binary) {
  assert(sizeof(ChkSumType) == 2);
  _chksm = 0;
  _output.write(VerSign, 4);
}

FileScrambler::~FileScrambler() {
  _Add(0);
  while (!_buffer.Empty()) {
    _Add(0);
  }
  char chksmbf[4];
  chksmbf[1] = _chksm % 256;
  chksmbf[3] = _chksm >> 8;
  _output.seekp(0);
}

void FileScrambler::Add(const char *fnm, const char *cntnts) {
  _Add(fnm, strlen(fnm) + 1);
  assert(sizeof(size_t) == 4);
  size_t l = strlen(cntnts);
  _Add(reinterpret_cast<const char *>(&l), sizeof(size_t));
  _Add(cntnts, l);
}

void FileScrambler::AddDir(const char *path) {
  std::string mask(path);
  mask += "\\*.*";
  FFile ff(mask.c_str());
  while (ff.Found()) {
    if (!ff.IsDirectory())
      Add(ff.FileName());
    ff.FindNext();
  }
}

void FileScrambler::Add(const char *fname) {
  /*
  std::ifstream src(fname, std::ios::in | std::ios::binary);
  if (!src.is_open())
          throw "Cannot open file for reading";

  src.seekg(0, std::ios_base::end);
  assert(sizeof(size_t) == 4);
  size_t sz = src.tellg();
  src.seekg(0, std::ios_base::beg);

  const char* bs = strrchr(fname, '\\');
  if (NULL == bs)
          bs = fname;
  _Add(bs, strlen(bs)+1);
  _Add(reinterpret_cast<const char*>(&sz), sizeof(size_t));
  for (size_t n = 0; n<sz; ++n)
  {
          char c;
          src.read(&c, 1);
          _Add(c);
  }
  */
}

void FileScrambler::_Add(const char *bf, size_t t) {
  for (size_t n = 0; n < t; ++n)
    _Add(bf[n]);
}

void FileScrambler::_Add(char c) {
  assert(!_buffer.Full());
  _buffer.Add(c);
  _chksm += c;
  if (_buffer.Full()) {
    _buffer.Shuffle();
    _buffer.Flush(_output);
  }
  assert(!_buffer.Full());
}

FileDescrambler::Buffer::Buffer(const char *fname)
    : _src(fname, std::ios_base::in | std::ios_base::binary),
      _last(BufferSize) {
  if (!_src.is_open())
    throw "Cannot open file %s for reading";
  char bf[4];
  _src.read(bf, 4);
  if (0 != strncmp(bf, VerSign, 4))
    throw "Tar file version mismatch";
}

char FileDescrambler::Buffer::_Get() {
  if (BufferSize == _last) {
    _src.read(_bf, BufferSize);
    _Deshuffle();
    _last = 0;
  }
  char res = _bf[_last];
  ++_last;
  return res;
}

void FileDescrambler::Extract() {
  std::string fnm;
  _ExtractFName(fnm);
  while (!fnm.empty()) {
    _ExtractFile(fnm.c_str());
    fnm = "";
    _ExtractFName(fnm);
  }
}

void FileDescrambler::_ExtractFName(std::string &fnm) {
  char c = _buffer.GetChar();
  while (c != '\0') {
    fnm += c;
    c = _buffer.GetChar();
  }
}

void FileDescrambler::_ExtractFile(const char *fnm) {
  assert(sizeof(size_t) == 4);
  size_t l;
  _buffer.Get(reinterpret_cast<char *>(&l), sizeof(size_t));
  std::ofstream trg(fnm, std::ios_base::out | std::ios_base::trunc |
                             std::ios_base::binary);
  for (size_t n = 0; n < l; ++n) {
    char c;
    _buffer.Get(&c, 1);
    trg << c;
  }
}
