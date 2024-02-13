CC = gcc
CFLAGS = -Wall -Werror
LDFLAGS = -lpulse-simple -lpulse -lm -lcurl

SRC_DIR = src
LIB_DIR = lib

CLIENT = client
DROPBOX_TESTER = dropbox_tester
SERVER = server

CLIENT_SRC = client.c
SERVER_SRC = server.c
DROPBOX_TESTER_SRC = dropbox_tester.c
AUDIO_SRC = $(SRC_DIR)/audio.c
PACKET_SRC = $(SRC_DIR)/packet.c
DROPBOX_SRC = $(SRC_DIR)/dropbox.c
JSON_SRC = $(SRC_DIR)/json.c

CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
SERVER_OBJ = $(SERVER_SRC:.c=.o)
DROPBOX_TESTER_OBJ = $(DROPBOX_TESTER_SRC:.c=.o)
AUDIO_OBJ = $(AUDIO_SRC:.c=.o)
PACKET_OBJ = $(PACKET_SRC:.c=.o)
DROPBOX_OBJ = $(DROPBOX_SRC:.c=.o)
JSON_OBJ = $(JSON_SRC:.c=.o)

.PHONY: all clean

all: client server dropbox_tester

client: $(CLIENT_OBJ) $(AUDIO_OBJ) $(PACKET_OBJ) $(DROPBOX_OBJ) $(JSON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

server: $(SERVER_OBJ) $(AUDIO_OBJ) $(PACKET_OBJ) $(DROPBOX_OBJ) $(JSON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

dropbox_tester: $(DROPBOX_TESTER_OBJ) $(AUDIO_OBJ) $(PACKET_OBJ) $(DROPBOX_OBJ) $(JSON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT_OBJ): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

$(SERVER_OBJ): $(SERVER_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

$(AUDIO_OBJ): $(AUDIO_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

$(PACKET_OBJ): $(PACKET_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

$(DROPBOX_OBJ): $(DROPBOX_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

$(JSON_OBJ): $(JSON_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(CLIENT) $(SERVER) $(DROPBOX_TESTER) $(CLIENT_OBJ) $(SERVER_OBJ) $(DROPBOX_TESTER_OBJ) $(AUDIO_OBJ) $(PACKET_OBJ) $(DROPBOX_OBJ) $(JSON_OBJ)
