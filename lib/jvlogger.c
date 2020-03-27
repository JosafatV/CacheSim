#include <stdio.h> 
#include <stdlib.h>
#include <time.h>

//const char* read_path = "/home/josav09/Documents/C tests/fileReader/readme.txt";
const char* write_path = "./logs/log.txt";

char read_char (char* read_path) {
	FILE *fp;

   fp = fopen(read_path, "r"); // read mode

   if (fp == NULL)
   {
      perror("Error while opening the file.\n");
      exit(EXIT_FAILURE);
   }

   printf("The contents of the file are:\n");

	char ch;
	ch = fgetc(fp);
	printf("%c", ch);
	/*
    while((ch = fgetc(fp)) != EOF) {
		printf("%c", ch);
	}*/

   fclose(fp);
   
   return ch;
}

void start_logg () {
	FILE *fp;
	fp = fopen(write_path, "w");

	if (fp == NULL) {
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	// give the log a time and date
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	fprintf(fp, "%s", "Logg started: ");
	fprintf(fp, "%s", asctime(timeinfo));

	fclose(fp);
}

void logg (char* data) {
	FILE *fp;
	char* newline = "\n";
	fp = fopen(write_path, "a"); // append mode

	if (fp == NULL) {
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
    }
	fprintf(fp, "%s", data);
	fprintf(fp, "%s", newline);

	fclose(fp);
}
