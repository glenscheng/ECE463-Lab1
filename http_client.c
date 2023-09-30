/* The code is subject to Purdue University copyright policies.
 * DO NOT SHARE, DISTRIBUTE, OR POST ONLINE
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgen.h> // for basename

#define MAX_BUFFER_SIZE 1024

// Function to create an HTTP GET request
char* create_get_request(const char* host, const char* path) {
  char* request = (char*)malloc(MAX_BUFFER_SIZE);
  if (request == NULL) {
    perror("Memory allocation error");
    exit(1);
  }

  snprintf(request, MAX_BUFFER_SIZE, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);
  return request;
}

int main(int argc, char *argv[])
{
  if (argc != 4) {
    fprintf(stderr, "usage: ./http_client [host] [port number] [filepath]\n");
    exit(1);
  }

  const char* host = argv[1];
  const char* port_str = argv[2];
  const char* filepath = argv[3];

  int port = atoi(port_str);

  // Extract the file name from the filepath
  const char* filename = basename((char*)filepath);

  // Create a socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("Socket creation error");
    exit(1);
  }

  // Resolve host name to IP address
  struct hostent* server = gethostbyname(host);
  if (server == NULL) {
    perror("Host not found");
    exit(1);
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

  // Connect to the server
  if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("Connection error");
    exit(1);
  }

  // Create an HTTP GET request
  char* request = create_get_request(host, filepath);

  // Send the request
  if (send(sockfd, request, strlen(request), 0) == -1) {
    perror("Request sending error");
    free(request);
    close(sockfd);
    exit(1);
  }

  // Receive and save the response to a file
  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    perror("File creation error");
    free(request);
    close(sockfd);
    exit(1);
  }

  char buffer[MAX_BUFFER_SIZE];
  int bytes_received;

  while ((bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
    fwrite(buffer, 1, bytes_received, file);
  }

  fclose(file);

  if (bytes_received == -1) {
    perror("Error while receiving response");
    free(request);
    close(sockfd);
    exit(1);
  }

  printf("File downloaded successfully: %s\n", filepath);

  // Clean up and close the socket
  free(request);
  close(sockfd);

  return 0;
}

