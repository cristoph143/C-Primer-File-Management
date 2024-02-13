## Test Scenarios for File Management Program

### Instructions:

#### Write:
- The program shall provide an interface to create and save the data to a file.
- On every file created and saved, 2 file duplicates shall be created and shall be stored into a separate folder.
- On every file saved, the CRC shall be computed and stored.
- File data contents shall not be readable from any text editor.

#### Read:
- The program shall provide an interface to retrieve the data from the file.
- The program shall provide an interface to retrieve the number of times the file is read.
- On every read, validation shall be performed:
  - CRC to be validated for data integrity
  - ID from the header to be checked for file validity
  - Contents of the 3 files are compared to check data corruption. If 1 is corrupted, automatically, the program will update the corrupted file.
- On every read, read count shall be updated.

---

## Header Information: The header consists of 2 bytes. The first byte is the file ID, and the second byte contains the read count of the file.

## Data: This is the actual content of the file, such as "Hello World" in your example.

## CRC (Cyclic Redundancy Check): The CRC is calculated as the One's Complement of the sum of the Header Information and the Data.

1. **Creating and Saving a File**:
    - Create a new file with Header Information = 0xFA05 and Data = "Hello World".
    - Check if the file is saved correctly.
    - Verify that two duplicate files are created and stored in a separate folder.
    - Confirm that the CRC is computed and stored correctly.

2. **Reading a File**:
    - Choose a file to read.
    - Retrieve the data from the file.
    - Ensure that the data retrieved matches the original data.
    - Check if the CRC is valid for data integrity.
    - Verify that the file ID is correct and matches the expected value.
    - Compare the contents of the three files (original and duplicates) to check for data corruption.
    - Confirm that the read count is updated.

3. **Retrieving the Number of Times the File is Read**:
    - Read the file multiple times.
    - Retrieve the number of times the file has been read.
    - Ensure that the read count is incremented correctly after each read operation.

4. **Data Corruption Handling**:
    - Introduce data corruption in one of the duplicate files.
    - Read the file and verify that the program automatically detects the corruption.
    - Confirm that the corrupted file is updated with correct data.

5. **Data Security**:
    - Attempt to open the saved files in a text editor to confirm that the file data contents are not readable.

6. **Boundary Testing**:
    - Test with different combinations of Header Information, including edge cases and boundary values.
    - Test with varying lengths of Data, including empty data and maximum data length.

7. **Error Handling**:
    - Test for error handling scenarios such as trying to read a non-existent file or attempting to create a file in a directory without proper permissions.
    - Verify that appropriate error messages are displayed.

8. **Performance Testing**:
    - Create and read a large number of files to test the performance of file creation, duplication, and reading operations.
    - Measure the time taken for each operation and ensure it meets the expected performance requirements.

By systematically testing these scenarios, you can validate that the program meets all the specified requirements and performs as expected under different conditions.

## Flow
**Write Process:**
1. **User Interface for Data Entry:**
   - Prompt the user to input the file's content (data).
   - Allow the user to specify the file ID and read count.

2. **Calculate CRC:**
   - Calculate the CRC by taking the One's Complement of the sum of the Header Information and Data.

3. **Save Data to File:**
   - Write the Header Information, Data, and CRC to a file.
   - Store this file in a designated folder.
   - Create two duplicates of the file and store them in the same folder.

4. **Ensure File Security:**
   - Encrypt or obscure the file contents to prevent direct reading from a text editor.

**Read Process:**
1. **User Interface for File Selection:**
   - Allow the user to select the file to be read.

2. **Retrieve File Data:**
   - Read the Header Information, Data, and CRC from the selected file.

3. **Validate CRC:**
   - Recalculate the CRC from the Header Information and Data.
   - Compare the recalculated CRC with the CRC stored in the file to verify data integrity.

4. **Validate Header Information:**
   - Ensure that the file ID matches the expected format.
   - Check the read count to determine the number of times the file has been read.

5. **Compare File Contents:**
   - Compare the contents of the three files (original and duplicates) to check for data corruption.
   - If one file is corrupted, automatically update the corrupted file with data from the other copies.

6. **Update Read Count:**
   - Increment the read count for the file.

7. **Provide File Data:**
   - Provide the file's content (data) to the user interface for display or further processing.

