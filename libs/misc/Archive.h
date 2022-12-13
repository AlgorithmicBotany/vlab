/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#ifndef __ARCHIVE_CLASS_H__
#define __ARCHIVE_CLASS_H__

#include "ProgressReporter.h"
#include <string>
#include <sys/types.h>

class Archive {
public:
  enum Destination { AR_FILE, AR_MEM, AR_SOCKET, AR_FD };

  /* constructor*/ Archive(ProgressReporter *rep = NULL);
  /* destructor*/ ~Archive(void);
  int pack(const char *dir_name, int recursive);
  int unpack(const char *archive_name, const char *dst_dir);
  void set_destination(Destination, ...);
  int list(const char *fname);

  char *get_errors(void);

  long get_archive_size(void);

private:
  void add_error(const char *fmt_str, ...);

  void add_error2(const std::string &str);

  int _pack(const char *path, int level, struct stat *dir_stat,
            double progressMin, double progressMax);
  int _unpack(const char *path, int level);
  int _list(const char *path, int level);
  int read_string(char *buff, u_int32_t size);
  int _pack_reg_file(const char *fname, struct stat *);
  int add_char(char c);
  int add_buff(const char *buff, long size);
  void indented_printf(long indent_level, const char *fmt_str, ...);

  int recursive;        // whether to proceed recursively or not
  char root_name[4096]; // name of the top level directory
  char dst_fname[4096]; // name of the file where to store the
                        // archive
  int fd;               // file descriptor of the destination

  char *errors; // collection of errors

  long archive_size; // size of the archive

  Destination dst_type;

  ProgressReporter *progressReporter_;
};

#endif
