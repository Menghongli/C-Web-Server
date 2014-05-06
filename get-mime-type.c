#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#define MIMETYPE "mime-types.tsv"
char * get_mime_type(char *name) {
	char *ext = strrchr(name, '.');
  char delimiters[] = " ";
	char *mime_type = NULL;
	mime_type = malloc (128 * sizeof(char)) ;
	char line[128];
	char *token;
	int line_counter = 1;
	ext++; // skip the '.';
	FILE *mime_type_file = fopen(MIMETYPE, "r");
	if (mime_type_file != NULL) {
		while(fgets(line, sizeof line, mime_type_file) != NULL) {
			if (line_counter > 1)
			{
				if((token = strtok(line,delimiters)) != NULL) {
					if(strcmp(token,ext) == 0) {
						token = strtok(NULL, delimiters);
						strcpy(mime_type, token);
						break;
					}
				}
			}
			line_counter++;
		}
		fclose( mime_type_file );
	} else {
		perror("open");
	}
	return mime_type;
}

int main(int argc, char **argv)
{
	char *content_name = NULL ;
	char *name = "hello.html";
	content_name = get_mime_type(name);
	printf("%s", content_name);	
	return 0;
}

