/* otp_dec program */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
  //check to see number of args is correct
  if (argc < 4) { 
    fprintf(stderr, "otp_dec usage:\n { otp_dec ciphertext key port }\n");
    exit(1);
  }

  //open socket with given ports provided by user
  int portNumber = atoi(argv[3]);
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  //if socket cannot open
  if (sock_fd < 0) {
    fprintf(stderr, "Error: Unable to open socket.\n");
    exit(1);
  }

  struct sockaddr_in server_add;
  bzero((char*)&server_add, sizeof(server_add));
  server_add.sin_family = AF_INET;
  server_add.sin_port = htons(portNumber);
  server_add.sin_addr.s_addr = htonl(INADDR_ANY);

  //connect to port
  //print error if unable to connect
  if (connect(sock_fd, (struct sockaddr*)&server_add, sizeof(server_add)) < 0) {
    fprintf(stderr, "Error: could not contact otp_dec_d on port %d\n", portNumber);
    exit(2);
  }

  //write ciphertext identification to socket
  int size = 1024;
  char buffer[size];
  char sentinel[size];
  sentinel[1] = '*';
  snprintf(buffer, size, "%c", sentinel[1]);
  int b = write(sock_fd, buffer, strlen(buffer));
  //unable to write ciphertext
  if (b < 0) {
    fprintf(stderr, "Error: Unable to write ciphertext filename to socket.\n");
    exit(1);
  }
  sleep(1);


  //write ciphertext filename to socket port
  snprintf(buffer, size, "%s", argv[1]);
  b = write(sock_fd, buffer, strlen(buffer));
  //unable to write
  if (b < 0) {
    fprintf(stderr, "Error: Unable to write ciphertext filename to socket.\n");
    exit(1);
  }
  sleep(1);

  //write key name to socket port
  snprintf(buffer, size, "%s", argv[2]);
  b = write(sock_fd, buffer, strlen(buffer));
  //unable to write
  if (b < 0) {
    fprintf(stderr, "Error: Unable to write key filename to socket.\n");
    exit(1);
  }

  //connect to server on given port
  bzero(buffer, size);
  b = read(sock_fd, buffer, size);
  //unable to connect
  if (b < 0) {
    fprintf(stderr, "Error: could not contact otp_dec_d on port %d", portNumber);
    exit(2);
  }

  //get response from server on given port
  //unable to connect
  if (strcmp(buffer,"error_3") == 0) {
    close(STDOUT_FILENO); // No FILE
    fprintf(stderr, "otp_dec error: unable to connect to otp_dec_d on port %d\n", portNumber);
  }
  //send information 
  else {
    printf("%s\n", buffer);
  }

  //close socket
  close(sock_fd);

  return 0;
}