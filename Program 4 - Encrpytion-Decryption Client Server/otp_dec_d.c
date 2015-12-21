/* otp_dec_d program */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

int inputValidation(char character); 
int decrypt(char character); 

int main(int argc, char *argv[]) {
  //check to see number of args is correct
  if (argc < 2) { 
    fprintf(stderr, "otp_dec_d usage:\n { otp_dec_d listening port }\n");
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
    if (receive[0] != '*') {
      snprintf(send, size, "%s", "error_3");
      sig = write(is_sock_fd_connected, send, strlen(send)); 
      //unable to read
      if (sig < 0) {
        fprintf(stderr, "Error: Unable to read from socket.\n");
        exit(1);
      }
      continue; 
    }

    //read ciphertext
    bzero(receive, size);
    sig = read(is_sock_fd_connected, receive, size);
    //unable to read
    if (sig < 0) {
      fprintf(stderr, "Error: Unable to read from socket.\n");
      exit(1);
    }

    //open ciphertext
    FILE *f;
    f = fopen(receive, "r");
    //unable to open
    if (f == NULL) {
      fprintf(stderr, "Error: Unable to open ciphertext.\n");
      exit(1);
    }

    //change all letters to uppercase
    char cipher[size];
    fgets(cipher, size, f);
    int i;
    for (i=0; i<strlen(cipher); i++) {
      cipher[i] = toupper(cipher[i]);
    }
    cipher[(strlen(cipher))+1] = '\0';

    
    for (i=0; i<(strlen(cipher)-1); i++) {
      //file contians bad characters
      if (inputValidation(cipher[i]) == 1) {
        fprintf(stderr, "otp_dec_d error: input contains bad characters\n");
        exit(1);
      }
    }
    fclose(f);

    //read key from socket
    bzero(receive, size);
    sig = read(is_sock_fd_connected, receive, size);
    //unable to read
    if (sig < 0) {
      fprintf(stderr, "Error: Unable to read from socket.\n");
      exit(1);
    }

    //open key file
    f = fopen(receive, "r");
    //unable to open file
    if (f == NULL) {
      fprintf(stderr, "Error: Unable to open key.\n");
      exit(1);
    }

    //store key for later use
    char key[size];
    fgets(key, size, f);
    fclose(f);

    //verify key is at least as long as text
    if (strlen(key) < strlen(cipher)) {
      fprintf(stderr, "Error: key '%s' is too short.\n", receive);
      exit(1);
    }

    //decrypt message
    char characters[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    char plaintext[strlen(cipher)];
    int key_num[strlen(cipher)];
    int plaintext_num[strlen(cipher)];
    int cipher_num[strlen(cipher)];

    //convert character to number
    for (i=0; i<strlen(cipher); i++) {
      key_num[i] = decrypt(key[i]);
      cipher_num[i] = decrypt(cipher[i]);
      //subtract key number from char number
      plaintext_num[i] = cipher_num[i] - key_num[i]; 
      //use mod conversion
      if (plaintext_num[i] < 0) { 
        plaintext_num[i] += 27;
      }
      //add each letter to the array for later use
      plaintext[i] = characters[plaintext_num[i]]; 
    }
    plaintext[strlen(cipher)-1] = '\0';

    //send decrypted message to socket port
    bzero(send, size);
    snprintf(send, size, "%s", plaintext);
    sig = write(is_sock_fd_connected, send, strlen(send));
    //unable to write
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

int decrypt(char character) {
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