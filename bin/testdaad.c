#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>


void parent(int toChild[2], int toParent[2], int pid);
void child(int toChild[2], int toParent[2]);

char   *checkText;
size_t line = 1;
int    fdIn, fdOut;
char   output[3000], sentence[256];

int main(int argc, char **argv)
{
	int to_child[2];
	int fr_child[2];
	int pid;
	int ret;
	
	if (argc!=2) {
		printf("\nTestDaad v1.0 (C) NataliaPC\n\nSintaxis:\n\ttestdaad <testfile.test>\n\n");
		return 1;
	}

	// Read test file
	FILE   *fp = fopen(argv[1], "rb");
	size_t read, filelen;
	char   *p;
	if (!fp) {
		printf("Can't read file '%s'...\n", argv[1]);
		return 1;
	}
	fseek(fp, 0L, SEEK_END);
	filelen = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	checkText = p = malloc(filelen+1);
	while (filelen) {
		read = fread(p, 1, filelen, fp);
		p += read;
		filelen -= read;
	}
	fclose(fp);
	*p = '\0';

	//Substitute '\n' with '\0'
	p = checkText;
	while (*p) {
		if (*p=='\n') *p = '\0';
		p++;
	}

	//Create pipes
	if (pipe(to_child) != 0 || pipe(fr_child) != 0) {
		printf("Failed to open pipes\n");
		return 1;
	}

	assert(to_child[0] > STDERR_FILENO && to_child[1] > STDERR_FILENO &&
		   fr_child[0] > STDERR_FILENO && fr_child[1] > STDERR_FILENO);
		   
	if ((pid = fork()) < 0) {
		printf("Failed to fork\n");
		return 1;
	}

	if (pid == 0)
		child(to_child, fr_child);
	else
		parent(to_child, fr_child, pid);

	fprintf(stderr, "Process %d continues and exits\n", (int)getpid());
	return 1;
}

//============================================================ child

void child(int toChild[2], int toParent[2])
{
	fflush(0);
	if (dup2(toChild[0], STDIN_FILENO) < 0 || dup2(toParent[1], STDOUT_FILENO) < 0) {
		printf("Failed to set standard I/O in child\n");
		exit(1);
	}
	close(toChild[0]);
	close(toChild[1]);
	close(toParent[0]);
	close(toParent[1]);

	fprintf(stderr, "Executing OpenMSX...\n");
	char *args[] = { "openmsx", "-control", "stdio", 
								"-ext" , "debugdevice", 
								"-machine", "msx2plus",
								"-diska", "dsk", NULL };
	execvp(args[0], args);

	fprintf(stderr, "Failed to execute %s\n", args[0]);
	exit(1);
}

//============================================================ parent

void sendCommandRaw(const char *cmd)
{
	write(fdOut, cmd, strlen(cmd));
}

void sendCommand(const char *cmd)
{
	write(fdOut, "<command>", 9);
	write(fdOut, cmd, strlen(cmd));
	write(fdOut, "</command>\n", 11);
}

void waitResponse()
{
	int  size, pos=0, count=1000;
	char c = 0, *cnv;

	do {
		size = read(fdIn, &c, 1);
		if (size) {
			output[pos++] = c;
		}
	} while (count-- && c!='\n' && c!='.' && c!='>');
	output[pos] = '\0';
}


bool eofSentence(char *txt)
{
	return !*txt;
}

char* nextSentence(char *txt)
{
	if (!eofSentence(txt)) {
		while (*txt++);
	}
	line++;
	return txt;
}

char* copyText(char *dest, char *orig)
{
	char *dest1 = dest;
	while (*orig) {
		if (*orig=='"') *dest1++ = '\\';
		*dest1++ = *orig++;
	}
	*dest1 = '\0';
	return dest;
}

void parent(int toChild[2], int toParent[2], int pid)
{
	char *aux = (char*)malloc(256);

	printf("Parent\n");
	close(toChild[0]);
	close(toParent[1]);
	//toChild[1] to write
	fdOut = toChild[1];
	//toParent[0] to read
	fdIn = toParent[0];

	waitResponse();
	sendCommandRaw("<openmsx-control>\n");
	sendCommand("set renderer SDL");
	waitResponse();
	sendCommand("set power on");
	waitResponse();
	sendCommand("set debugoutput stdout");
	waitResponse();
	sendCommand("set speed 99999");
	waitResponse();

	int status;
	while (!eofSentence(checkText) && waitpid(pid, &status, WNOHANG) != pid) {
		waitResponse();

		if (*output) {
			while (*checkText=='#') {
				checkText = nextSentence(checkText);
			}
			if (*checkText=='@') {
				unsigned long wait = atol(checkText+1);
				printf("%lu: Waiting %lu msec\n", line, wait);
				usleep(wait * 1000L);
				checkText = nextSentence(checkText);
			}
			if (*checkText=='<') {
//printf("%s",output);		//For debugging only
				if (strstr(output, checkText+1)) {
					printf("%lu: #OK %s\n", line, checkText);
					checkText = nextSentence(checkText);
				}
			}
			if (*checkText=='>') {
				usleep(75000);
				sprintf(sentence, "type \"%s\r\"", copyText(aux, checkText+1));
				sendCommand(sentence);
				fprintf(stderr, "%lu: #OK %s\n", line, checkText);
				checkText = nextSentence(checkText);
			}
		}
	}

	usleep(500000);
	sendCommand("set speed 100");
	waitResponse();

	fprintf(stderr, "##################################\n"
					"######### TEST PASSED OK #########\n"
					"##################################\n");
	exit(0);
}
