#include <stdio.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

typedef int (*RIntPCCharIntVar_FuncType)(const char*, int, ...);
typedef int (*RIntPCCharModeT_FuncType)(const char*, mode_t);
typedef int (*RIntPCCharModeTDevT_FuncType)(const char*, mode_t, dev_t);
typedef int (*RIntPCCharOffT_FuncType)(const char*, off_t);
typedef int (*RIntP2CChar_FuncType)(const char*, const char*);
typedef FILE* (*RFileP2CChar_FuncType)(const char*, const char*);
typedef int (*Execve_FuncType)(const char *, char *const [], char *const []);

static void* handle = NULL;

static RIntPCCharModeT_FuncType real_creat = NULL;
static RIntPCCharModeT_FuncType real_creat64 = NULL;
static RFileP2CChar_FuncType real_fopen = NULL;
static RFileP2CChar_FuncType real_fopen64 = NULL;
static RIntP2CChar_FuncType real_link = NULL;
static RIntPCCharIntVar_FuncType real_open = NULL;
static RIntPCCharIntVar_FuncType real_open64 = NULL;
static RIntPCCharModeT_FuncType real_mkdir = NULL;
static RIntPCCharModeT_FuncType real_mkfifo = NULL;
static RIntPCCharModeTDevT_FuncType real_mknod = NULL;
static RIntP2CChar_FuncType real_rename = NULL;
static RIntP2CChar_FuncType real_symlink = NULL;
static RIntPCCharOffT_FuncType real_truncate = NULL;
#if 0
static Execve_FuncType real_execve = NULL;

static char PreloadStr[] = "LD_PRELOAD=libwatcher.so";
#endif

#define MAX_PATH_LEN 4096

FILE* OpenLogFile()
{
  FILE* file;
  char* filename = NULL;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return NULL;
    }
  }
  if ( !real_fopen )
  {
    real_fopen = (RFileP2CChar_FuncType)dlsym(handle, "fopen");
    if ( !real_fopen )
    {
      fprintf(stderr, "Unable to find symbol fopen\n");
      return NULL;
    }
  }

  filename = getenv("APPROVER_WATCHFILE");
  if ( filename )
    file = real_fopen(filename, "a");
  else
    file = real_fopen("/usr/AppRover/oops.txt", "a");

  return file;
}

int open(const char *pathname, int flags, ...)
{
  va_list vl;
  mode_t mode;
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];
  int readWrite = (flags & 3);

  if ( !pathname )
    return -1;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_open )
  {
    real_open = (RIntPCCharIntVar_FuncType)dlsym(handle, "open");
    if ( !real_open )
    {
      fprintf(stderr, "Unable to find symbol open\n");
      return -1;
    }
  }

  /* Only log files which are being written or created */
  if ( (flags & O_CREAT) || (readWrite == O_WRONLY) || (readWrite == O_RDWR) )
  {
    logFile = OpenLogFile();
    if ( logFile )
    {
      if ( pathname[0] == '/' )
        fprintf(logFile, "FILE %s\n", pathname);
      else
      {
        if ( getcwd(workingDir, MAX_PATH_LEN) )
          fprintf(logFile, "FILE %s/%s\n", workingDir, pathname);
        else
          fprintf(logFile, "FILE /SOMETHING_REALLY_BIG/%s\n", pathname);
      }
      fclose(logFile);
    }
  }

  if ( flags & O_CREAT )
  {
    va_start(vl, flags);
    mode = va_arg(vl, mode_t);
    va_end(vl);
    return real_open(pathname, flags, mode);
  }

  return real_open(pathname, flags);
}

int creat(const char *pathname, mode_t mode)
{
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];

  if ( !pathname )
    return -1;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_creat )
  {
    real_creat = (RIntPCCharModeT_FuncType)dlsym(handle, "creat");
    if ( !real_creat )
    {
      fprintf(stderr, "Unable to find symbol creat\n");
      return -1;
    }
  }

  logFile = OpenLogFile();
  if ( logFile )
  {
    if ( pathname[0] == '/' )
      fprintf(logFile, "FILE %s\n", pathname);
    else
    {
      if ( getcwd(workingDir, MAX_PATH_LEN) )
        fprintf(logFile, "FILE %s/%s\n", workingDir, pathname);
      else
        fprintf(logFile, "FILE /SOMETHING_REALLY_BIG/%s\n", pathname);
    }
    fclose(logFile);
  }

  return real_creat(pathname, mode);
}

FILE *fopen(const char *path, const char *mode)
{
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];

  if ( !path || !mode )
    return NULL;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return NULL;
    }
  }
  if ( !real_fopen )
  {
    real_fopen = (RFileP2CChar_FuncType)dlsym(handle, "fopen");
    if ( !real_fopen )
    {
      fprintf(stderr, "Unable to find symbol fopen\n");
      return NULL;
    }
  }

  /* Only log files which are being written or created */
  if ( (mode[0] != 'r') || strchr(mode, '+') )
  {
    logFile = OpenLogFile();
    if ( logFile )
    {
      if ( path[0] == '/' )
        fprintf(logFile, "FILE %s\n", path);
      else
      {
        if ( getcwd(workingDir, MAX_PATH_LEN) )
          fprintf(logFile, "FILE %s/%s\n", workingDir, path);
        else
          fprintf(logFile, "FILE /SOMETHING_REALLY_BIG/%s\n", path);
      }
      fclose(logFile);
    }
  }

  return real_fopen(path, mode);
}

int link(const char *oldpath, const char *newpath)
{
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];

  if ( !oldpath || !newpath )
    return -1;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_link )
  {
    real_link = (RIntP2CChar_FuncType)dlsym(handle, "link");
    if ( !real_link )
    {
      fprintf(stderr, "Unable to find symbol link\n");
      return -1;
    }
  }

  logFile = OpenLogFile();
  if ( logFile )
  {
    if ( newpath[0] == '/' )
      fprintf(logFile, "LINK %s\n", newpath);
    else
    {
      if ( getcwd(workingDir, MAX_PATH_LEN) )
        fprintf(logFile, "LINK %s/%s\n", workingDir, newpath);
      else
        fprintf(logFile, "LINK /SOMETHING_REALLY_BIG/%s\n", newpath);
    }
    fclose(logFile);
  }

  return real_link(oldpath, newpath);
}

int mkdir(const char *pathname, mode_t mode)
{
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];

  if ( !pathname )
    return -1;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_mkdir )
  {
    real_mkdir = (RIntPCCharModeT_FuncType)dlsym(handle, "mkdir");
    if ( !real_mkdir )
    {
      fprintf(stderr, "Unable to find symbol mkdir\n");
      return -1;
    }
  }

  logFile = OpenLogFile();
  if ( logFile )
  {
    if ( pathname[0] == '/' )
      fprintf(logFile, "DIR %s\n", pathname);
    else
    {
      if ( getcwd(workingDir, MAX_PATH_LEN) )
        fprintf(logFile, "DIR %s/%s\n", workingDir, pathname);
      else
        fprintf(logFile, "DIR /SOMETHING_REALLY_BIG/%s\n", pathname);
    }
    fclose(logFile);
  }

  return real_mkdir(pathname, mode);
}

int mknod(const char *pathname, mode_t mode, dev_t dev)
{
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];

  if ( !pathname )
    return -1;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_mknod )
  {
    real_mknod = (RIntPCCharModeTDevT_FuncType)dlsym(handle, "mknod");
    if ( !real_mknod )
    {
      fprintf(stderr, "Unable to find symbol mknod\n");
      return -1;
    }
  }

  logFile = OpenLogFile();
  if ( logFile )
  {
    if ( pathname[0] == '/' )
      fprintf(logFile, "FILE %s\n", pathname);
    else
    {
      if ( getcwd(workingDir, MAX_PATH_LEN) )
        fprintf(logFile, "FILE %s/%s\n", workingDir, pathname);
      else
        fprintf(logFile, "FILE /SOMETHING_REALLY_BIG/%s\n", pathname);
    }
    fclose(logFile);
  }

  return real_mknod(pathname, mode, dev);
}

int mkfifo(const char *pathname, mode_t mode)
{
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];

  if ( !pathname )
    return -1;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_mkfifo )
  {
    real_mkfifo = (RIntPCCharModeT_FuncType)dlsym(handle, "mkfifo");
    if ( !real_mkfifo )
    {
      fprintf(stderr, "Unable to find symbol mkfifo\n");
      return -1;
    }
  }

  logFile = OpenLogFile();
  if ( logFile )
  {
    if ( pathname[0] == '/' )
      fprintf(logFile, "FILE %s\n", pathname);
    else
    {
      if ( getcwd(workingDir, MAX_PATH_LEN) )
        fprintf(logFile, "FILE %s/%s\n", workingDir, pathname);
      else
        fprintf(logFile, "FILE /SOMETHING_REALLY_BIG/%s\n", pathname);
    }
    fclose(logFile);
  }

  return real_mkfifo(pathname, mode);
}

int rename(const char *oldpath, const char *newpath)
{
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];

  if ( !oldpath || !newpath )
    return -1;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_rename )
  {
    real_rename = (RIntP2CChar_FuncType)dlsym(handle, "rename");
    if ( !real_rename )
    {
      fprintf(stderr, "Unable to find symbol rename\n");
      return -1;
    }
  }

  logFile = OpenLogFile();
  if ( logFile )
  {
    if ( newpath[0] == '/' )
      fprintf(logFile, "RENAME %s\n", newpath);
    else
    {
      if ( getcwd(workingDir, MAX_PATH_LEN) )
        fprintf(logFile, "RENAME %s/%s\n", workingDir, newpath);
      else
        fprintf(logFile, "RENAME /SOMETHING_REALLY_BIG/%s\n", newpath);
    }
    fclose(logFile);
  }

  return real_rename(oldpath, newpath);
}

int symlink(const char *oldpath, const char *newpath)
{
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];

  if ( !oldpath || !newpath )
    return -1;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_symlink )
  {
    real_symlink = (RIntP2CChar_FuncType)dlsym(handle, "symlink");
    if ( !real_symlink )
    {
      fprintf(stderr, "Unable to find symbol symlink\n");
      return -1;
    }
  }

  logFile = OpenLogFile();
  if ( logFile )
  {
    if ( newpath[0] == '/' )
      fprintf(logFile, "SYMLINK %s\n", newpath);
    else
    {
      if ( getcwd(workingDir, MAX_PATH_LEN) )
        fprintf(logFile, "SYMLINK %s/%s\n", workingDir, newpath);
      else
        fprintf(logFile, "SYMLINK /SOMETHING_REALLY_BIG/%s\n", newpath);
    }
    fclose(logFile);
  }

  return real_symlink(oldpath, newpath);
}

int truncate(const char *path, off_t length)
{
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];

  if ( !path )
    return -1;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_truncate )
  {
    real_truncate = (RIntPCCharOffT_FuncType)dlsym(handle, "truncate");
    if ( !real_truncate )
    {
      fprintf(stderr, "Unable to find symbol truncate\n");
      return -1;
    }
  }

  logFile = OpenLogFile();
  if ( logFile )
  {
    if ( path[0] == '/' )
      fprintf(logFile, "FILE %s\n", path);
    else
    {
      if ( getcwd(workingDir, MAX_PATH_LEN) )
        fprintf(logFile, "FILE %s/%s\n", workingDir, path);
      else
        fprintf(logFile, "FILE /SOMETHING_REALLY_BIG/%s\n", path);
    }
    fclose(logFile);
  }

  return real_truncate(path, length);
}

int open64(const char *pathname, int flags, ...)
{
  va_list vl;
  mode_t mode;
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];
  int readWrite = (flags & 3);

  if ( !pathname )
    return -1;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_open64 )
  {
    real_open64 = (RIntPCCharIntVar_FuncType)dlsym(handle, "open64");
    if ( !real_open64 )
    {
      fprintf(stderr, "Unable to find symbol open64\n");
      return -1;
    }
  }

  /* Only log files which are being written or created */
  if ( (flags & O_CREAT) || (readWrite == O_WRONLY) || (readWrite == O_RDWR) )
  {
    logFile = OpenLogFile();
    if ( logFile )
    {
      if ( pathname[0] == '/' )
        fprintf(logFile, "FILE %s\n", pathname);
      else
      {
        if ( getcwd(workingDir, MAX_PATH_LEN) )
          fprintf(logFile, "FILE %s/%s\n", workingDir, pathname);
        else
          fprintf(logFile, "FILE /SOMETHING_REALLY_BIG/%s\n", pathname);
      }
      fclose(logFile);
    }
  }

  if ( flags & O_CREAT )
  {
    va_start(vl, flags);
    mode = va_arg(vl, mode_t);
    va_end(vl);
    return real_open64(pathname, flags, mode);
  }

  return real_open64(pathname, flags);
}

int creat64(const char *pathname, mode_t mode)
{
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];

  if ( !pathname )
    return -1;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_creat64 )
  {
    real_creat64 = (RIntPCCharModeT_FuncType)dlsym(handle, "creat64");
    if ( !real_creat64 )
    {
      fprintf(stderr, "Unable to find symbol creat64\n");
      return -1;
    }
  }

  logFile = OpenLogFile();
  if ( logFile )
  {
    if ( pathname[0] == '/' )
      fprintf(logFile, "FILE %s\n", pathname);
    else
    {
      if ( getcwd(workingDir, MAX_PATH_LEN) )
        fprintf(logFile, "FILE %s/%s\n", workingDir, pathname);
      else
        fprintf(logFile, "FILE /SOMETHING_REALLY_BIG/%s\n", pathname);
    }
    fclose(logFile);
  }

  return real_creat64(pathname, mode);
}

FILE *fopen64(const char *path, const char *mode)
{
  FILE* logFile;
  char workingDir[MAX_PATH_LEN];

  if ( !path || !mode )
    return NULL;

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return NULL;
    }
  }
  if ( !real_fopen64 )
  {
    real_fopen64 = (RFileP2CChar_FuncType)dlsym(handle, "fopen64");
    if ( !real_fopen64 )
    {
      fprintf(stderr, "Unable to find symbol fopen64\n");
      return NULL;
    }
  }

  /* Only log files which are being written or created */
  if ( (mode[0] != 'r') || strchr(mode, '+') )
  {
    logFile = OpenLogFile();
    if ( logFile )
    {
      if ( path[0] == '/' )
        fprintf(logFile, "FILE %s\n", path);
      else
      {
        if ( getcwd(workingDir, MAX_PATH_LEN) )
          fprintf(logFile, "FILE %s/%s\n", workingDir, path);
        else
          fprintf(logFile, "FILE /SOMETHING_REALLY_BIG/%s\n", path);
      }
      fclose(logFile);
    }
  }

  return real_fopen64(path, mode);
}

#if 0
/* This is unnecessary unless we want to look out for people who are explicitly
   removing stuff from the environment list. Same goes for execle and fexecve */
int execve(const char *filename, char *const argv[], char *const envp[])
{
  FILE* logFile;
  int i, numArgs;
  char** newEnvList = NULL;
  char logfileEnv[MAX_PATH_LEN];

  if ( !handle )
  {
    handle = dlopen("libc.so.6", RTLD_LAZY);
    if ( !handle )
    {
      fprintf(stderr, "Unable to dlopen libc.so\n");
      return -1;
    }
  }
  if ( !real_execve )
  {
    real_execve = (Execve_FuncType)dlsym(handle, "execve");
    if ( !real_execve )
    {
      fprintf(stderr, "Unable to find symbol execve\n");
      return -1;
    }
  }

  numArgs = 0;
  while ( envp[numArgs] )
    numArgs++;
  
  newEnvList = (char**)malloc( (numArgs + 2) * sizeof(char*) );
  if ( !newEnvList )
  {
    fprintf(stderr, "Error allocating environment list\n");
    return -1;
  }

  sprintf(logfileEnv, "APPROVER_WATCHFILE=%s", getenv("APPROVER_WATCHFILE"));
  newEnvList[0] = strdup(PreloadStr);
  newEnvList[1] = strdup(logfileEnv);
  for (i = 0; i < numArgs; i++)
    newEnvList[i + 2] = envp[i];
  newEnvList[numArgs + 2] = NULL;

#if 1
  logFile = OpenLogFile();
  if ( logFile )
  {
    fprintf(logFile, "execve %s\n", filename);
#if 0
    for (i = 0; i < numArgs; i++)
      fprintf(logFile, "Env %s\n", envp[i]);
#endif
    fclose(logFile);
  }
#endif

  return real_execve(filename, argv, newEnvList);
}
#endif
