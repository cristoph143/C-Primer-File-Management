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
  // Create the path for the original file
  char originalPath[256];
  snprintf(originalPath, sizeof(originalPath), "files/%s", filename);

  // Read the content of the original file
  FILE *originalFile = fopen(originalPath, "rb");
  if (originalFile == NULL) {
    printf("Error opening original file '%s' for reading.\n", originalPath);
    return true; // Consider as corrupted if original file cannot be opened
  }

  fseek(originalFile, 0, SEEK_END);
  long originalSize = ftell(originalFile);
  fseek(originalFile, 0, SEEK_SET);
  char originalData[originalSize + 1];
  if (fread(originalData, sizeof(char), originalSize, originalFile) !=
      originalSize) {
    fclose(originalFile);
    printf("Error reading original file '%s'.\n", originalPath);
    return true; // Consider as corrupted if unable to read original file
  }
  originalData[originalSize] = '\0';
  fclose(originalFile);

  // Iterate through each duplicate file
  for (int i = 1; i <= NUM_DUPLICATES; i++) {
    // Create the path for the duplicate file
    char duplicatePath[256];
    snprintf(duplicatePath, sizeof(duplicatePath), "%s%s_duplicates%d.txt",
             FILE_FOLDER, filename, i);
    // Read the content of the duplicate file
    FILE *duplicateFile = fopen(duplicatePath, "rb");
    if (duplicateFile == NULL) {
      printf("Error opening duplicate file '%s' for reading.\n", duplicatePath);
      continue; // Continue with the next duplicate if this one cannot be opened
    }

    fseek(duplicateFile, 0, SEEK_END);
    long duplicateSize = ftell(duplicateFile);
    fseek(duplicateFile, 0, SEEK_SET);
    char duplicateData[duplicateSize + 1];
    if (fread(duplicateData, sizeof(char), duplicateSize, duplicateFile) !=
        duplicateSize) {
      fclose(duplicateFile);
      printf("Error reading duplicate file '%s'.\n", duplicatePath);
      continue; // Continue with the next duplicate if unable to read
    }
    duplicateData[duplicateSize] = '\0';
    fclose(duplicateFile);

    // Compare the content of the duplicate with the original
    if (strcmp(originalData, duplicateData) == 0) {
      // Found a matching duplicate, file is not corrupted
      printf("File '%s' is not corrupted. Match found in duplicate %d.\n",
             filename, i);
      return false;
    }
  }

  // None of the duplicates match the original, file is considered corrupted
  printf("File '%s' is corrupted. No matching duplicate found.\n", filename);
  return true;
}

void readFromFile(const char *filename) {
  // Create full file path by appending directory path
  char filepath[256]; // Adjust size as needed
  snprintf(filepath, sizeof(filepath), FILE_FOLDER "%s", filename);
  printf("filepath %s", filepath);

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
    // Check if file is corrupted by comparing with duplicates
    if (isFileCorrupted(filename)) {
      // Attempt to repair corrupted file
      repairFile(filename);
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

// Function to repair corrupted file
void repairFile(const char *filename) {
  printf("filename %s\n", filename);
  char originalFilename[100];
  snprintf(originalFilename, sizeof(originalFilename), "files/%s",
           filename); // Add "files/" prefix
  printf("%s\n", originalFilename);

  // Attempt to repair from duplicates
  for (int i = 1; i <= NUM_DUPLICATES; i++) {
    char duplicateFilename[100];
    snprintf(duplicateFilename, sizeof(duplicateFilename),
             "%s_duplicates%d.txt", originalFilename, i);
    printf("%s\n", duplicateFilename);

    FILE *originalFile = fopen(originalFilename, "rb");
    FILE *duplicateFile = fopen(duplicateFilename, "rb");

    if (originalFile == NULL || duplicateFile == NULL) {
      if (originalFile)
        fclose(originalFile);
      if (duplicateFile)
        fclose(duplicateFile);
      continue; // Skip if either original or duplicate file is not found
    }

    char originalData[MAX_DATA_SIZE];
    char duplicateData[MAX_DATA_SIZE];

    // Read data from original and duplicate files
    size_t originalSize =
        fread(originalData, sizeof(char), MAX_DATA_SIZE, originalFile);
    size_t duplicateSize =
        fread(duplicateData, sizeof(char), MAX_DATA_SIZE, duplicateFile);

    fclose(originalFile);
    fclose(duplicateFile);

    // Check if both files were read successfully
    if (originalSize == 0 || duplicateSize == 0) {
      printf("Error reading files for repair.\n");
      return;
    }
    // printf("originalSize == duplicateSize %d\n", originalSize ==
    // duplicateSize); printf("memcmp(originalData, duplicateData, originalSize)
    // == 0 %d\n",
    //        memcmp(originalData, duplicateData, originalSize) == 0);
    // Compare data
    if (originalSize == duplicateSize &&
        memcmp(originalData, duplicateData, originalSize) == 0) {
      FILE *file = fopen(originalFilename, "wb");
      if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
      }
      fwrite(duplicateData, sizeof(char), duplicateSize, file);
      fclose(file);

      printf("File repaired successfully using duplicate %d.\n", i);
      return;
    }
  }

  printf("File repair failed. No valid duplicates found.\n");
}

void decryptData(char *data) {
  char key = 0x5A; // Decryption key
  while (*data) {
    // *data = *data ^ key;

    *data = ~(*data);
    data++;
  }
}
