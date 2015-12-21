/* otp_enc program */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
  //check to see number of args is correct
  if (argc < 4) { 
    fprintf(stderr, "otp_enc usage:\n { otp_enc plaintext key port }\n");
    exit(1);
  }

  //open socket with given port from user
  int portNumber = atoi(argv[3]);
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  //unable to open
  if (sock_fd < 0) {
    fprintf(stderr, "Error: Unable to open socket.\n");
    exit(1);
  }

  struct sockaddr_in server_add;
  bzero((char*)&server_add, sizeof(server_add));
  server_add.sin_family = AF_INET;
  server_add.sin_port = htons(portNumber);
  server_add.sin_addr.s_addr = htonl(INADDR_ANY);

  //connect to socket port
  //unable to connect
  if (connect(sock_fd, (struct sockaddr*)&server_add, sizeof(server_add)) < 0) {
    fprintf(stderr, "Error: could not contact otp_enc_d on port %d\n", portNumber);
    exit(2);
  }

  //write ciphertext identification to socket
  int size = 1024;
  char buff[size];
  char sentinel[size];
  sentinel[1] = '^';
  snprintf(buff, size, "%c", sentinel[1]);
  int sig = write(sock_fd, buff, strlen(buff));
  //unable to write ciphertext
  if (sig < 0) {
    fprintf(stderr, "Error: Unable to write plaintext filename to socket.\n");
    exit(1);
  }
  sleep(1);

  //write plaintext file to socket port
  snprintf(buff, size, "%s", argv[1]);
  sig = write(sock_fd, buff, strlen(buff));
  //unable to write
  if (sig < 0) {
    fprintf(stderr, "Error: Unable to write plaintext filename to socket.\n");
    exit(1);
  }
  sleep(1);

  //write fey filename to socket port
  snprintf(buff, size, "%s", argv[2]);
  sig = write(sock_fd, buff, strlen(buff));
  //unable to write
  if (sig < 0) {
    fprintf(stderr, "Error: Unable to write key filename to socket.\n");
    exit(1);
  }

  //get response from server on given port
  bzero(buff, size);
  sig = read(sock_fd, buff, size);
  //unable to connect
  if (sig < 0) {
    fprintf(stderr, "Error: could not contact otp_end_d on port %d", portNumber);
    exit(2);
  }

  //if key is too short
  if (strcmp(buff,"error_1") == 0) {
    close(STDOUT_FILENO); 
    fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
    exit(1);
  }
  //if bad characters
  else if (strcmp(buff,"error_2") == 0) {
    close(STDOUT_FILENO); 
    fprintf(stderr, "otp_enc_d error: input contains bad characters\n");
    exit(1);
  }
  //cant connect to server on given port
  else if (strcmp(buff,"error_3") == 0) {
    close(STDOUT_FILENO); 
    fprintf(stderr, "otp_enc error: unable to connect to otp_enc_d on port %d\n", portNumber);
  }
  //all goes well
  else {
    printf("%s\n", buff);
  }

  //close socket port
  close(sock_fd);

  return 0;
}