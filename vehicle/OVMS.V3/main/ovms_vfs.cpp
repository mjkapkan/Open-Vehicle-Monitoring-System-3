/*
;    Project:       Open Vehicle Monitor System
;    Date:          14th March 2017
;
;    Changes:
;    1.0  Initial release
;
;    (C) 2011       Michael Stegen / Stegen Electronics
;    (C) 2011-2017  Mark Webb-Johnson
;    (C) 2011        Sonny Chen @ EPRO/DX
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
; THE SOFTWARE.
*/

#include "ovms_log.h"
static const char *TAG = "vfs";

#include <string>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <libgen.h>
#include "ovms_vfs.h"
#include "ovms_config.h"
#include "ovms_command.h"
#include "crypt_md5.h"

#ifdef CONFIG_OVMS_COMP_EDITOR
#include "vfsedit.h"
#endif // #ifdef CONFIG_OVMS_COMP_EDITOR

void vfs_ls(int verbosity, OvmsWriter* writer, OvmsCommand* cmd, int argc, const char* const* argv)
  {
  DIR *dir;
  struct dirent *dp;
  if (argc == 0)
    {
    if ((dir = opendir (".")) == NULL)
      {
      writer->puts("Error: VFS cannot open directory listing");
      return;
      }
    }
  else
    {
    if (MyConfig.ProtectedPath(argv[0]))
      {
      writer->puts("Error: protected path");
      return;
      }
    if ((dir = opendir (argv[0])) == NULL)
      {
      writer->puts("Error: VFS cannot open directory listing for that directory");
      return;
      }
    }

  while ((dp = readdir (dir)) != NULL)
    {
    writer->puts(dp->d_name);
    }

  closedir(dir);
  }

void vfs_cat(int verbosity, OvmsWriter* writer, OvmsCommand* cmd, int argc, const char* const* argv)
  {
  if (MyConfig.ProtectedPath(argv[0]))
    {
    writer->puts("Error: protected path");
    return;
    }

  FILE* f = fopen(argv[0], "r");
  if (f == NULL)
    {
    writer->puts("Error: VFS file cannot be opened");
    return;
    }

  char buf[512];
  while(size_t n = fread(buf, sizeof(char), sizeof(buf), f))
    {
    writer->write(buf, n);
    }
  fclose(f);
  }

void vfs_stat(int verbosity, OvmsWriter* writer, OvmsCommand* cmd, int argc, const char* const* argv)
  {
  if (MyConfig.ProtectedPath(argv[0]))
    {
    writer->puts("Error: protected path");
    return;
    }

  FILE* f = fopen(argv[0], "r");
  if (f == NULL)
    {
    writer->puts("Error: VFS file cannot be opened");
    return;
    }

  OVMS_MD5_CTX* md5 = new OVMS_MD5_CTX;
  OVMS_MD5_Init(md5);
  int filesize = 0;
  uint8_t *buf = new uint8_t[512];
  while(size_t n = fread(buf, sizeof(char), 512, f))
    {
    filesize += n;
    OVMS_MD5_Update(md5, buf, n);
    }
  uint8_t* rmd5 = new uint8_t[16];
  OVMS_MD5_Final(rmd5, md5);

  char dchecksum[33];
  sprintf(dchecksum,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
    rmd5[0],rmd5[1],rmd5[2],rmd5[3],rmd5[4],rmd5[5],rmd5[6],rmd5[7],
    rmd5[8],rmd5[9],rmd5[10],rmd5[11],rmd5[12],rmd5[13],rmd5[14],rmd5[15]);
  writer->printf("File %s size is %d and digest %s\n",
    argv[0],filesize,dchecksum);

  delete [] rmd5;
  delete [] buf;
  fclose(f);
  }

void vfs_rm(int verbosity, OvmsWriter* writer, OvmsCommand* cmd, int argc, const char* const* argv)
  {
  if (MyConfig.ProtectedPath(argv[0]))
    {
    writer->puts("Error: protected path");
    return;
    }

  if (unlink(argv[0]) == 0)
    { writer->puts("VFS File deleted"); }
  else
    { writer->puts("Error: Could not delete VFS file"); }
  }

void vfs_mv(int verbosity, OvmsWriter* writer, OvmsCommand* cmd, int argc, const char* const* argv)
  {
  if (MyConfig.ProtectedPath(argv[0]))
    {
    writer->puts("Error: protected path");
    return;
    }
  if (MyConfig.ProtectedPath(argv[1]))
    {
    writer->puts("Error: protected path");
    return;
    }
  if (rename(argv[0],argv[1]) == 0)
    { writer->puts("VFS File renamed"); }
  else
    { writer->puts("Error: Could not rename VFS file"); }
  }

void vfs_mkdir(int verbosity, OvmsWriter* writer, OvmsCommand* cmd, int argc, const char* const* argv)
  {
  if (MyConfig.ProtectedPath(argv[0]))
    {
    writer->puts("Error: protected path");
    return;
    }

  if (mkdir(argv[0],0) == 0)
    { writer->puts("VFS directory created"); }
  else
    { writer->puts("Error: Could not create VFS directory"); }
  }

void vfs_rmdir(int verbosity, OvmsWriter* writer, OvmsCommand* cmd, int argc, const char* const* argv)
  {
  if (MyConfig.ProtectedPath(argv[0]))
    {
    writer->puts("Error: protected path");
    return;
    }

  if (rmdir(argv[0]) == 0)
    { writer->puts("VFS directory removed"); }
  else
    { writer->puts("Error: Could not remove VFS directory"); }
  }

void vfs_cp(int verbosity, OvmsWriter* writer, OvmsCommand* cmd, int argc, const char* const* argv)
  {
  if (MyConfig.ProtectedPath(argv[0]))
    {
    writer->puts("Error: protected path");
    return;
    }
  if (MyConfig.ProtectedPath(argv[1]))
    {
    writer->puts("Error: protected path");
    return;
    }

  FILE* f = fopen(argv[0], "r");
  if (f == NULL)
    {
    writer->puts("Error: VFS source file cannot be opened");
    return;
    }

  FILE* w = fopen(argv[1], "w");
  if (w == NULL)
    {
    writer->puts("Error: VFS target file cannot be opened");
    fclose(f);
    return;
    }

  char buf[512];
  while(size_t n = fread(buf, sizeof(char), sizeof(buf), f))
    {
    fwrite(buf,n,1,w);
    }
  fclose(w);
  fclose(f);
  writer->puts("VFS copy complete");
  }

void vfs_append(int verbosity, OvmsWriter* writer, OvmsCommand* cmd, int argc, const char* const* argv)
  {
  if (MyConfig.ProtectedPath(argv[1]))
    {
    writer->puts("Error: protected path");
    return;
    }

  FILE* w = fopen(argv[1], "a");
  if (w == NULL)
    {
    writer->puts("Error: VFS target file cannot be opened");
    return;
    }

  int len = strlen(argv[0]);
  fwrite(argv[0], len, 1, w);
  fwrite("\n", 1, 1, w);
  fclose(w);
  }


class VfsTailCommand : public OvmsCommandTask
  {
  using OvmsCommandTask::OvmsCommandTask;
  public:
    int fd = -1;
    char buf[128];
    off_t fpos = -1;
    ssize_t len;
    
    void Service()
      {
      while (true)
        {
        // Note: the esp-idf libs do currently not provide read/seek into
        // data appended by other tasks, we need to reopen for each check
        fd = open(argv[0], O_RDONLY|O_NONBLOCK);
        if (fd == -1)
          {
          writer->puts("Error: VFS file cannot be opened");
          return;
          }
        
        if (fpos == -1)
          {
          fpos = lseek(fd, 0, SEEK_END);
          if (fpos > 0)
            fpos = lseek(fd, -MIN(500, fpos), SEEK_END);
          }
        else
          {
          lseek(fd, fpos, SEEK_SET);
          }
        
        while ((len = read(fd, buf, sizeof buf)) > 0)
          writer->write(buf, len);
        fpos = lseek(fd, 0, SEEK_CUR);
        close(fd);
        
        if (IsTerminated())
          return;
        
        vTaskDelay(pdMS_TO_TICKS(250));
        }
      }
    
    static void Execute(int verbosity, OvmsWriter* writer, OvmsCommand* cmd, int argc, const char* const* argv)
      {
      if (MyConfig.ProtectedPath(argv[0]))
        {
        writer->puts("Error: protected path");
        return;
        }
      
      new VfsTailCommand(verbosity, writer, cmd, argc, argv);
      }
  };


class VfsInit
  {
  public: VfsInit();
} MyVfsInit  __attribute__ ((init_priority (5200)));

VfsInit::VfsInit()
  {
  ESP_LOGI(TAG, "Initialising VFS (5200)");

  OvmsCommand* cmd_vfs = MyCommandApp.RegisterCommand("vfs","VFS framework",NULL,"$C <file(s)>",0,0,true);
  cmd_vfs->RegisterCommand("ls","VFS Directory Listing",vfs_ls, "[<file>]", 0, 1, true);
  cmd_vfs->RegisterCommand("cat","VFS Display a file",vfs_cat, "<file>", 1, 1, true);
  cmd_vfs->RegisterCommand("stat","VFS Status of a file",vfs_stat, "<file>", 1, 1, true);
  cmd_vfs->RegisterCommand("mkdir","VFS Create a directory",vfs_mkdir, "<path>", 1, 1, true);
  cmd_vfs->RegisterCommand("rmdir","VFS Delete a directory",vfs_rmdir, "<path>", 1, 1, true);
  cmd_vfs->RegisterCommand("rm","VFS Delete a file",vfs_rm, "<file>", 1, 1, true);
  cmd_vfs->RegisterCommand("mv","VFS Rename a file",vfs_mv, "<source> <target>", 2, 2, true);
  cmd_vfs->RegisterCommand("cp","VFS Copy a file",vfs_cp, "<source> <target>", 2, 2, true);
  cmd_vfs->RegisterCommand("append","VFS Append a line to a file",vfs_append, "<quoted line> <file>", 2, 2, true);
  cmd_vfs->RegisterCommand("tail","VFS output tail of a file",VfsTailCommand::Execute, "<file>", 1, 1, true);
  #ifdef CONFIG_OVMS_COMP_EDITOR
  cmd_vfs->RegisterCommand("edit","VFS edit a file",vfs_edit, "<path>", 1, 1, true);
  #endif // #ifdef CONFIG_OVMS_COMP_EDITOR

  }
