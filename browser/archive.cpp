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



#include <dirent.h>
#include <iostream>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


#include <strings.h>


#include <errno.h>

#include "archive.h"
#include "dsprintf.h"
#include "utilities.h"
#include "xmemory.h"

#define LINK 'L'
#define REGULAR_FILE 'R'
#define DIRECTORY 'D'
#define EXIT_DIRECTORY 'E'

static int _archive(RA_Connection *connection, char *name, FILE *f);
static int writeFile(RA_Connection *connection, char *name, FILE *f);
static int writeLink(RA_Connection *connection, char *name, FILE *f);
static int _dearchive(RA_Connection *connection, FILE *f, char *destDir);
static int readFile(RA_Connection *connection, FILE *fin, char *destDir);
static int readLink(RA_Connection *connection, FILE *f, char *destDir);
static int readName(FILE *f, char *name);
static int readLong(FILE *f, unsigned long *l);
static int writeLong(FILE *f, unsigned long l);

static long fileLength;
static long filePos;

/*****************************************************************************
 *
 * archive()
 * will create an archive in 'outFile' of the directory 'name'.
 * Links will be preserved.
 *
 * if recursive is set to 0, then the subdirectories are omitted
 *
 */

int archive(RA_Connection *connection, char *dirName, char *outFile,
            int recursive) {
  char *baseName;
  FILE *f;

  // open the destination file
  f = fopen(outFile, "w");
  if (f == NULL) {
    perror("archive.c::archive():fopen()");
    fprintf(stderr, "Couldn't open '%s' for writing.\n", outFile);
    return 1;
  }

  baseName = getBaseName(dirName);

  // first write the name to the archive
  fputc(DIRECTORY, f);
  fprintf(f, "%s", baseName);
  fputc(0, f);

  // get a list of files in the directory
  char **list;
  int nFiles = RA::Get_dir(connection, dirName, &list);
  if (nFiles <= 0) {
    // no directories inside
    fputc(EXIT_DIRECTORY, f);
    fclose(f);
    return 0;
  }

  // go through the list of files and archive them
  int success = 1;
  int i;
  for (i = 0; i < nFiles; i++) {
    // get statistics about this entry
    char fullPath[4096];
    sprintf(fullPath, "%s/%s", dirName, list[i]);
    RA_Stat_Struc statBuf;
    if (RA::Stat(connection, fullPath, &statBuf)) {
      // error while retrieving information about this entry
      RA::Error("archive.c::archive()");
      fprintf(stderr,
              "archive.c::archive():Could not archive entry"
              "                    :       %s\n",
              fullPath);
      continue;
    }
    if (statBuf.type == RA_NOEXIST_TYPE) {
      // this file doesn't really exist... strange error, ignore it
      continue;
    }

    // is this entry a link?
    if (statBuf.is_link) {
      // the entry is a link, but we have to check whether
      // it isn't a link to a directory
      if (statBuf.type == RA_DIR_TYPE) {
        // the file is a link to a directory, so store the link only
        // if 'recursive' is set
        if (recursive != 0) {
          if (0 != writeLink(connection, fullPath, f)) {
            // error occured while writing a link
            success = 0;
            break;
          }
        }
      } else { // ! RA_DIR_TYPE
        // the entry is a link to a regular file,
        // so store the link
        if (0 != writeLink(connection, fullPath, f)) {
          // error occured while writing a file
          success = 0;
          break;
        }
      }
    } else { // ! is_link
      // the entry is not a link
      if (statBuf.type == RA_DIR_TYPE) {
        // the entry is a directory, so archive it (if 'recursive!=0')
        if (recursive != 0) {
          if (_archive(connection, fullPath, f)) {
            // error while archiving subdirectory
            success = 0;
            break;
          }
        }
      } else { // ! RA_DIR_TYPE
        // the entry is a regular file, copy it into the archive
        if (0 != writeFile(connection, fullPath, f)) {
          // error while writing a file
          success = 0;
          break;
        }
      }
    }
  }

  // finish the archive
  if (success)
    fputc(EXIT_DIRECTORY, f);
  fclose(f);

  // destroy the status dialog

  // destroy the list of files
  for (i = 0; i < nFiles; i++)
    xfree(list[i]);
  xfree(list);

  return !success;
}

/******************************************************************************
 *
 * writes the control code "REGULAR_FILE" to f, then the name, then code 0,
 * then the size of the file, and then the file itself
 *
 * returns:  0 = success
 *          !0 = error
 */
static int writeFile(RA_Connection *connection, char *name, FILE *f) {
  // read the file into a buffer
  char *buffer;
  long size;
  if (RA::Read_file(connection, name, buffer, size)) {
    RA::Error("archive.c::writeFile():Read_file()");
    return 1;
  }

  // get the base name of the object
  char *baseName = getBaseName(name);

  // first write the control code
  fputc(REGULAR_FILE, f);

  // now write the file's name (basename)
  fprintf(f, "%s", baseName);
  fputc(0, f);

  // now write the length of the file
  if (0 != writeLong(f, size)) {
    xfree(buffer);
    return 1;
  }

  // write the file into archive
  if (1 != fwrite(buffer, size, 1, f)) {
    perror("archive.c::writeFile():fwrite()");
    xfree(buffer);
    return 1;
  }

  xfree(buffer);

  // no errors
  return 0;
}

/******************************************************************************
 *
 * writes to 'f' the LINK code, then the name, then code 0, then the
 * link, then the code 0
 *
 */
static int writeLink(RA_Connection *connection, char *name, FILE *f) {
  // get the base name
  char *baseName = getBaseName(name);

  // read the link
  char *linkName;
  if (RA::Readlink(connection, name, linkName)) {
    RA::Error("archive.c::writeLink()");
    return 1;
  }

  fputc(LINK, f);
  fprintf(f, "%s", baseName);
  fputc(0, f);
  fprintf(f, "%s", linkName);
  fputc(0, f);

  // no errors
  return 0;
}

/******************************************************************************
 *
 * _archive is doing basically the same thing as archive, but if there is a
 * link, it stores it as a link (in all cases), and it assumes:
 *
 * the current directory is that of the parent to the file 'name'.
 * the name is just the base name of the directory that needs to be
 *    archived
 *
 */
static int _archive(RA_Connection *connection, char *dirName, FILE *f) {
  // get the base name of this directry
  char *baseName = getBaseName(dirName);

  // first write the dir_name to the archive
  fputc(DIRECTORY, f);
  fprintf(f, "%s", baseName);
  fputc(0, f);

  // archive all the regular files and directories
  char **list;
  int nFiles = RA::Get_dir(connection, dirName, &list);
  if (nFiles <= 0) {
    // nothing to archive
    fputc(EXIT_DIRECTORY, f);
    return 0;
  }

  // archive the files one by one
  int success = 1;
  int i;
  for (i = 0; i < nFiles; i++) {
    // get statistics about this entry
    char fullPath[4096];
    sprintf(fullPath, "%s/%s", dirName, list[i]);
    RA_Stat_Struc statBuf;
    if (RA::Stat(connection, fullPath, &statBuf)) {
      // error while retrieving information about this entry
      RA::Error("archive.c::_archive()");
      fprintf(stderr,
              "archive.c::_archive():Could not archive entry"
              "                     :       %s\n",
              fullPath);
      continue;
    }
    if (statBuf.type == RA_NOEXIST_TYPE) {
      // this file doesn't really exist... strange error, ignore it
      continue;
    }

    // is this entry a link?
    if (statBuf.is_link) {
      // the file is a link, so just copy the link into the archive
      if (0 != writeLink(connection, fullPath, f)) {
        // error while writing a link
        success = 0;
        break;
      }
    } else {
      // the entry is not a link, so check if it's a regular
      // file, or a directory
      if (statBuf.type == RA_DIR_TYPE) {
        // the entry is a directory, so archive it
        if (0 != _archive(connection, fullPath, f)) {
          // error occured while archiving directory
          success = 0;
          break;
        }
      } else {
        // the entry is a regular file, so archive it */
        if (0 != writeFile(connection, fullPath, f)) {
          // close the directory
          success = 0;
          break;
        }
      }
    }
  }

  // we are done with this directory
  if (success)
    fputc(EXIT_DIRECTORY, f);

  // destroy the list of files
  for (i = 0; i < nFiles; i++)
    xfree(list[i]);
  xfree(list);

  // no error
  return !success;
}

/******************************************************************************
 *
 * dearchive will dearchive the 'archivefile' in the current directory
 * ( returns 0 if successful, returns !0 otherwise )
 *
 */
int dearchive(RA_Connection *connection, char *archive, char *destDir) {
  int result;
  FILE *f;

  // find out the total lengh of the archive - for status reporting
  fileLength = getFileLength(archive);
  filePos = 0;

  fprintf(stderr, "dearchive(): archive name: %s\n", archive);
  fflush(stderr);

  // open the archive
  f = fopen(archive, "r");
  if (f == NULL) {
    perror("fopen");
    return 1;
  }

  // do the actual dearchiving
  result = _dearchive(connection, f, destDir);

  // close the file
  fclose(f);


  return result;
}

/******************************************************************************
 *
 * _dearchive will dearchive the contents of the archive that 'f' points to
 * ( returns 0 if successful, returns !0 otherwise ) into directory
 * specified in 'destDir'
 *
 */
static int _dearchive(RA_Connection *connection, FILE *f, char *destDir) {
  int c;
  char dirName[4096];
  // read the code from the archive - must be a 'DIRECTORY'
  c = fgetc(f);
  fprintf(stderr, "expected %c received %c\n", DIRECTORY, c);
  filePos++;
  if (c != DIRECTORY) {
    fprintf(stderr, "Archive is corrupted\n");
    c = fgetc(f);
    fprintf(stderr, "expected %c received %c\n", DIRECTORY, c);
    c = fgetc(f);
    fprintf(stderr, "expected %c received %c\n", DIRECTORY, c);
    c = fgetc(f);
    fprintf(stderr, "expected %c received %c\n", DIRECTORY, c);
    c = fgetc(f);
    fprintf(stderr, "expected %c received %c\n", DIRECTORY, c);
    c = fgetc(f);
    fprintf(stderr, "expected %c received %c\n", DIRECTORY, c);
    fflush(stderr);
    return 1;
  }
  if (readName(f, dirName) != 0) {
    fprintf(stderr, "Archive is corrupted\n");
    fflush(stderr);
    return 1;
  }
  // create the directory
  char *fullDirName = dsprintf("%s/%s", destDir, dirName);
  if (RA::Mkdir(connection, fullDirName, 0755)) {
    perror("mkdir");
    xfree(fullDirName);
    return 1;
  }
  // unpack everything in this directory
  int success = 0;
  while (1) {
    // read a character from the archive
    c = fgetc(f);
    // if it is 'EXIT_DIRECTORY', exit the loop
    if (c == EXIT_DIRECTORY) {
      success = 1;
      break;
    }
    // update the progression meter
    filePos++;
    // if this is a regular file, unpack it
    if (c == REGULAR_FILE) {
      if (0 != readFile(connection, f, fullDirName))
        break;
    } else if (c == LINK) {
      if (0 != readLink(connection, f, fullDirName))
        break;
    } else if (c == DIRECTORY) {
      ungetc(c, f);
      filePos--;
      if (_dearchive(connection, f, fullDirName) != 0)
        break;
    } else {
      fprintf(stderr, "The archive is corrupt.\n");
      break;
    }
  }
  filePos++;

  // we don't need full dirName
  xfree(fullDirName);

  // return errors
  return !success;
}

/******************************************************************************
 *
 * reads a name from file f until there is a '\0'
 * ( returns 0 if successful, returns !0 otherwise )
 */
static int readName(FILE *f, char *name) {
  int i = 0;
  int c;

  while ((c = fgetc(f)) != '\0') {
    filePos++;
    name[i++] = (char)c;
    if (feof(f))
      return 1;
  }
  filePos++;
  name[i] = '\0';

  /* no errors */
  return 0;
}

/******************************************************************************
 *
 * read the file from the archive and write it to the disk
 * ( returns 0 if successful, returns !0 otherwise )
 *
 */
static int readFile(RA_Connection *connection, FILE *fin, char *destDir) {
  char name[4096];
  unsigned long size;
  char *buffer;

  // read the name of the file
  if (0 != readName(fin, name))
    return 1;

  // read in the size of the file
  if (0 != readLong(fin, &size))
    return 1;

  // allocate memory for the whole file
  if (NULL == (buffer = (char *)xmalloc(size)))
    return 1;

  // read the entire content of the file into memory
  if (1 != fread(buffer, size, 1, fin)) {
    filePos += size;
    perror("fopen");
    xfree(buffer);
    return 1;
  }
  filePos += size;

  // write to the destination file
  char destFile[4096];
  sprintf(destFile, "%s/%s", destDir, name);
  if (RA::Write_file(connection, destFile, buffer, size)) {
    RA::Error("archive.c::readFile():WriteFile()");
    xfree(buffer);
    return 1;
  }

  // free the memory
  xfree(buffer);

  // no errors
  return 0;
}

/******************************************************************************
 *
 * read link will read the link from the archive and create it in the
 * file system
 * ( returns 0 if successful, returns !0 otherwise )
 *
 */
static int readLink(RA_Connection *connection, FILE *f, char *destDir) {
  char name1[4096];
  char name2[4096];
  char fullName[4096];

  // read the name of the link
  if (0 != readName(f, name1))
    return 1;

  // read the content of the link
  if (0 != readName(f, name2))
    return 1;

  // create the link
  sprintf(fullName, "%s/%s", destDir, name1);
  if (RA::Symlink(connection, name2, fullName)) {
    // try to remove the link
    RA::Unlink(connection, fullName);
    // and try to create the link again
    if (RA::Symlink(connection, name2, fullName)) {
      RA::Error("archive.c::readLink()");
      return 1;
    }
  }

  // no error
  return 0;
}

/******************************************************************************
 *
 * readLong will read a long from the file f
 *
 */
static int readLong(FILE *f, unsigned long *l) {

  if (1 != fread(l, sizeof(unsigned long), 1, f)) {
    filePos += sizeof(unsigned long);
    return 1;
  }
  filePos += sizeof(unsigned long);

  return 0;
}

/******************************************************************************
 *
 * will write a long to the file f
 *
 */
static int writeLong(FILE *f, unsigned long l) {
  if (1 != fwrite(&l, sizeof(unsigned long), 1, f))
    return 1;

  return 0;
}
