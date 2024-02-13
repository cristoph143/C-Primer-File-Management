#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <stdbool.h>
#include <time.h>

#define FILE_FOLDER "files/"
#define NUM_DUPLICATES 2
#define MAX_DATA_SIZE 100

// Structure to represent the header information
typedef struct {
  unsigned char fileID;
  unsigned char readCount;
} HeaderInfo;

void createDirectory(const char *path);
// Function prototypes
void encryptData(char *data);
// Function to calculate CRC (One's Complement)
unsigned char calculateCRC(HeaderInfo header, const char *data);
// Function to generate a random file ID
unsigned char generateRandomFileID();
// Function to write data to file
void writeToFile(const HeaderInfo header, const char *data);
void readFromFile(const char *filename);
// Function to repair corrupted file
void repairFile(const char *filename);
bool isFileCorrupted(const char *filename);
void decryptData(char *data); // Declaration of decryptData
#endif                        /* FILE_OPERATIONS_H */
