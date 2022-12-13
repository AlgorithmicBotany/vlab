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



#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <limits.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "Archive.h"
#include "dirList.h"
#include "utilities.h"
#include "xmemory.h"
#include "xstring.h"

static const int DEBUG = 0;

enum FILE_CODES { AR_DIRECTORY = 1, AR_EXIT_DIRECTORY, AR_FILE, AR_LINK };

/*------------.
| constructor |
`------------*/

Archive::Archive(ProgressReporter *rep) {
  progressReporter_ = rep;
  dst_type = AR_FILE;
  strcpy(dst_fname, "/var/tmp/archive.ar");

  // archive size is currently undefined
  archive_size = -1;

  errors = xstrdup("");
}

/*-----------------.
| get_archive_size |
`-----------------*/

long Archive::get_archive_size(void) { return archive_size; }

/*-----------.
| destructor |
`-----------*/

Archive::~Archive() { xfree(errors); }

/*--------------------------------.
| set the destination for packing |
`--------------------------------*/

void Archive::set_destination(Archive::Destination dst, ...) {
  va_list ap;
  va_start(ap, dst);

  if (dst == AR_FILE) {
    char *fname = va_arg(ap, char *);
    dst_type = AR_FILE;
    strcpy(dst_fname, fname);
  } else {
    add_error("Archive::set_destination(): %d not supported.\n", (int)dst);
    return;
  }
  va_end(ap);
}

/*--------------------.
| ARCHIVE a directory |
`--------------------*/

int Archive::pack(const char *dir_name, int recursive) {
  // if the destination type is a file, open the file first
  if (dst_type == AR_FILE) {
    fd = open(dst_fname, O_WRONLY | O_CREAT | O_TRUNC,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd == -1) {
      add_error("%s could not be opened for writing.\n", dst_fname);
      add_error("    because: %s\n", strerror(errno));
      return -1;
    }
  } else {
    if (dst_type == AR_MEM)
      add_error("Packing into memory not implemented yet.\n");
    else if (dst_type == AR_SOCKET)
      add_error("Packing into socket not implemented yet.\n");
    else // if( dst_type == AR_FD)
      add_error("Packing into open descriptor not "
                "implemented yet.\n");
    return -2;
  }

  // copy parameter recursive
  this->recursive = recursive;

  // store the current directory if possible
  char old_cwd[PATH_MAX + 1];
  if (NULL == getcwd(old_cwd, sizeof(old_cwd)))
    sprintf(old_cwd, "/");
  //     {
  // 	add_error( "Archive::pack(): getcwd() failed.\n");
  // 	add_error( "    because: %s\n", strerror( errno));
  // 	return -1;
  //     }

  // change the current directory to dir_name
  if (chdir(dir_name)) {
    add_error("Archive::pack(): chdir(2) to %s failed.\n", dir_name);
    add_error("    because: %s\n", strerror(errno));

    return -1;
  }

  // determine the full path to this directory
  if (NULL == getcwd(root_name, sizeof(root_name))) {
    add_error("Archive::pack(): getcwd() on %s failed.\n", dir_name);
    add_error("    because: %s\n", strerror(errno));
    return -1;
  }

  // get stat() on root_dir
  struct stat stat_buff;
  if (stat(".", &stat_buff)) {
    add_error("Archive::pack(): Cannot stat(2) %s.\n", root_name);
    add_error("    because: %s\n", strerror(errno));

    chdir(old_cwd);
    // 	if( chdir( old_cwd))
    // 	{
    // 	    add_error( "Archive::pack(): to top it off, chdir(2) just "
    // 		       "failed on %s\n", old_cwd);
    // 	    add_error( "    because: %s\n", strerror( errno));
    // 	}
    return -1;
  }

  // set archive_size to ZERO
  archive_size = 0;

  // pack the directory
  int res = _pack(root_name, 0, &stat_buff, 0, 1);

  // change back to the original directory
  chdir(old_cwd);

  // return the result
  return res;
}

int Archive::_pack(const char *fname // name of the directory to pack now
                   ,
                   int level // level (starting at 0)
                   ,
                   struct stat *dir_stat // stat structure of the directory
                   ,
                   double progressMin, double progressMax) {
  // start a AR_DIRECTORY
  add_char(AR_DIRECTORY);

  // put the name of the directory in the archive
  char base_name[4096];
  get_base_name(fname, base_name);
  add_buff(base_name, xstrlen(base_name) + 1);

  // store the mode of the directory
  u_int32_t dir_mode = dir_stat->st_mode;
  dir_mode = htonl(dir_mode);
  add_buff((char *)(&dir_mode), sizeof(dir_mode));

  if (DEBUG && level == 0)
    fprintf(stderr, "%s\n", fname);

  // get a list of files in this directory (but if recursive was set
  // to 0, then pretend the list is empty for any level > 0)
  char **list = NULL;
  long n_files = 0;
  if (recursive || level == 0)
    n_files = dirList(".", &list);

  // stat each file on the list
  struct stat *stat_buff = new struct stat[n_files + 1];
  for (long i = 0; i < n_files; i++) {
    std::string fullName = fname + std::string("/") + list[i];
    if (lstat(list[i], &stat_buff[i])) {
      add_error2("Archive - failure - weird file " + fullName + "\n" +
                 "\t- because on lstat() system reported: " + strerror(errno) +
                 "\n");
      // report error (but free memory before that)
      if (list)
        xfree(list);
      delete[] stat_buff;
      return -1;
    }
  }

  // count directories - for progress reporting
  long n_dirs = 0;
  for (long i = 0; i < n_files; i++) {
    if (S_ISDIR(stat_buff[i].st_mode) && !S_ISLNK(stat_buff[i].st_mode))
      n_dirs++;
  }

  int success = 1;

  // archive every entry in the directory
  long dir_num = 0;
  for (long i = 0; i < n_files; i++) {
    if (progressReporter_) {
      double val = double(dir_num) / n_dirs;
      double prog = (1 - val) * progressMin + val * progressMax;
      progressReporter_->set(prog);
    }

    // archive each entry
    if (DEBUG)
      fprintf(stderr, "%s/%s ", fname, list[i]);

    // prepare a full name into full_name[]
    char full_name[4096];
    strcpy(full_name, fname);
    strcat(full_name, "/");
    strcat(full_name, list[i]);

    // call lstat(2) on this full_name
    struct stat stbuff = stat_buff[i];

    // DEAL WITH SYMBOLIC LINKS
    // ==================================================
    if (S_ISLNK(stbuff.st_mode)) {
      //
      // read the contents of the link
      //
      char link_str[PATH_MAX + 1];
      long link_str_len = readlink(list[i], link_str, sizeof(link_str));
      if (link_str_len == -1) {
        add_error("Archive::_pack(): readlink(%s) "
                  "failed.\n",
                  full_name);
        add_error("    because: %s\n", strerror(errno));
        xfree(list[i]);
        continue;
      }
      // null terminate the name of the link
      link_str[link_str_len] = '\0';
      if (DEBUG)
        fprintf(stderr, " -> %s", link_str);

      // Find out if we want to archive this entry as a
      // link.  Only archive files the links point to if
      // the link starts with '../' and the depth level
      // is 0.
      int archive_as_link = 1;
      if ((level == 0) && (strncmp("../", link_str, 3) == 0))
        archive_as_link = 0;

      if (archive_as_link) {
      archive_link:
        if (DEBUG)
          fprintf(stderr, " storing_link\n");

        add_char(AR_LINK);
        if (add_buff(list[i], xstrlen(list[i]) + 1)) {
          success = 0;
          xfree(list[i]);
          break;
        }
        if (add_buff(link_str, link_str_len + 1)) {
          success = 0;
          xfree(list[i]);
          break;
        }
      } else // archive as a file
      {
        // obtain statistics about this file
        if (stat(list[i], &stbuff)) {
          // archive this as a link instead then
          add_error("Archive::_pack(): cannot "
                    "stat(2) %s. "
                    "Storing as a link.\n",
                    full_name);
          add_error("    because: %s\n", strerror(errno));
          goto archive_link;
        }

        // if this link is a directory, archive it
        // as a link
        if (S_ISDIR(stbuff.st_mode))
          goto archive_link;

        if (DEBUG)
          fprintf(stderr, " storing_file");

        int result = _pack_reg_file(list[i], &stbuff);
        if (result) {
          // error occured
          if (DEBUG)
            fprintf(stderr, " error.\n");
          success = 0;
          break;
        }
        if (DEBUG)
          fprintf(stderr, " success.\n");
      }
    }

    // DEAL WITH SUBDIRECTORIES
    // ==================================================
    else if (S_ISDIR(stbuff.st_mode)) {
      // this entry is a directory - archive it recursively
      // --------------------------------------------------
      // change to the directory
      if (chdir(list[i])) {
        // chdir() was unsuccessful - compose an
        // error message
        add_error("Cannot archive subdirectory '%s'\n"
                  "\t- located in '%s'\n"
                  "\t- because system reported: %s\n"
                  "\t- when trying to execute: "
                  "chdir(%s)\n",
                  list[i], fname, strerror(errno), list[i]);
        success = 0;
        break;
      }
      // pack the insides of the directory
      double r1 = double(dir_num) / n_dirs;
      double r2 = double(dir_num + 1) / n_dirs;
      double pmin = (1 - r1) * progressMin + r1 * progressMax;
      double pmax = (1 - r2) * progressMin + r2 * progressMax;
      dir_num++;
      int result = _pack(full_name, level + 1, &stbuff, pmin, pmax);
      // change back to the previous directory
      if (chdir(fname)) {
        // chdir () was unsuccessful - which is weird -
        // compose an error message
        add_error("Cannot finish packing '%s'\n"
                  "\t- after packing the subdir. '%s'\n"
                  "\t  I could not change dir. back\n"
                  "\t- becuase system reported: %s\n",
                  fname, list[i], strerror(errno));
        success = 0;
        break;
      }
      if (result) {
        // error occured
        success = 0;
        break;
      }
    }

    // DEAL WITH REGULAR FILES
    // ==================================================
    else if (S_ISREG(stbuff.st_mode)) {
      int result = _pack_reg_file(list[i], &stbuff);
      if (result) {
        // error occured
        if (DEBUG)
          fprintf(stderr, "error.\n");
        success = 0;
        break;
      }
      if (DEBUG)
        fprintf(stderr, "success.\n");
    }

    xfree(list[i]);
  }
  if (list) {
    xfree(list);
    list = NULL;
  }

  delete[] stat_buff;

  // end the directory
  add_char(AR_EXIT_DIRECTORY);
  return success ? 0 : -1;
}

/*-----------------------------------.
| adds a regular file to the archive |
`-----------------------------------*/

int Archive::_pack_reg_file(const char *fname, struct stat *st) {
  // open fname for reading
  int src = open(fname, O_RDONLY);
  if (src == -1) {
    add_error("Archive::_pack_reg_file(): Could not open(2) %s "
              "for reading.\n",
              fname);
    add_error("    because: %s\n", strerror(errno));
    return -1;
  }

  // determine the size of the file
  struct stat stat_tmp;
  if (st == NULL) {
    if (stat(fname, &stat_tmp)) {
      add_error("Archive::_pack_reg_file(): Could not stat(2) %s.\n", fname);
      add_error("    because: %s\n", strerror(errno));
      return -1;
    }
    st = &stat_tmp;
  }

  // write the file name and size of the file
  {
    // REGULAR_FILE marker
    add_char(AR_FILE);

    // the name of the file
    add_buff(fname, xstrlen(fname) + 1);

    // mode of the file
    //	u_int32_t net_num = htonl( st-> st_mode & S_IAMB);
    u_int32_t net_num = htonl(st->st_mode);
    add_buff((char *)(&net_num), sizeof(net_num));

    // size of the file
    net_num = htonl(st->st_size);
    add_buff((char *)(&net_num), sizeof(net_num));
  }

  // read 16k at a time and write it
  while (1) {
    char buff[16384];
    long n_read = read(src, buff, sizeof(buff));
    if (n_read == 0)
      break;
    if (n_read < 0) {
      add_error("Archive::_pack_reg_file(): Cannot finish reading %s.\n",
                fname);
      add_error("    because: %s\n", strerror(errno));
      close(src);
      return -1;
    }
    int res = add_buff(buff, n_read);
    if (res) {
      close(src);
      return -1;
    }
  }

  // close the source file
  close(src);

  // return success
  return 0;
}

/*-------------------------------------------.
| adds a single character to the destination |
`-------------------------------------------*/

int Archive::add_char(char c) { return add_buff(&c, 1); }

/*----------------------------------------.
| adds a chunk of data to the destination |
`----------------------------------------*/

int Archive::add_buff(const char *data, long size) {
  if (dst_type == AR_FILE) {
    long remaining = size;
    long ptr = 0;
    while (remaining > 0) {
      long n = write(fd, data + ptr, remaining);
      if (n <= 0) {
        add_error("Archive::add_buff(): write(2) error.\n");
        add_error("    because: %s\n", strerror(errno));
        return -1;
      }
      ptr += n;
      remaining -= n;
    }
    // adjust archive_size
    archive_size += size;

    return 0;
  }

  add_error("Archive::add_buff(): destination type %d not implemented.\n",
            dst_type);
  return -1;
}

void Archive::indented_printf(long indent_level, const char *fmt_str, ...) {
  for (long i = 0; i < indent_level; i++)
    fprintf(stderr, " ");
  va_list arg;
  va_start(arg, fmt_str);
  vfprintf(stderr, fmt_str, arg);
  va_end(arg);
}

/*---------------------------------.
| list the contents of the archive |
`---------------------------------*/

int Archive::list(const char *fname) {
  // open the archive
  fd = open(fname, O_RDONLY);
  if (fd == -1)
    return -1;

  // get the first byte indicating type
  char type;
  if (1 != read(fd, &type, 1)) {
    add_error("Archive::list(%s): cannot read the first entry.\n", fname);
    close(fd);
    return -1;
  }

  // make sure that type is a AR_DIRECTORY
  if (type != AR_DIRECTORY) {
    add_error("Archive::list(%s): first entry is not a directory.\n", fname);
    close(fd);
    return -1;
  }

  int res = _list(".", 0);

  close(fd);
  return res;
}

int Archive::_list(const char *path, int level) {
  // get the name of the directory
  char dir_name[4096];
  if (read_string(dir_name, sizeof(dir_name))) {
    add_error("Archive::_list(): directory name could not be "
              "extracted.\n");
    return -1;
  }

  // get the directory mode
  u_int32_t dir_mode;
  if (4 != read(fd, &dir_mode, sizeof(dir_mode))) {
    add_error("Archive::_list(): dir_mode could not "
              "be extracted.\n");
    add_error("    because: %s\n", strerror(errno));
    return -1;
  }
  dir_mode = ntohl(dir_mode);

  // print the directory name
  fprintf(stderr, "%s/%s/ [%ld]\n", path, dir_name, (long)dir_mode);

  // read all entries until we hit AR_EXIT_DIRECTORY
  while (1) {
    char type;
    if (1 != read(fd, &type, 1)) {
      add_error("Archive::_list(): AR_EXIT_DIRECTORY missing.\n");
      return -1;
    }

    /*------------------.
    | AR_EXIT_DIRECTORY |
    `------------------*/
    if (type == AR_EXIT_DIRECTORY) {
      return 0;
    }

    /*--------.
    | AR_LINK |
    `--------*/
    else if (type == AR_LINK) {
      // read the name of the link
      char link_name[4096];
      if (read_string(link_name, sizeof(link_name))) {
        add_error("Archive::_list(): symbolic link could not "
                  "be extracted.\n");
        return -1;
      }

      // read the contents of the link
      char link_str[4096];
      if (read_string(link_str, sizeof(link_str))) {
        add_error("Archive::_list(): contents of a symbolic link "
                  "could not be extracted.\n");
        return -1;
      }

      // print the symbolic link
      fprintf(stderr, "%s/%s -> %s\n", path, link_name, link_str);
    }

    /*--------.
    | AR_FILE |
    `--------*/

    else if (type == AR_FILE) {
      // read the name of the file
      char file_name[4096];
      if (read_string(file_name, sizeof(file_name))) {
        add_error("Archive::_list(): file name could "
                  "not be extracted.\n");
        return -1;
      }

      // read the mode of the file
      u_int32_t file_mode;
      if (4 != read(fd, &file_mode, sizeof(file_mode))) {
        add_error("Archive::_list(): file mode could not "
                  "be extracted.\n");
        return -1;
      }
      file_mode = ntohl(file_mode);

      // read the length of the file
      u_int32_t file_size;
      if (4 != read(fd, &file_size, sizeof(file_size))) {
        add_error("Archive::_list(): file size could not "
                  "be extracted.\n");
        return -1;
      }
      file_size = ntohl(file_size);

      fprintf(stderr, "%s/%s (%lld,%lld)\n", path, file_name,
              (long long)file_size, (long long)file_mode);

      // read file_size data
      long remain = file_size;
      while (remain > 0) {
        char buff[16384];
        long to_read = sizeof(buff);
        if (to_read > remain)
          to_read = remain;
        long n_read = read(fd, buff, to_read);
        if (n_read <= 0) {
          add_error("Archive::_list(): File %s truncated.\n", file_name);
          return -1;
        }
        remain -= n_read;
      }
    }

    /*----------.
    | AR_DIRECTORY |
    `----------*/

    else if (type == AR_DIRECTORY) {
      // recurse into another directory
      char new_path[4096];
      sprintf(new_path, "%s/%s", path, dir_name);
      int res = _list(new_path, level + 1);
      if (res)
        return res;
    }

    /*-------------.
    | UNKNOWN type |
    `-------------*/

    else {
      add_error("Archive::_list(): archive corrupted.\n");
      return -1;
    }
  }
}

int Archive::read_string(char *buff, u_int32_t buff_size) {
  // read a string terminated by '\0'
  long i = 0;
  while (1) {
    char c;
    if (1 != read(fd, &c, 1)) {
      add_error("Archive::read_string(): failed to read a "
                "character.\n");
      add_error("    because: %s\n", strerror(errno));
      return -1;
    }

    if ((unsigned)i >= buff_size) {
      add_error("Archive::read_string(): string too long.\n");
      return -1;
    }

    buff[i++] = c;
    if (c == 0)
      break;
  }

  return 0;
}

/*-----------------------------------.
| unpack an archive into a directory |
`-----------------------------------*/

int Archive::unpack(const char *archive_name, const char *dst_dir) {
  // open the archive
  fd = open(archive_name, O_RDONLY);
  if (fd == -1)
    return -1;

  // get the first byte indicating type
  char type;
  if (1 != read(fd, &type, 1)) {
    add_error("Archive::unpack(%s): cannot read the first entry.\n",
              archive_name);
    add_error("    because: %s\n", strerror(errno));
    close(fd);
    return -1;
  }
  // make sure that type is a AR_DIRECTORY
  if (type != AR_DIRECTORY) {
    add_error("Archive::unpack(%s): first entry is not a directory.\n",
              archive_name);
    close(fd);
    return -1;
  }

  // remember the current working directory
  char old_dir[PATH_MAX + 1];
  if (NULL == getcwd(old_dir, sizeof(old_dir)))
    sprintf(old_dir, "/");
 
  // chdir() to the destination directory
  if (chdir(dst_dir)) {
    add_error("Archive::unpack(): Cannot chdir(2) to %s.\n", dst_dir);
    add_error("    because: %s\n", strerror(errno));
    close(fd);
    chdir(old_dir);
    return -1;
  }

  // get realpath of the destination
  char real_path[PATH_MAX + 1];
  if (NULL == realpath(".", real_path)) {
    add_error("Archive::unpack(): realpath(2) failed on %s.\n", dst_dir);
    add_error("    because: %s\n", strerror(errno));

    chdir(old_dir);
    close(fd);

    return -1;
  }
  if (DEBUG)
    fprintf(stderr, "realpath(%s) = %s\n", dst_dir, real_path);

  // unpack the archive in the current directory
  int res = _unpack(real_path, 0);

  // change back to the original directory
  chdir(old_dir);

  // close the archive
  close(fd);

  // return the result
  return res;
}

int Archive::_unpack(const char *path, int level) {

  // get the name of the directory
  char dir_name[4096];
  if (read_string(dir_name, sizeof(dir_name))) {
    add_error("Archive::_unpack(): directory name could not be "
              "extracted.\n");
    return -1;
  }

  // get the directory mode
  u_int32_t dir_mode;
  if (sizeof(dir_mode) != read(fd, &dir_mode, sizeof(dir_mode))) {
    add_error("Archive::_unpack(): dir_mode could not "
              "be extracted.\n");
    add_error("    because: %s\n", strerror(errno));
    return -1;
  }
  dir_mode = ntohl(dir_mode);

  // print the directory name
  if (DEBUG)
    fprintf(stderr, "%s/%s/ (%ld)\n", path, dir_name, (long)dir_mode);

  // create the directory
  if (mkdir(dir_name, dir_mode)) {
    add_error("Archive::_unpack(): mkdir(%s) in %s failed.\n", dir_name, path);
    add_error("    because: %s\n", strerror(errno));

    return -1;
  }

  // change into that directory
  if (chdir(dir_name)) {
    add_error("Archive::_unpack(): chdir(2) to %s in %s failed.\n", dir_name,
              path);
    add_error("    because: %s\n", strerror(errno));

    return -1;
  }

  // read all entries until we hit AR_EXIT_DIRECTORY
  while (1) {
    char type;
    if (1 != read(fd, &type, 1)) {
      add_error("Archive::_unpack(): AR_EXIT_DIRECTORY missing.\n");
      add_error("    because: %s\n", strerror(errno));
      return -1;
    }

    /*------------------.
    | AR_EXIT_DIRECTORY |
    `------------------*/
    if (type == AR_EXIT_DIRECTORY) {
      // now we can change to the original directory
      if (chdir(path)) {
        add_error("Archive::_unpack(): Cannot chdir(2) back to %s.\n", path);
        add_error("    because: %s\n", strerror(errno));

        return -1;
      }

      // and return SUCCESS
      return 0;
    }

    /*--------.
    | AR_LINK |
    `--------*/
    else if (type == AR_LINK) {
      // read the name of the link
      char link_name[4096];
      if (read_string(link_name, sizeof(link_name))) {
        add_error("Archive:: unpack(): Symbolic link could not be "
                  "extracted.\n");
        return -1;
      }

      // read the contents of the link
      char link_str[4096];
      if (read_string(link_str, sizeof(link_str))) {
        add_error("Archive::_unpack(): Contents of a symbolic link "
                  "could not be extracted.\n");
        return -1;
      }

      // print the symbolic link
      if (DEBUG)
        fprintf(stderr, "%s/%s -> %s\n", path, link_name, link_str);

      // create the symbolic link
      if (symlink(link_str, link_name)) {
        add_error("Archive::_unpack(): Cannot create a symbolic link "
                  "%s->%s in %s.\n",
                  link_name, link_str, path);
        add_error("    because: %s\n", strerror(errno));
        return -1;
      }
    }

    /*--------.
    | AR_FILE |
    `--------*/

    else if (type == AR_FILE) {
      // read the name of the file
      char file_name[4096];
      if (read_string(file_name, sizeof(file_name))) {
        add_error("Archive::_unpack(): File name could not "
                  "be extracted.\n");
        return -1;
      }

      // read the mode of the file
      u_int32_t file_mode;
      if (sizeof(u_int32_t) != read(fd, &file_mode, sizeof(file_mode))) {
        add_error("Archive::_unpack(): File mode could not be "
                  "extracted.\n");
        return -1;
      }
      file_mode = ntohl(file_mode);

      // read the length of the file
      u_int32_t file_size;
      if (sizeof(u_int32_t) != read(fd, &file_size, sizeof(file_size))) {
        add_error("Archive::_unpack(): File size could not be "
                  "extracted.\n");
        return -1;
      }
      file_size = ntohl(file_size);

      if (DEBUG)
        fprintf(stderr, "%s/%s (%lld,%lld)\n", path, file_name,
                (long long)file_size, (long long)file_mode);

      // create the destination file
      int dst_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, file_mode);
      if (dst_fd == -1) {
        add_error("Archive::_unpack(): Could not create %s in %s.\n", file_name,
                  path);
        add_error("    because: %s\n", strerror(errno));
        return -1;
      }

      // copy the file into destination
      long remain = file_size;
      while (remain > 0) {
        char buff[16384];
        long to_read = sizeof(buff);
        if (to_read > remain)
          to_read = remain;
        long n_read = read(fd, buff, to_read);
        if (n_read <= 0) {
          add_error("Archive::_unpack(): File %s is truncated.\n", file_name);
          add_error("    because: %s\n", strerror(errno));
          close(dst_fd);
          return -1;
        }
        remain -= n_read;

        // write the bytes that we just read
        long n_to_write = n_read;
        long ptr = 0;
        while (n_to_write > 0) {
          long n = write(dst_fd, buff + ptr, n_to_write);
          if (n <= 0) {
            add_error("Archive::_unpack(): write(2) on %s/%s "
                      "failed.\n",
                      path, file_name);
            add_error("    because: %s\n", strerror(errno));
            close(dst_fd);
            return -1;
          }
          ptr += n;
          n_to_write -= n;
        }
      }

      // close the newly created file
      close(dst_fd);
    }

    /*----------.
    | AR_DIRECTORY |
    `----------*/

    else if (type == AR_DIRECTORY) {
      // recurse into another directory
      char new_path[4096];
      sprintf(new_path, "%s/%s", path, dir_name);
      int res = _unpack(new_path, level + 1);
      if (res)
        return res;
    }

    /*-------------.
    | UNKNOWN type |
    `-------------*/

    else {
      add_error("Archive::_unpack(): archive corrupted.\n");
      return -1;
    }
  }
}

/*-------------------------------------------------.
| add_error() - adds a new error to the error list |
`-------------------------------------------------*/

void Archive::add_error(const char *fmt_str, ...) {
  va_list arg;
  va_start(arg, fmt_str);

  // print an error into a buffer
  char buff[16384];
  vsprintf(buff, fmt_str, arg);
  va_end(arg);

  if (DEBUG)
    fprintf(stderr, "%s", buff);

  // append buff[] to errors
  long old_pos = xstrlen(errors);
  long new_len = old_pos + xstrlen(buff) + 1;
  errors = (char *)xrealloc(errors, new_len);
  strcpy(errors + old_pos, buff);
}

void Archive::add_error2(const std::string &str) {
  long old_pos = xstrlen(errors);
  long new_len = old_pos + str.length() + 1;
  errors = (char *)xrealloc(errors, new_len);
  strcpy(errors + old_pos, str.c_str());
}

/*-------------------------------------------.
| get_errors() - returns the value of errors |
`-------------------------------------------*/

char *Archive::get_errors(void) { return errors; }
