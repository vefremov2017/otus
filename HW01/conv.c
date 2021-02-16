/*
 *  conv - simple utility for convert text from {koi8-r|cp-1251|iso8859-5} codepages to utf-8
 *  usage description see in printUsage() function ;-)
 *
 *  Version: 1.0
 *  16/02/2021
 *  Efremov V.
 *
 * Compiled by
 * gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-36)
 *
 * gcc -Wall -Wextra -Wpedantic -std=c11 conv.c -o conv
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>

#define RUN_WITH_HELP_OPTION    "run with -h option for detailed information of usage\n"
#define ERROR_CODE_PAGE         "Error: unknown code page"
#define ERROR_NOT_ENOUGH_PARAMS "Error: not enough parameters"
#define ERROR_OPEN_FILE         "Error: cannot open file"

#define KOI "koi8-r"
#define WIN "cp-1251"
#define ISO "iso8859-5"

typedef struct runArgs_t {
        char *codePage;
  const char *inFileName;
  const char *outFileName;
        FILE *inFile;
        FILE *outFile;
} runArgs;

static const char *optString = "c:i:o:h";

static int koi8map[255] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  1025, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1105, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 1102, 1072, 1073, 1094, 1076, 1077, 1092, 1075, 1093, 1080, 1081, 1082, 
  1083, 1084, 1085, 1086, 1087, 1103, 1088, 1089, 1090, 1091, 1078, 1074, 1100, 1099, 
  1079, 1096, 1101, 1097, 1095, 1098, 1070, 1040, 1041, 1062, 1044, 1045, 1060, 1043, 
  1061, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055, 1071, 1056, 1057, 1058, 1059, 
  1046, 1042, 1068, 1067, 1047, 1064, 1069, 1065, 1063, 1066
};

void  parseArgs(int argc, char *argv[], runArgs *args);
void  openFiles (runArgs *args);
void  closeFiles(runArgs *args);
void  convertIt (runArgs *args);
void  exitWithMessage(int status, char *msg);
char* toLower(char *str);
void  printUsage();
void  printRunArgs(runArgs *args);

int main(int argc, char *argv[]) {
  runArgs args = {NULL, NULL, NULL, NULL, NULL};
    
  parseArgs(argc, argv, &args);
  openFiles(&args);
//printRunArgs(&args);
  convertIt(&args);
  closeFiles(&args);

  return 0;
}

void parseArgs(int argc, char *argv[], runArgs *args) {
  char msg[1024] = {'\0'};

  // run without any parameters
  if (argc == 1) 
    exitWithMessage(EXIT_SUCCESS, RUN_WITH_HELP_OPTION);
 
  // get first argument 
  int opt = getopt(argc, argv, optString);

  // iterate and parse arguments
  while (opt != -1) {
    switch(opt) {
      // code page
      case 'c':
        if (strcmp(toLower(optarg), KOI) && strcmp(toLower(optarg), WIN) && strcmp(toLower(optarg), ISO)) {
          sprintf(msg, "%s - %s\n%s", ERROR_CODE_PAGE, optarg, RUN_WITH_HELP_OPTION);          
          exitWithMessage(EXIT_FAILURE, msg);
        }
        args->codePage = !(strcmp(toLower(optarg), KOI)) ? KOI : (!(strcmp(toLower(optarg), WIN)) ? WIN : ISO);
        break;

      // input file
      case 'i':
        args->inFileName = optarg;
        break;

      // output file
      case 'o':
        args->outFileName = optarg;
        break;

      // help
      case 'h':
        printUsage();
        exit(EXIT_SUCCESS);

      default:
        exitWithMessage(EXIT_FAILURE, RUN_WITH_HELP_OPTION);
    }
    // next argument
    opt = getopt(argc, argv, optString);
  }

  // check total parse arguments
  if (args->codePage == NULL || args->inFileName == NULL || args->outFileName == NULL) {
    sprintf(msg, "%s\n%s", ERROR_NOT_ENOUGH_PARAMS, RUN_WITH_HELP_OPTION);          
    exitWithMessage(EXIT_FAILURE, msg);
  }
}

void openFiles(runArgs *args) {
  char msg[1024] = {'\0'};
  char *erroredFileName = (char *)malloc((strlen(args->inFileName) > strlen(args->outFileName)) ? strlen(args->inFileName) : strlen(args->outFileName)*sizeof(char));

  args->inFile = fopen(args->inFileName, "r");

  if (args->inFile == NULL) {
    strcpy(erroredFileName, args->inFileName);
  }
  else {
    args->outFile = fopen(args->outFileName, "w");
    if (args->outFile == NULL) {
      strcpy(erroredFileName, args->outFileName);
      fclose(args->inFile);
    }
    else
      return;
  }

  sprintf(msg, "%s - %s\n", ERROR_OPEN_FILE, erroredFileName);
  exitWithMessage(EXIT_FAILURE, msg);
}

void convertIt (runArgs *args) {
  int ch;
  wchar_t w_ch;

  setlocale(LC_ALL, "ru_RU.utf8");

  while((ch = fgetc(args->inFile)) != EOF) {
    w_ch = ch;
    switch (*args->codePage) {
      // koi
      case 'k':
         if ((ch >= 192 && ch <= 255) || ch == 163 || ch == 179)
          w_ch = koi8map[ch-1];
        break;
        
      // iso
      case 'i':
        if (ch >= 176 && ch <= 239)
          w_ch = 864+ch;
        else if (ch == 161)
          w_ch = 1025;
        else if (ch == 241)
          w_ch = 1105;
        break;
      
      // cp-1251
      case 'c':
        if (ch >= 192 && ch <= 255)
          w_ch = 848+ch;
        else if (ch == 168)
          w_ch = 1025;
        else if (ch == 184)
          w_ch = 1105;
        break;
    } 
    
    fputwc(w_ch, args->outFile);
  }
}

void closeFiles(runArgs *args) {
  fclose(args->inFile);
  fclose(args->outFile);
}

void  exitWithMessage(int status, char *msg) {
  if (msg) printf("%s\n", msg);

  if (!status) exit(EXIT_FAILURE);
  
  exit(EXIT_SUCCESS);
}

void printRunArgs(runArgs *args) {
  printf("codePage     : %s\n" , args->codePage);
  printf("inFileName   : %s\n" , args->inFileName);
  printf("outFileName  : %s\n" , args->outFileName);
  printf("inFile       : %p\n",  (void *)args->inFile);
  printf("outFile      : %p\n",  (void *)args->outFile);
}

char* toLower(char *str) {
  for (char *p=str; *p; p++) *p=tolower(*p);
  return str;
}

void printUsage() {
  printf("conv utility - convert text from {koi8-r|cp-1251|iso8859-5} codepage to utf-8\n\n");

  printf("required options:\n");
  printf("\t-c {koi8-r|cp-1251|iso8859-5} - codepage of input file\n"); 
  printf("\t-i input file\n"); 
  printf("\t-o output file\n\n"); 

  printf("Examlpe of usage:\n"); 
  printf("./conv -c koi8-r -i koi8.txt -o koi8.out.txt\n");
  printf("./conv -c cp-1251 -i cp1251.txt -o cp1251.out.txt\n");
  printf("./conv -c iso8859-5 -i iso-8859-5.txt -o iso-8859-5.out.txt\n\n");
}
