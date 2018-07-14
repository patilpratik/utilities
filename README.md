# utilities
This is a collection of following utilities useful for various purpose

  1) ptar: simple taring utility
  2) ptrace : light weight logging utility 

  
  ## ptar
    A simple light weight taring utility with minimum required operations like open, read, write, seek, close.
    For POSIX systems, mmap is used. If cross-compiling for non POSIX system, make sure to remove POSIX_SYSTEM macro from ptar.h
    
    Currently it has support for only files. taring directories are not supported. 
  
  ## ptrace
    This is macro based simple logging module with 4 log levels to control the amount of information to be logged.
    
    ### Logging Levels
    It defines 4 levels of logging.
       1) NO_LOG : it will not log anything
       2) ERROR_LEVEL : Used to mark errors.
       3) INFO_LEVEL  : Used to log information
       4) DEBUG_LEVEL : Used to log info to assist in debugging
   
       You can define LOG_LEVEL to any one of the above. DEBUG_LEVEL has the highest level i.e if
       LOG_LEVEL is defined to DEBUG_LEVEL, all other logs will also be recorded.
       This will control information that logged.
   
    ### Log Files
    You can use ERR_STREAM macro to define the stream where errors will be logged
    If this macro is not defined, errors will be put to stderr.
   
    Following could be the way of using this
   
    #define LOG_LEVEL ERROR_LEVEL
    #ifdef ERR_STREAM
    #undef ERR_STREAM
    #endif
    #define LOG_FILE_PATH "error.log"
    #define ERR_STREAM fopen(LOG_FILE_PATH, "a+")
     ...
     PTrace(ERROR_LEVEL, "Failed to open file : %s, Error : %d", filename, err=errno);
   
