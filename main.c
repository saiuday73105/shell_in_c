#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*Function Declarations for builtin shell commands:*/
int ush_cd(char **args);
int ush_help(char **args);
int ush_exit(char **args);

/*List of builtin commands, followed by their corresponding functions.*/
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &ush_cd,
  &ush_help,
  &ush_exit
};

int ush_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*Builtin function implementations.*/
int ush_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "ush: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("ush");
    }
  }
  return 1;
}

int ush_help(char **args)
{
  int i;
  printf("Uday's Shell\n");
  printf("The following are built in:\n");

  for (i = 0; i < ush_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int ush_exit(char **args)
{
  return 0;
}

int ush_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("ush");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("ush");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int ush_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < ush_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return ush_launch(args);
}

#define USH_RL_BUFSIZE 1024
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *ush_read_line(void)
{
  int bufsize = USH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "ush: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += USH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "ush: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define USH_TOK_BUFSIZE 64
#define USH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **ush_split_line(char *line)
{
  int bufsize = USH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "ush: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, USH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += USH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "ush: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, USH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void ush_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("--> ");
    line = ush_read_line();
    args = ush_split_line(line);
    status = ush_execute(args);

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  ush_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
