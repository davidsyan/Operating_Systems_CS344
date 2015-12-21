/* keygen program */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
  //check to see number of args is correct
  if (argc != 2) { 
    printf("keygen usage:\n { keygen keylength }\n");
    exit(1);
  }
  else {
    srand(time(NULL));
    int length = atoi(argv[1]);
    char key[length];

    //generate random letters for length of key
    int i;
    char curr;
    for (i=0; i<length; i++) {
      curr = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "[random()%27];
      key[i] = curr;
    }
    key[length] = '\0';

    //print key
    printf("%s\n", key); 
  }

  return 0;
}