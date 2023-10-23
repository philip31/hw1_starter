#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);
  
  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");

  return 0;
}


/* beargit add <filename>
 * 
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}


/* beargit rm <filename>
 * 
 * See "Step 2" in the homework 1 spec.
 *
 */

int beargit_rm(const char* filename) {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  char exists = 0;
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0)
      exists = 1;
    else
      fprintf(fnewindex, "%s\n", line);
  }

  if (exists == 0)
  {
    fprintf(stderr, "ERROR: File %s not tracked\n", filename);
    fclose(findex);
    fclose(fnewindex);
    fs_rm(".beargit/.newindex");
    return 1;
  }

  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");
  return 0;
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char* go_bears = "GO BEARS!";
const size_t go_bears_length = 9;
int is_commit_msg_ok(const char* msg) {
  /* COMPLETE THE REST */

  char ok = 0;
  size_t i = 0;
  size_t j = 0;
  while (ok != 1 && msg[i] != '\0')
  {
    if(msg[i] == go_bears[j])
      j++;
    else if (j == go_bears_length)
      ok = 1;
    else
      j = 0;
    i++;
  }

  return ok;
}

char generate_char(char a, char b)
{
  uint16_t c = a + b;

  unsigned bit = 0;
  bit = ((c & 4) >> 2) ^ (c & 1);
  bit = ((c & 8) >> 3) ^ bit;
  bit = ((c & 32) >> 5) ^ bit;
  bit <<= 15;
  c >>= 1;
  c |= bit;

  char remainder = c%3;

  switch (remainder)
  {
    case 0: return 'c';
    case 1: return '1';
    case 2: return '6';
  }
}

void make_space(char* str, size_t length)
{
  for (size_t i = length; i > 0; i--)
    str[i] = str[i-1];    
}

int is_first_commit(char *commit_id)
{
  char first_id[COMMIT_ID_SIZE];
  for (size_t i = 0; i< COMMIT_ID_SIZE; i++)
    first_id[i] = '0';
  first_id[COMMIT_ID_SIZE-1] = '\0';
  
  return strcmp(first_id, commit_id) == 0;
}

void next_commit_id(char* commit_id) {
  /* COMPLETE THE REST */

  
  if(is_first_commit(commit_id))
  {
    for(size_t i = 0 ; i < COMMIT_ID_SIZE; i++)
      commit_id[i] = '1';
    commit_id[COMMIT_ID_SIZE] = '\0';

    return;
  }

  char *new_id = commit_id;
  char character = '\0';

  character = generate_char(commit_id[COMMIT_ID_SIZE-3],commit_id[COMMIT_ID_SIZE-1]);
  character = generate_char(character, commit_id[COMMIT_ID_SIZE-4]);
  character = generate_char(character, commit_id[COMMIT_ID_SIZE-6]);

  make_space(commit_id, COMMIT_ID_SIZE);
  commit_id[0] = character;
  commit_id[COMMIT_ID_SIZE] = '\0';

}

int beargit_commit(const char* msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);
  /* COMPLETE THE REST */

  char folder_name[COMMIT_ID_SIZE + 10];
  strcpy(folder_name, ".beargit/");
  strcat(folder_name, commit_id);
  strcat(folder_name, "/");
  fs_mkdir(folder_name);

  char file_name [strlen(folder_name) + 50];
  strcpy(file_name, folder_name);
  strcat(file_name, ".index");
  fs_cp(".beargit/.index", file_name);

  strcpy(file_name, folder_name);
  strcat(file_name, ".prev");
  fs_cp(".beargit/.prev", file_name);
  
  char message_file [strlen(folder_name) + 30];
  strcpy(message_file, folder_name);
  strcat(message_file, ".msg");

  
  write_string_to_file(message_file, msg);


  FILE* tracked_files = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
   while(fgets(line, sizeof(line), tracked_files)) {
    strtok(line, "\n");
    strcpy(file_name, folder_name);
    strcat(file_name, line);
    fs_cp(line, file_name);
  }
  fclose(tracked_files);


  write_string_to_file(".beargit/.prev", commit_id);
  return 0;
}

/* beargit status
 *
 * See "Step 1" in the homework 1 spec.
 *
 */

int beargit_status() {
  /* COMPLETE THE REST */
  printf("%s\n", "Tracked files:");
  FILE* findex = fopen(".beargit/.index", "r");

  unsigned int nr_files = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    
    strtok(line, "\n");
    printf("\n\t%s", line);
    nr_files++;
  }

  printf("\n\n%d %s\n", nr_files, "files total");
  fclose(findex);

  return 0;
}

/* beargit log
 *
 * See "Step 4" in the homework 1 spec.
 *
 */
#define MAX_MSG 100
int beargit_log() {
  /* COMPLETE THE REST */
  char *last_id[COMMIT_ID_SIZE];
  
  read_string_from_file(".beargit/.prev" ,last_id, COMMIT_ID_SIZE);

  if (is_first_commit(last_id))
  {
    printf("ERROR: There are no commits!\n");
    return 9;
  }  
  
  char *prev_file[100];
  char *msg_file[100];

  while (!is_first_commit(last_id))
  {
    strcpy(msg_file, ".beargit/");
    strcat(msg_file, last_id);
    strcat(msg_file, "/.msg");
    
    char *message[MAX_MSG];
    read_string_from_file(msg_file, message, MAX_MSG);

    printf("\ncommit %s\n", last_id);
    printf("\t%s\n", message);
    
    strcpy(prev_file, ".beargit/");
    strcat(prev_file, last_id);
    strcat(prev_file, "/.prev");
    read_string_from_file(prev_file, last_id, COMMIT_ID_SIZE);
  }

  printf("\n");
  return 0;
}
