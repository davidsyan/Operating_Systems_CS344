/*
 *  Author: Nathan Thunem
 *	Course: CS344 - Program #2
 *	Filename: thunemn.adventure.c
 *
 *	Description:	This program creates a game map using different
 *		rooms.  The user has to traverse the map from the starting room
 *		to the ending room.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

//Create directory in current folder location
char *createDirectory(int pid);
//Create room files
struct Rooms createRooms(char *roomDirectory);
//store room files in directory
void storeRooms(char*);
//Randomize room names order
void randomizeRooms(char **arr, int size);

struct Rooms {
	char *startRoom;
	char *endRoom;
};

int main() {
	//create random seed
	srand(time(NULL));

	//get pid
	int pid = getpid();
	/* FOR TESTING  */
	printf("PID=%d\n", pid);
	//create room directory
	char *roomDirectory = createDirectory(pid);
	//createDirectory(pid);
	//create each room
	struct Rooms startLoc = createRooms(roomDirectory);

	return 0;
}

char *createDirectory(int pid) {
	//max size of room directory name
	int size = 25;
	//allocate size for directory name string
	char *directoryName = malloc(size);
	//generic name of file (without pid)
	char *genericName = "thunemn.rooms.";

	//create directory name
	//snprintf(char *str, size, string format, var1, var2);
	snprintf(directoryName, size, "%s%d", genericName, pid);

	//create directory
	struct stat st = {0};
	if(stat(directoryName, &st) == -1) {
		mkdir(directoryName);//, 0755);
	}

	return directoryName;
}

void randomizeRooms(char **arr, int size) {
	int i;
	for(i = 0; i < size; ++i) {
		//create random number from 0-(size-1)
		int swapIndex = rand()%size;
		//swap room names
		char *temp = arr[i];
		arr[i] = arr[swapIndex];
		arr[swapIndex] = temp;
	}
}

struct Rooms createRooms(char *roomDirectory) {
	struct Rooms r;
	//room names
	char *roomNames[10];
	roomNames[0] = "Giraffe";
	roomNames[1] = "Elephant";
	roomNames[2] = "Aardvark";
	roomNames[3] = "Gator";
	roomNames[4] = "Canary";
	roomNames[5] = "Rhino";
	roomNames[6] = "Flamingo";
	roomNames[7] = "Peacock";
	roomNames[8] = "Cardinal";
	roomNames[9] = "Lion";

	randomizeRooms(roomNames, 10);

	//create room files
	//file name size
	int size = 128;
	char *fileName = malloc(size);
	//rooms being used
	char *roomsUsed[7];
	int i;
	for(i = 0; i < 7; ++i) {
		//create file name- format: roomDirectory/roomNames[i]
		//For example: thunemn.rooms.19903/Aardvark
		snprintf(fileName, size, "%s/%s", roomDirectory, roomNames[i]);
		FILE *f = fopen(fileName, "w");
		if(f == NULL) {
			perror("Unable to open file.\n");
			exit(1);
		}
		//add room name to files
		else {
			fprintf(f, "ROOM NAME: %s\n", roomNames[i]);
			roomsUsed[i] = roomNames[i];
		}

		//close file
		fclose(f);
	}

	//determine which room is START, MID, END
	//random number 0-6
	int startRoomIndex = rand()%7;
	int endRoomIndex = rand()%7;
	//make sure start and end room is not the same
	while(startRoomIndex == endRoomIndex) {
		endRoomIndex = rand()%7;
	}

	int numConnections;
	char *currentRoom;
	char *connectedRoom;
	int numRoomConnections[6];
	int roomConnectionsCount;
	//manage connections between rooms
	for(i = 0; i < 6; ++i) {
		numRoomConnections[i] = 0;
	}

	//make connections for each room
	for(i = 0; i < 7; ++i) {
		//randomize room order to make connections random every time
		randomizeRooms(roomNames, 7);
		//get file name to open file
		snprintf(fileName, size, "%s/%s", roomDirectory, roomsUsed[i]);
		//open file- append mode
		FILE *f = fopen(fileName, "a");

		if(f == NULL) {
			perror("Unable to open file.\n");
			exit(1);
		}
		else {
			//get amount of connections to make (3-6)
			numConnections = rand()%4 + 3;
			//subtract number of connections from number of existing connections
			numConnections -= numRoomConnections[i];
			currentRoom = roomsUsed[i];
			int k = 0;
			int j;
			for(j = 0; j < numConnections; ++j) {
				connectedRoom = roomsUsed[k];
				//prevent room connecting to itself
				if(connectedRoom == currentRoom) {
					++k;	//go to next in line
					connectedRoom = roomsUsed[k];
				}
				//print connection in room file
				fprintf(f, "CONNECTION %d: %s\n", j+1, connectedRoom);
				//++numRoomConnections[k];	//increase connection count for connecting room
				roomConnectionsCount = numRoomConnections[k] + 1;
				++k;		//go to next in line

				//close current file
				fclose(f);
				//open new connecting room file
				snprintf(fileName, size, "%s/%s", roomDirectory, connectedRoom);
				FILE *fConnected = fopen(fileName, "a");
				//print current room connection in connecting room file
				fprintf(fConnected, "CONNECTION %d: %s\n", roomConnectionsCount, currentRoom);
				//close connecting file
				fclose(f);
				//open current file
				snprintf(fileName, size, "%s/%s", roomDirectory, currentRoom);
				f = fopen(fileName, "a");
			}
		}

	}

	return r;
}




