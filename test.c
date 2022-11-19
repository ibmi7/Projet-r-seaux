#include <stdio.h>
#include <string.h>

int main() {
	char buffer[20] = "Bonjour Homme";
	char test[10];
	printf("%s \n", strtok(buffer, " "));
	printf("%s \n", strtok(NULL, " "));
	printf("%s", strtok(NULL, " "));
}