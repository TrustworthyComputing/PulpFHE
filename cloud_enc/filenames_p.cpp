#include "filenames_p.h"

int exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

char* gen_filename()
{
  srand(time(0));
  int isTaken = 1;
  char* fileName = (char*) malloc(50);
  while (isTaken != 0) {
      strcpy(fileName, "");
      strcpy(fileName, "ctxt");
      int num = rand() % 10000;
      char strNum[sizeof(num)];
      sprintf(strNum, "%d", num);
      strcat(fileName, strNum);
      strcat(fileName, ".data");
      isTaken = exists(fileName);
  }
  return fileName;
}
