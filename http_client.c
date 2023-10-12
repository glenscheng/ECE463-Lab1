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

#define MAX_BUFFER_SIZE 65535

// Function to create an HTTP GET request
char *create_get_request(const char *host, const char *path) {
  char *request = (char *) malloc(MAX_BUFFER_SIZE);
  if (request == NULL) {
    perror("Memory allocation error");
    exit(1);
  }

  snprintf(request, MAX_BUFFER_SIZE, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);
  return request;
}

// My own basename function from libgen.h
const char *my_basename(const char* path) {
  const char *last_slash = strrchr(path, '/');
  if (last_slash == NULL) {
    return path;
  }
  return last_slash + 1;
}

int main(int argc, char **argv)
{
  if (argc != 4) {
    fprintf(stderr, "usage: ./http_client [host] [port number] [filepath]\n");
    exit(1);
  }

  const char *host = argv[1];
  const char *port_str = argv[2];
  const char *filepath = argv[3];

  int port = atoi(port_str);

  // Extract the file name from the filepath
  const char *filename = my_basename((char *) filepath);

  // Create a socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("Socket creation error");
    exit(1);
  }

  // Resolve host name to IP address
  struct hostent *server = gethostbyname(host);
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
  if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
    perror("Connection error");
    exit(1);
  }

  // Create an HTTP GET request
  char *request = create_get_request(host, filepath);

  // Send the request
  if (send(sockfd, request, strlen(request), 0) == -1) {
    perror("Request sending error");
    free(request);
    close(sockfd);
    exit(1);
  }

  /*
  // Receive and save the response to a file
  FILE *ofp = fopen(filename, "w");
  if (ofp == NULL) {
  perror("File creation error");
  free(request);
  close(sockfd);
  exit(1);
  }
   */

  FILE *ofp; // will open this if http_status_code == 200 and there is a Content-Length field
  char buffer[MAX_BUFFER_SIZE];
  int header_done = 0;  // Flag to track when headers are done
  int http_status_code = -1;
  long content_length = -1; // Initialize content_length to -1
  int bytes_received; // bytes received in each recv call

  while ((bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
    if (!header_done) {
      // Search for the blank line that separates headers from content
      char* blank_line = strstr(buffer, "\r\n\r\n");
      if (blank_line != NULL) {
        // Found the blank line, headers are done
        int content_start = blank_line - buffer + 4;  // +4 to skip \r\n\r\n
        // fwrite(buffer + content_start, 1, bytes_received - content_start, ofp);
        header_done = 1;  // Set the flag to indicate headers are done

        // deal with the status code
        if (http_status_code == -1) {
          char *status_line = strtok(buffer, "\r\n");
          if (status_line != NULL) {
            int trash;
            sscanf(status_line, "HTTP/1.%d %d", &trash, &http_status_code);
            if (http_status_code != 200) {
              fprintf(stdout, "%s\n", status_line);
              free(request);
              close(sockfd);
              exit(1);
            }
          }
        }
        // check for Content-Length field
        if (content_length == -1) {
          char *length_line;
          int content_length_field = -1;
          int retval;
          while ((retval = sscanf((length_line = strtok(NULL, "\r\n")), "Content-Length:  %ld", &content_length)) == 0) {
            if (retval == EOF) {
              content_length = -1;
              break;
            }
          }
          if (content_length == -1) { // Content-Length field not present in header
            fprintf(stdout, "Error: could not download the requested file (file length unknown)");
            free(request);
            close(sockfd);
            exit(1);
          }
        }
        ofp = fopen(filename, "w");
        if (ofp == NULL) {
          perror("File creation error");
          free(request);
          close(sockfd);
          exit(1);
        }
        fwrite(buffer + content_start, 1, bytes_received - content_start, ofp);
      }
    } else {
      // Write the content to the file
      fwrite(buffer, 1, bytes_received, ofp);
    }
  }

  // printf("http status code: %d\n", http_status_code);
  // printf("content length: %ld\n", content_length);

  // Clean up 
  fclose(ofp);
  free(request);
  close(sockfd);

  return 0;
}

