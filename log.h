#ifndef LOG_H_
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>


// To initialize, put LOG_INIT under includes and put l_init() at the top of main


#define LOG_INIT FILE *LGFD = NULL; // put LOG_INIT at the beginning of the file

/*
* Looks for and creates a 'logs' directory where log files are placed.
* Logs are named in the format: "YYYY-MM-DD--HH:MM:SS.log"
* Required at the beginning of the program
*/
int l_init();

/*
* Close the log file.
* Call before end of program.
*/
int l_close();

/*
* Writes into the logfile and one of the STD ouutputs
* Flags determines which STD output the message is sent to in addition to the logfile
* Flags include:
*   - INFO
*   - ERR
*/
#define l_log(flag, msg) \
    __write_two(__FILE__, __LINE__, flag, msg)

/*
* Formatted logging.
* Otherwise same as log.
*/
#define l_logf(flag, msg, ...) \
    __write_two_f(__FILE__, __LINE__, flag, msg, __VA_ARGS__)


/*
* Writes only to file
*/
#define l_flog(flag, msg) \
    __write_file(__FILE__, __LINE__, flag, msg)

/*
* Formatted write to file
*/
#define l_flogf(flag, msg, ...) \
    __write_file_f(__FILE__, __LINE__, flag, msg, __VA_ARGS__)


/*
* Writes only to stdout
*/
#define l_olog(flag, msg) \
    __write_out(__FILE__, __LINE__, flag, msg)

/*
* Formatted write to stdout
*/
#define l_ologf(flag, msg, ...) \
    __write_out_f(__FILE__, __LINE__, flag, msg, __VA_ARGS__)


/*
* Writes only to stderr
*/
#define l_elog(flag, msg) \
    __write_err(__FILE__, __LINE__, flag, msg)

/*
* Formatted write to stderr
*/
#define l_elogf(flag, msg, ...) \
    __write_err_f(__FILE__, __LINE__, flag, msg, __VA_ARGS__)


static void __write_two(char *file, int line, int flag, char *msg);
static void __write_two_f(char *file, int line, int flag, const char *msg, ...);
static void __write_one(char *file, int line, FILE* dest, int flag, char *msg);
static void __write_one_f(char *file, int line, FILE* dest, int flag, const char *msg, va_list args);

static void __write_file(char *file, int line, int flag, char *msg);
static void __write_out(char *file, int line, int flag, char *msg);
static void __write_err(char *file, int line, int flag, char *msg);
static void __write_file_f(char *file, int line, int flag, const char *msg, ...);
static void __write_out_f(char *file, int line, int flag, const char *msg, ...);
static void __write_err_f(char *file, int line, int flag, const char *msg, ...);
inline char* getTime();

typedef enum {
    L_INFO,
    L_ERR,
} Flag;

extern FILE *LGFD; // log file descriptor

inline int l_init() {
    // check/create /logs directory
    int mk = mkdir("./logs", 0777);

    // get and format current date-time
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char s[64];
    size_t ret = strftime(s, sizeof(s), "%F--%T", tm);

    // format log filename with date-time
    char cwd[2048] = {0};
    char *directory = getcwd(cwd, 2048);
    char path[3072] = {0};
    if (directory == NULL) {
        // back up in case getcwd fails
        strcpy(path, "./logs/");
    } else {
        strcpy(path, directory);
        strcat(path, "/logs/");
    }
    char fname[3072];
    char title[3072];
    strcpy(fname, path);
    strcat(fname, s);
    strcpy(title, fname);
    strcat(fname, ".log");

    // create log file
    FILE *fd = fopen(fname, "w+");
    if (fd == NULL) {
        perror("error finding/creating file\n");
        return -1;
    }
    fprintf(fd, "------------------ %s ------------------\n\n", title);
    fflush(fd);

    LGFD = fd;
    return 0;
}

char* getTime() {
    // get and format current time
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char *time = (char*)malloc(64 * sizeof(char));
    // remember to free time after use
    size_t ret = strftime(time, sizeof(time), "%T", tm);

    return time;
}

inline void __write_two(char *file, int line, int flag, char *msg) {

    FILE *fd = LGFD;

    char *time = getTime();

    if (flag == L_INFO) {
        // send to stdout
        fprintf(fd, "[INFO]\t %s:%i %s --> %s\n", file, line, time, msg);
        fprintf(stdout, "\033[033m[INFO]\033[0m\t %s:%i %s --> %s\n", file, line, time, msg);
    } else if (flag == L_ERR) {
        // send to stderr
        fprintf(fd, "[ERROR]\t %s:%i %s --> %s\n", file, line, time, msg);
        fprintf(stderr, "\033[031m[ERROR]\033[0m\t %s:%i %s --> %s\n", file, line, time, msg);
    }
    fflush(fd);
    free(time);
    return;
}

inline void __write_two_f(char *file, int line, int flag, const char *msg, ...) {

    FILE *fd = LGFD;
    char *time = getTime();

    va_list args;
    va_start(args, msg);
    if (flag == L_INFO) {
        // send to file
        fprintf(fd, "[INFO]\t %s:%i %s --> ", file, line, time);
        vfprintf(fd, msg, args);
        fprintf(fd, "\n");
        // send to stdout
        fprintf(stdout, "\033[033m[INFO]\033[0m\t %s:%i %s --> ", file, line, time);
        vfprintf(stdout, msg, args);
        fprintf(stdout, "\n");
    } else if (flag == L_ERR) {
        // send to file
        fprintf(fd, "[ERROR]\t %s:%i %s --> ", file, line, time);
        vfprintf(fd, msg, args);
        fprintf(fd, "\n");
        // send to stderr
        fprintf(stderr, "\033[031m[ERROR]\033[0m\t %s:%i %s --> ", file, line, time);
        vfprintf(stderr, msg, args);
        fprintf(stderr, "\n");

    }

    va_end(args);
    fflush(fd);
    free(time);
    return;
}

static void __write_one(char *file, int line, FILE* dest, int flag, char *msg) {

    char *time = getTime();

    if (flag == L_INFO) {
        if (dest == stdout) {
            // stdout
            fprintf(stdout, "\033[033m[INFO]\033[0m\t %s:%i %s --> %s\n", file, line, time, msg);
        } else if (dest == stderr) {
            // stderr
            fprintf(stderr, "\033[033m[INFO]\033[0m\t %s:%i %s --> %s\n", file, line, time, msg);
        } else {
            // file
            fprintf(LGFD, "[INFO]\t %s:%i %s --> %s\n", file, line, time, msg);
        }
    } else if (flag == L_ERR) {
        // send to stderr
        if (dest == stdout) {
            // stdout
            fprintf(stdout, "\033[031m[ERROR]\033[0m\t %s:%i %s --> %s\n", file, line, time, msg);
        } else if (dest == stderr) {
            // stderr
            fprintf(stderr, "\033[031m[ERROR]\033[0m\t %s:%i %s --> %s\n", file, line, time, msg);
        } else {
            // file
            fprintf(LGFD, "[ERROR]\t %s:%i %s --> %s\n", file, line, time, msg);
        }
    }
    fflush(dest);
    free(time);
    return;
}

inline void __write_one_f(char *file, int line, FILE* dest, int flag, const char *msg, va_list args) {

    char *time = getTime();

    if (flag == L_INFO) {
        // send to stdout
        if (dest == stdout) {
            // stdout
            fprintf(stdout, "\033[033m[INFO]\033[0m\t %s:%i %s --> ", file, line, time);
            vfprintf(stdout, msg, args);
            fprintf(stdout, "\n");
        } else if (dest == stderr) {
            // stderr
            fprintf(stderr, "\033[033m[INFO]\033[0m\t %s:%i %s --> ", file, line, time);
            vfprintf(stderr, msg, args);
            fprintf(stderr, "\n");
        } else {
            // file
            fprintf(LGFD, "[INFO]\t %s:%i %s --> ", file, line, time);
            vfprintf(LGFD, msg, args);
            fprintf(LGFD, "\n");
        }
    } else if (flag == L_ERR) {
        // send to stderr
       if (dest == stdout) {
            // stdout
            fprintf(stdout, "\033[031m[ERROR]\033[0m\t %s:%i %s --> ", file, line, time);
            vfprintf(stdout, msg, args);
            fprintf(stdout, "\n");
        } else if (dest == stderr) {
            // stderr
            fprintf(stderr, "\033[031m[ERROR]\033[0m\t %s:%i %s --> ", file, line, time);
            vfprintf(stderr, msg, args);
            fprintf(stderr, "\n");
        } else {
            // file
            fprintf(LGFD, "[ERROR]\t %s:%i %s --> ", file, line, time);
            vfprintf(LGFD, msg, args);
            fprintf(LGFD, "\n");
        }
    }
    fflush(dest);
    free(time);
    return;
}

static void __write_file(char *file, int line, int flag, char *msg) {
    __write_one(file, line, LGFD, flag, msg);
    return;
}

static void __write_out(char *file, int line, int flag, char *msg) {
    __write_one(file, line, stdout, flag, msg);
    return;
}

static void __write_err(char *file, int line, int flag, char *msg) {
    __write_one(file, line, stderr, flag, msg);
    return;
}

static void __write_file_f(char *file, int line, int flag, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    __write_one_f(file, line, LGFD, flag, msg, args);
    va_end(args);
    return;
}

static void __write_out_f(char *file, int line, int flag, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    __write_one_f(file, line, stdout, flag, msg, args);
    va_end(args);
    return;
}

static void __write_err_f(char *file, int line, int flag, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    __write_one_f(file, line, stderr, flag, msg, args);
    va_end(args);
    return; 
}

inline int l_close() {
    if (fclose(LGFD) == EOF) {
        perror("Error closing file");
        return -1;
    }
    return 0;
}

#endif // LOG_H_
