#include "xeric-utils.h"

void accept_client(void);
int output_file(int client, const char * filename);
void process_block(const char * block, int block_size);

/* The main method. */
int main(int argc, char * argv[]) {
	int i;
	int port = 80;
	struct in_addr address = {0};

	/* Parse the arguments: */
	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "--port") == 0) {
			i++;

			if(i == argc) {
				warn("Expected port number after --port option. Continuing with port 80.");
				break;
			}

			port = strtoport(argv[i]);

			if(!port) {
				warn("Invalid port number specified. Continuing with port 80.");
				port = 80;
			}
		} else if(strcmp(argv[i], "--address") == 0) {
			i++;

			if(i == argc) {
				warn("Expected address after --address option. Continuing with address 0.0.0.0.");
				break;
			}

			if(inet_pton(AF_INET, argv[i], &address) != 1) {
				warn("Invalid address specified. Continuing with address 0.0.0.0.");
			}
		} else {
			printf("Unrecognized option %s.\n", argv[i]);
		}
	}

	char str_address[16];
	struct sockaddr_in serv_addr;

	/* Create the socket: */
	s = socket(AF_INET, SOCK_STREAM, 0);

	if(s < 0) {
		warn("Error opening socket.");
		return 1;
	}

	/* Listen on the socket: */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr = address;

	errno = 0;

	if(bind(s, (struct sockaddr *) &serv_addr, sizeof serv_addr) < 0) {
		if(errno == EADDRINUSE)
			warn("Could not bind to port; already in use.");
		else
			warn("Could not bind to port due to an unknown error.");

		return 1;
	}

	listen(s, 5);

	success("Xeric 0.1.0 has started!");

	inet_ntop(AF_INET, &serv_addr.sin_addr, str_address, 15);
	printf("Listening at %s:%d.\n", str_address, port);

	/* Enable the Ctrl+C SIGINT handler so that the socket can be closed */
	signal(SIGINT, interrupted);

	while(keepRunning) {
		accept_client();
	}

	if(s >= 0) {
		close(s);
	}

	return 0;
}

void accept_client(void) {
	char buffer[1024];
	struct sockaddr_in cli_addr;
	int n;
	int client;
	socklen_t clilen = sizeof(struct sockaddr_in);

	/* Accept a client: */
	client = accept(s, (struct sockaddr *)&cli_addr, &clilen);

	if(client < 0) {
		warn("Error on accept.");
		return;
	}

	ip4_t r_addr = {cli_addr.sin_addr.s_addr};
	printf("Got client from %d.%d.%d.%d.\n", r_addr.components.p1, r_addr.components.p2, r_addr.components.p3, r_addr.components.p4);

	/* Read data from the client: */
	do {
		bzero(buffer, 1024);
		n = read(client, buffer, 1023);

		if(n < 0) {
			warn("Error reading from socket; breaking connection.");
			close(client);
			return;
		}

		process_block(buffer, n);
	} while(n == 1023);

	process_block(NULL, 0);

	printf("\n");

	/* Write data to the client: */
	const char * output = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
	n = write(client, output, strlen(output));

	if(n < 0) {
		warn("Error writing to socket; breaking connection.");
		close(client);
		return;
	}

	if(output_file(client, "public/index.html")) {
		close(client);
		return;
	}

	/* Close the connection: */
	close(client);
}

int output_file(int client, const char * filename) {
	FILE * fp = fopen(filename, "r");

	if(!fp) {
		warn("Failed to read from file.");
		return 1;
	}

	char buffer[1024];
	size_t n;

	while(1) {
		n = fread(buffer, 1, 1024, fp);

		if(!n) {
			break;
		}

		n = write(client, buffer, n);

		if(n <= 0) {
			warn("Error writing to socket; breaking connection.");
			return 1;
		}
	}

	fclose(fp);

	return 0;
}

void process_block(const char * block, int block_size) {
	static enum {
		HTTP_HEADER,
		HEADER_NAME,
		HEADER_VALUE,
		BODY
	} state = HTTP_HEADER;

	static string_buffer buffer;
	static int sb_initialized = 0;

	if(!sb_initialized) {
		sb_initialized = 1;
		buffer = sb_alloc();
	}

	if(block == NULL) {
		state = HTTP_HEADER;
	}

	int i = -1;

	switch(state) {
		case HTTP_HEADER: goto _0;
		case HEADER_NAME: goto _1;
		case HEADER_VALUE: goto _2;
		case BODY: goto _3;
	}

_0:
	while(++i < block_size) {
		if(block[i] == '\n') {
			state = HEADER_NAME;
			printf("Got HTTP header: \"%s\"\n", sb_tostring(buffer));
			sb_free(buffer);
			buffer = sb_alloc();
			goto _1;
		}

		if(block[i] != '\r') /* Hm... */
			sb_append(&buffer, block[i]);
	}

	return;

_1:
	while(++i < block_size) {
		if(block[i] == '\n') {
			/* TODO: Check that current header name is empty */
			state = BODY;
			goto _3;
		} else if(block[i] == ':') {
			state = HEADER_VALUE;
			printf("Got header name: \"%s\"\n", sb_tostring(buffer));
			sb_free(buffer);
			buffer = sb_alloc();
			goto _2;
		}

		if(block[i] != '\r') /* Hm... */
			sb_append(&buffer, block[i]);
	}

	return;

_2:
	while(++i < block_size) {
		if(block[i] == '\n') {
			state = HEADER_NAME;
			printf("Got header value: \"%s\"\n", sb_tostring(buffer));
			sb_free(buffer);
			buffer = sb_alloc();
			goto _1;
		}

		if(block[i] != '\r') /* Hm... */
			sb_append(&buffer, block[i]);
	}

	return;

_3:
	while(++i < block_size) {
		sb_append(&buffer, block[i]);
	}

	printf("Got request body: \"%s\"\n", sb_tostring(buffer));
	sb_free(buffer);
	buffer = sb_alloc();
}