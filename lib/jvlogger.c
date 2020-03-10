#include <stdio.h> 
#include <stdlib.h> 

//const char* read_path = "/home/josav09/Documents/C tests/fileReader/readme.txt";
//const char* write_path = "/home/josav09/Documents/C tests/fileReader/cpu.log";

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

void write_log (char* data, char* write_path) {
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
