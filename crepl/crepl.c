#include <stdio.h>
#include <string.h>
#include<stdlib.h>

int main(int argc, char *argv[]) {
    char s1[128];
    sprintf(s1, "PATH=%s", getenv("PATH"));
    puts(s1);

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
