#include "file_operations.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void createDirectory(const char *path) {
  char command[256];
  sprintf(command, "mkdir -p %s", path);
  system(command);
}

char *constructFilePath(const char *filename) {
  char *filepath = malloc(strlen(FILE_FOLDER) + strlen(filename) + 1);
  if (filepath == NULL) {
    perror("Memory allocation failed");
    exit(EXIT_FAILURE);
  }
  strcpy(filepath, FILE_FOLDER);
  strcat(filepath, filename);
  return filepath;
}

void writeToFile(const HeaderInfo header, const char *data) {
  // Calculate CRC
  unsigned char crc = calculateCRC(header, data);

  // Create folder if it doesn't exist
  createDirectory(FILE_FOLDER);

  // Construct file name
  char filename[100];
  snprintf(filename, sizeof(filename), "%d_%d.txt", header.fileID,
           header.readCount);

  // Create file path
  char *filepath = constructFilePath(filename);

  // Write header, data, and CRC to file
  writeHeaderAndDataToFile(header, data, filepath);
  // Create duplicates
  for (int i = 0; i < NUM_DUPLICATES; i++) {
    char duplicateFilename[100];
    snprintf(duplicateFilename, sizeof(duplicateFilename),
             "%s%s_duplicates%d.txt", FILE_FOLDER, filename, i + 1);
    writeHeaderAndDataToFile(header, data, duplicateFilename);
  }

  printf("File saved successfully.\n");
  free(filepath);
}

void writeHeaderAndDataToFile(const HeaderInfo header, const char *data,
                              const char *filepath) {
  // Open file for writing
  FILE *file = fopen(filepath, "wb");
  if (file == NULL) {
    perror("Error opening file for writing");
    free((char *)filepath);
    exit(EXIT_FAILURE);
  }

  // Write header, data, and CRC to file
  fwrite(&header, sizeof(HeaderInfo), 1, file);
  fwrite(data, sizeof(char), strlen(data), file);
  unsigned char crc = calculateCRC(header, data);
  fwrite(&crc, sizeof(unsigned char), 1, file);

  // Close file
  fclose(file);
}

// Function to calculate CRC (One's Complement)
unsigned char calculateCRC(HeaderInfo header, const char *data) {
  unsigned int sum = header.fileID + header.readCount;
  printf("Sum: %d\n", sum);
  while (*data) {
    sum += *data;
    data++;
  }
  printf("data %s", data);
  return ~(sum & 0xFF); // One's complement
}

// Function to generate a random file ID
unsigned char generateRandomFileID() {
  return rand() % 256; // Generates a random number between 0 and 255
}

// Function to encrypt data (simple XOR encryption for demonstration purposes)
void encryptData(char *data) {
  char key = 0x5A; // Encryption key
  while (*data) {
    *data = *data ^ key;
    // *data = ~(*data);
    data++;
  }
}

bool isFileCorrupted(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    printf("Error opening file '%s'.\n", filename);
    return true; // Treat as corrupted if file cannot be opened
  }

  // Read a portion of the file content and perform checks for corruption
  char buffer[BUFFER_SIZE];
  size_t bytesRead = fread(buffer, 1, BUFFER_SIZE, file);

  // Example: Check if the file is empty or smaller than expected
  bool isCorrupted = false;
  if (bytesRead == 0) {
    isCorrupted = true; // Empty file is considered corrupted
  } else if (bytesRead < MAX_DATA_SIZE) {
    isCorrupted = true; // File is smaller than expected
  }
  // You can add more checks here based on your file format or requirements

  fclose(file);
  return isCorrupted;
}

void readFromFile(const char *filename) {
  // Create full file path by appending directory path
  char filepath[256]; // Adjust size as needed
  snprintf(filepath, sizeof(filepath), FILE_FOLDER "%s", filename);
  printf("filepath %s\n", filepath);

  // Open file for reading
  FILE *file = fopen(filepath, "rb");
  if (file == NULL) {
    printf("Error opening file '%s' for reading.\n", filepath);
    return;
  }

  // Read header from file
  HeaderInfo header;
  if (fread(&header, sizeof(HeaderInfo), 1, file) != 1) {
    printf("Error reading header from file '%s'.\n", filepath);
    fclose(file);
    return;
  }

  // Calculate data size
  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, sizeof(HeaderInfo), SEEK_SET);
  long dataSize = fileSize - sizeof(HeaderInfo) - sizeof(unsigned char);

  // Allocate memory for data
  char *data = (char *)malloc(dataSize + 1);
  if (data == NULL) {
    printf("Error allocating memory for data.\n");
    fclose(file);
    return;
  }

  // Read data from file
  if (fread(data, sizeof(char), dataSize, file) != dataSize) {
    printf("Error reading data from file '%s'.\n", filepath);
    free(data);
    fclose(file);
    return;
  }
  data[dataSize] = '\0';
  // decryptData(data); // Decrypt data

  // Read CRC from file
  unsigned char crc;
  if (fread(&crc, sizeof(unsigned char), 1, file) != 1) {
    printf("Error reading CRC from file '%s'.\n", filepath);
    free(data);
    fclose(file);
    return;
  }

  // Close file
  fclose(file);

  // Validate CRC
  unsigned char calculatedCRC = calculateCRC(header, data);
  if (calculatedCRC != crc) {
    printf("CRC validation failed for file '%s'. Data may be corrupted.\n",
           filepath);
    // Check and repair corrupted file
    if (!checkAndRepairCorruptedFile(filename, filename)) {
      printf("Failed to repair file '%s'.\n", filepath);
    }
    free(data);
    return;
  }

  // Validate Header Information
  if (header.fileID < 0 || header.fileID > 255) {
    printf("Invalid file ID for file '%s'.\n", filepath);
    free(data);
    return;
  }

  // Increment read count
  header.readCount++;

  // Recalculate CRC based on updated data
  crc = calculateCRC(header, data);

  // Rewrite the file with updated read count and CRC
  file = fopen(filepath, "wb");
  if (file == NULL) {
    printf("Error opening file '%s' for writing.\n", filepath);
    free(data);
    return;
  }
  fwrite(&header, sizeof(HeaderInfo), 1, file);
  fwrite(data, sizeof(char), dataSize, file);
  fwrite(&crc, sizeof(unsigned char), 1, file);
  fclose(file);

  // Display file content and header information
  printf("File ID: %d\n", header.fileID);
  printf("Read Count: %d\n", header.readCount);
  printf("Data: %s\n", data);

  // Free allocated memory
  free(data);
}

bool compareFiles(const char *file1, const char *file2) {
  FILE *f1 = fopen(file1, "rb");
  FILE *f2 = fopen(file2, "rb");

  if (f1 == NULL || f2 == NULL) {
    // Error opening files
    if (f1)
      fclose(f1);
    if (f2)
      fclose(f2);
    return false;
  }

  char buffer1[BUFFER_SIZE];
  char buffer2[BUFFER_SIZE];

  size_t bytesRead1, bytesRead2;

  do {
    bytesRead1 = fread(buffer1, 1, BUFFER_SIZE, f1);
    bytesRead2 = fread(buffer2, 1, BUFFER_SIZE, f2);

    if (bytesRead1 != bytesRead2 || memcmp(buffer1, buffer2, bytesRead1) != 0) {
      fclose(f1);
      fclose(f2);
      return false; // Files are different
    }
  } while (bytesRead1 > 0);

  fclose(f1);
  fclose(f2);
  return true; // Files are identical
}

// Function to copy content from one file to another
bool copyFileContent(const char *sourceFilename,
                     const char *destinationFilename) {
  FILE *sourceFile = fopen(sourceFilename, "rb");
  FILE *destinationFile = fopen(destinationFilename, "wb");

  if (sourceFile == NULL || destinationFile == NULL) {
    if (sourceFile)
      fclose(sourceFile);
    if (destinationFile)
      fclose(destinationFile);
    return false;
  }

  // Copy header
  HeaderInfo header;
  if (fread(&header, sizeof(HeaderInfo), 1, sourceFile) != 1) {
    fclose(sourceFile);
    fclose(destinationFile);
    return false;
  }
  fwrite(&header, sizeof(HeaderInfo), 1, destinationFile);

  // Copy data
  char buffer[BUFFER_SIZE];
  size_t bytesRead;

  while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, sourceFile)) > 0) {
    fwrite(buffer, 1, bytesRead, destinationFile);
  }

  fclose(sourceFile);
  fclose(destinationFile);
  return true;
}

// Function to repair corrupted file using duplicates
bool isRepairFile(const char *originalFilename, const char *corruptedFilename) {
  printf("Attempting to repair file '%s'...\n", corruptedFilename);

  // Construct the paths to the original and duplicate files
  char originalPath[256];
  snprintf(originalPath, sizeof(originalPath), "files/%s", originalFilename);

  char duplicatePath[256];
  snprintf(duplicatePath, sizeof(duplicatePath), "files/%s", corruptedFilename);

  // Open the original and duplicate files
  FILE *originalFile = fopen(originalPath, "ab");
  FILE *duplicateFile = fopen(duplicatePath, "rb");

  if (originalFile == NULL || duplicateFile == NULL) {
    printf("Error opening files for repair.\n");
    if (originalFile)
      fclose(originalFile);
    if (duplicateFile)
      fclose(duplicateFile);
    return false;
  }

  // Read data from the duplicate file and copy it to the original file
  char buffer[BUFFER_SIZE];
  size_t bytesRead;
  while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, duplicateFile)) > 0) {
    fwrite(buffer, 1, bytesRead, originalFile);
  }

  // Close the files
  fclose(originalFile);
  fclose(duplicateFile);

  printf("File repaired successfully using duplicate '%s'.\n",
         corruptedFilename);
  return true; // Return true to indicate success
}
bool checkAndRepairCorruptedFile(const char *originalFilename,
                                 const char *corruptedFilename) {
  // Read the content of the corrupted file
  FILE *corruptedFile = fopen(corruptedFilename, "rb");
  if (corruptedFile == NULL) {
    printf("Error opening corrupted file '%s'.\n", corruptedFilename);
    return false;
  }

  // Read the content and perform checks for corruption
  bool isCorrupted = isFileCorrupted(corruptedFilename);

  // Close the corrupted file
  fclose(corruptedFile);

  // If the file is corrupted, attempt to repair it
  if (isCorrupted) {
    printf("Corruption detected in file '%s'. Attempting repair...\n",
           corruptedFilename);
    return isRepairFile(originalFilename, corruptedFilename);
  } else {
    printf("File '%s' is not corrupted.\n", corruptedFilename);
    return true;
  }
}

void decryptData(char *data) {
  char key = 0x5A; // Decryption key
  while (*data) {
    // *data = *data ^ key;

    *data = ~(*data);
    data++;
  }
}
