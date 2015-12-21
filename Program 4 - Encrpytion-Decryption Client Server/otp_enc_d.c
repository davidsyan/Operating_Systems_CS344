/* otp_enc_d program */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

int inputValidation(char character); 
int encrypt(char character);

int main(int argc, char *argv[]) {
  //check to see number of args is correct
  if (argc < 2) { 
    fprintf(stderr, "otp_enc_d usage:\n { otp_enc_d listening port }\n");
    exit(1);
  }

   //fork to create child process
  pid_t pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }
  umask(0); 

  //make session ID for child process
  pid_t sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  close(STDOUT_FILENO);
  close(STDIN_FILENO);

  //open socket on given port
  int port_num = atoi(argv[1]);
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  //unable to open socket
  if (sock_fd < 0) {
    fprintf(stderr, "Error: Unable to open socket.\n");
    exit(1);
  }

  struct sockaddr_in server_add;
  struct sockaddr_in client_add;
  bzero((char*)&server_add, sizeof(server_add));
  server_add.sin_family = AF_INET;
  server_add.sin_port = htons(port_num);
  server_add.sin_addr.s_addr = htonl(INADDR_ANY);

  //bind server to socket port
  if (bind(sock_fd, (struct sockaddr*)&server_add, sizeof(server_add)) < 0) {
    fprintf(stderr, "Error: Unable to bind server to socket.\n");
    exit(1);
  }

  //connection listener
  //allows five concurrent connections
  listen(sock_fd, 5); 
  socklen_t client_add_length = sizeof(client_add);
  //test connection to socket port
  int is_sock_fd_connected;
  //continue to listen for connections
  while (1) {
    is_sock_fd_connected = accept(sock_fd, (struct sockaddr*)&client_add, &client_add_length);
    //unable to connect
    if (is_sock_fd_connected < 0) {
      fprintf(stderr, "Error: Unable to accept.\n");
      exit(1);
    }

    //test signal is correct
    int size = 1024;
    char send[size];
    bzero(send, size);
    char receive[size];
    bzero(receive, size);
    int sig = read(is_sock_fd_connected, receive, size);
    //unable to read
    if (sig < 0) {
      fprintf(stderr, "Error: Unable to read from socket.\n");
      exit(1);
    }
    printf("%c",receive[0]);
    if (receive[0] != '^') {
      snprintf(send, size, "%s", "error_3");
      sig = write(is_sock_fd_connected, send, strlen(send)); 
      //unable to read
    	if (sig < 0) {
        fprintf(stderr, "Error: Unable to read from socket.\n");
    	  exit(1);
    	}
      continue; // Keep waiting!
    }

    ////read plaintext
    bzero(receive, size);
    sig = read(is_sock_fd_connected, receive, size);
    //unable to read
    if (sig < 0) {
      fprintf(stderr, "Error: Unable to read from socket.\n");
      exit(1);
    }

    //oppen plaintext file
    FILE *f;
    f = fopen(receive, "r");
    //unable to read
    if (f == NULL) {
      fprintf(stderr, "Error: Unable to open plaintext.\n");
      exit(1);
    }

    //change all letters to uppercase
    char plaintext[size];
    fgets(plaintext, size, f);
    int i;
    for (i=0; i<strlen(plaintext); i++) {
      plaintext[i] = toupper(plaintext[i]);
    }
    plaintext[(strlen(plaintext))+1] = '\0';

    //check to see if all characters are valid
    bzero(send, size);
    for (i=0; i<(strlen(plaintext)-1); i++) {
      if (inputValidation(plaintext[i]) == 1) {
        snprintf(send, size, "%s", "error_2");
        sig = write(is_sock_fd_connected, send, strlen(send));
        //unable to write to socket
	      if (sig < 0) {
          fprintf(stderr, "Error: Unable to write to socket.\n");
	        exit(1);
	      }
	      break;
      }
      continue;
    }
    fclose(f);

    //receive key from socket port
    bzero(receive, size);
    sig = read(is_sock_fd_connected, receive, size);
    //unable to read
    if (sig < 0) {
      fprintf(stderr, "Error: Unable to read from socket.\n");
      exit(1);
    }

    //open key
    f = fopen(receive, "r");
    //unable to open
    if (f == NULL) {
      fprintf(stderr, "Error: Unable to open key.\n");
      exit(1);
    }

    //store key for later use
    char key[size];
    fgets(key, size, f);
    fclose(f);

    //verify key is at least as long as text
    if (strlen(key) < strlen(plaintext)) {
      bzero(send, size);
      snprintf(send, size, "%s", "error_1");
      sig = write(is_sock_fd_connected, send, strlen(send));
      //unable to write
      if (sig < 0) {
        fprintf(stderr, "Error: Unable to write to socket.\n");
        exit(1);
      }
      continue; 
    }

    //encrypt message
    char characters[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    char ciphertext[strlen(plaintext)];
    int key_num[strlen(plaintext)];
    int plaintext_num[strlen(plaintext)];
    int cipher_num[strlen(plaintext)];

    //convert character to number
    for (i=0; i<strlen(plaintext); i++) {
      key_num[i] = encrypt(key[i]);
      plaintext_num[i] = encrypt(plaintext[i]);
      //add key number to char number
      cipher_num[i] = plaintext_num[i] + key_num[i]; 
      //use mod conversion
      if (cipher_num[i] > 26) {
        cipher_num[i] -= 27;
      }
      //add each letter to the array for later use
      ciphertext[i] = characters[cipher_num[i]]; 
    }
    ciphertext[strlen(plaintext)-1] = '\0';

    //send encrypted message to socket port
    bzero(send, size);
    snprintf(send, size, "%s", ciphertext);
    //unable to write
    sig = write(is_sock_fd_connected, send, strlen(send));
    if (sig < 0) {
      fprintf(stderr, "Error: Unable to write to socket.\n");
      exit(1);
    }
  }

  return 0;
}

int inputValidation(char character) {
  switch (character) {
    case 'A':
      return 0;
    case 'B':
      return 0;
    case 'C':
      return 0;
    case 'D':
      return 0;
    case 'E':
      return 0;
    case 'F':
      return 0;
    case 'G':
      return 0;
    case 'H':
      return 0;
    case 'I':
      return 0;
    case 'J':
      return 0;
    case 'K':
      return 0;
    case 'L':
      return 0;
    case 'M':
      return 0;
    case 'N':
      return 0;
    case 'O':
      return 0;
    case 'P':
      return 0;
    case 'Q':
      return 0;
    case 'R':
      return 0;
    case 'S':
      return 0;
    case 'T':
      return 0;
    case 'U':
      return 0;
    case 'V':
      return 0;
    case 'W':
      return 0;
    case 'X':
      return 0;
    case 'Y':
      return 0;
    case 'Z':
      return 0;
    case ' ':
      return 0;
    default:
      return 1;
  }
}

int encrypt(char character) {
  switch (character) {
    case 'A':
      return 0;
    case 'B':
      return 1;
    case 'C':
      return 2;
    case 'D':
      return 3;
    case 'E':
      return 4;
    case 'F':
      return 5;
    case 'G':
      return 6;
    case 'H':
      return 7;
    case 'I':
      return 8;
    case 'J':
      return 9;
    case 'K':
      return 10;
    case 'L':
      return 11;
    case 'M':
      return 12;
    case 'N':
      return 13;
    case 'O':
      return 14;
    case 'P':
      return 15;
    case 'Q':
      return 16;
    case 'R':
      return 17;
    case 'S':
      return 18;
    case 'T':
      return 19;
    case 'U':
      return 20;
    case 'V':
      return 21;
    case 'W':
      return 22;
    case 'X':
      return 23;
    case 'Y':
      return 24;
    case 'Z':
      return 25;
    case ' ':
      return 26;
  }
}