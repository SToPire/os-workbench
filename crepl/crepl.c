#include <stdio.h>
#include <string.h>
#include<stdlib.h>

int main(int argc, char *argv[]) {
    char s1[128];
    strcpy(s1, getenv("PATH"));
    char s2[128] = "PATH=";
    strcat(s2, s1);

    static char line[4096];
    while (1) {
        printf("crepl> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        printf("%s", line);
        printf("Got %zu chars.\n", strlen(line));  // WTF?
  }
}
