/*
Name: AKASH VASU
Regd_no: 25048_039
Submission Date: 21/05/2026
Project name : Steaganography
Title : Image Steganography using LSB Technique in BMP Images
*/


#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "common.h"


Opr_type check_operation(char *operation_option)
{
    if(strcmp(operation_option, "-e") == 0)
        return encode;

    else if(strcmp(operation_option, "-d") == 0)
        return decode;

    else
        return unsupported;
}


int main(int argc, char *argv[])
{
    EncodeInfo encodeInfo;

    if(argc == 1)
    {
        printf("INFO : Please pass proper command line arguments\n");

        printf("INFO : Encoding  -> ./a.out -e <source.bmp> <secret_file> <output.bmp>\n");

        printf("INFO : Decoding  -> ./a.out -d <stego.bmp> <output_file>\n");

        return 0;
    }


    int operation_type = check_operation(argv[1]);


    if(operation_type == encode)
    {
        if(argc < 4 || argc > 5)
        {
            printf("INFO : \033[1;31mFailure\033[0m : Invalid arguments for encoding\n");

            printf("INFO : Usage -> ./a.out -e <source.bmp> <secret_file> <output.bmp>\n");

            return 0;
        }
        else
        {
            printf("INFO : You selected encoding operation\n");

            if(validate_encode_args(argv,
                                    &encodeInfo) == failure)
            {
                printf("INFO : \033[1;31mFailure\033[0m : Validation failed\n");

                return 0;
            }
            else
            {
                printf("INFO : Validation completed successfully\n");

                printf("INFO : Source image file : %s\n",
                        encodeInfo.src_image_fname);

                printf("INFO : Secret file : %s\n",
                       encodeInfo.secret_fname);

                printf("INFO : Output image file : %s\n",
                       encodeInfo.output_image_fname);

                do_encoding(argv,
                            &encodeInfo);
            }
        }
    }


    else if(operation_type == decode)
    {
        DecodeInfo decodeInfo;

        if(argc < 3 || argc > 4)
        {
            printf("INFO : \033[1;31mFailure\033[0m : Invalid arguments for decoding\n");

            printf("INFO : Usage -> ./a.out -d <stego.bmp> <output_file>\n");

            return 0;
        }
        else
        {
            printf("INFO : You selected decoding operation\n");

            if(validate_decode_args(argv,
                                    &decodeInfo) == failure)
            {
                printf("INFO : \033[1;31mFailure\033[0m : Validation failed\n");

                return 0;
            }
            else
            {
                printf("INFO : Validation completed successfully\n");

                do_decoding(argv,
                            &decodeInfo);
            }
        }
    }


    else
    {
        printf("INFO : \033[1;31mFailure\033[0m : Invalid operation selected\n");

        printf("INFO : Please use only:\n");

        printf("INFO : -e for encoding\n");

        printf("INFO : -d for decoding\n");
    }

    return 0;
}


/*
Project Description:
This project hides a secret file inside a .bmp image using the Least Significant Bit (LSB) method and later retrieves the hidden file back without visibly changing the image.

This project supports both:
Encoding → Hiding secret data inside image
Decoding → Extracting hidden data from image

The project is implemented completely in C programming language.

1. What is Steganography?
Steganography is a technique used to hide secret information inside another file such as:
Image, Audio, Video, Text ,without making the hidden information noticeable.

In this project:

Carrier File → BMP image
Secret File → .txt, .c, .cpp, .sh
Output → Stego image containing hidden data

2. Main Objective of the Project

The main objective is:
To securely hide a secret file inside an image
To retrieve the hidden file later without data loss
To make the image appear visually unchanged

3. Technique Used – LSB (Least Significant Bit)

This project uses:
LSB Encoding Technique
Every pixel byte in a BMP image contains 8 bits.

Example:

Original byte:
10110110
The last bit is called:
Least Significant Bit (LSB)
Your program modifies only this last bit.
Example:
10110110
Change LSB to 1:
10110111
Only 1 bit changes.
This tiny change is invisible to the human eye.

4. Why BMP Image?

BMP images are used because:
BMP is uncompressed
Pixel bytes are stored directly
Easy to modify bits
No quality loss after editing
This code checks .bmp format during validation.

5. Files Used in the Project
1. main.c
Controls the entire program.
Responsibilities:
Reads command-line arguments
Detects encode/decode operation
Calls validation functions
Calls encoding or decoding functions

Functions:
check_operation()
main()

2. common.h

Contains common definitions.
Includes:
Magic string
Status enums
Operation enums
Magic string used:
#define MAGIC_STRING "#*"

3. encode.h

Contains:
Encode structure
Function declarations
Stores:
Source image info
Secret file info
Output image info

4. encode.c

Performs complete encoding operation.
Responsibilities:
Open files
Validate files
Check image capacity
Copy BMP header
Encode data into image
Create stego image

5. decode.h

Contains:
Decode structure
Decoding function declarations

6. decode.c

Performs complete decoding process.
Responsibilities:
Open stego image
Verify magic string
Decode extension
Decode secret size
Decode hidden data
Recreate original file

6. Project Workflow
Encoding Process
Secret File + BMP Image
        ↓
Read Secret Data
        ↓
Convert Secret Data into Bits
        ↓
Modify LSB of BMP Pixels
        ↓
Create Stego Image
Decoding Process
Stego Image
      ↓
Read LSB Bits
      ↓
Reconstruct Binary Data
      ↓
Recover Original Secret File


7. Encoding Process – Step by Step
Step 1: Validate Arguments

Program checks:
Source image is .bmp
Secret file exists
Output file exists
Supported secret files:
.txt
.c
.cpp
.sh
Step 2: Open Files
The program opens:
Source BMP image
Secret file
Output stego image
Using:
fopen()

Step 3: Check Image Capacity
Program calculates:
Image Capacity > Secret Data Size
BMP capacity formula:
width × height × 3
because BMP stores:
Red
Green
Blue
bytes for each pixel.
Step 4: Copy BMP Header
BMP header size
54 bytes
The header is copied directly because it contains:
Image format
Width
Height
Color information
Step 5: Encode Magic String
Magic string:
#*
Purpose:

Identifies whether image contains hidden data
Each character is stored using:
8 image bytes
Step 6: Encode File Extension
Example:
.txt
Program stores:
Extension size
Extension characters
This helps decoder recreate original file type.

Step 7: Encode Secret File Size

File size is stored as:
4-byte integer
using:
32 image bytes

Step 8: Encode Secret File Data
Each secret character:
1 byte
is hidden into:
8 image bytes

using LSB substitution.

Example:
Character:
A = 01000001
Each bit goes into one image byte LSB.
Step 9: Copy Remaining Image Data
After encoding completes:
Remaining bytes are copied unchanged.

8. Decoding Process – Step by Step
Step 1: Open Stego Image
Decoder opens encoded BMP image.
Step 2: Verify Magic String
Decoder extracts hidden magic string.
If it matches:
#*
then image contains secret data.
Step 3: Decode File Extension
Decoder extracts:
Extension size
Extension characters
Then recreates filename.

Step 4: Decode Secret File Size
Extracts original hidden file size.

Step 5: Decode Secret Data
Reads:
8 image bytes → 1 secret byte
Reconstructs original file completely.

9. Important Concepts Used
File Handling
Functions used:

fopen()
fread()
fwrite()
fseek()
ftell()
fclose()
Bitwise Operations
Used heavily in LSB encoding.
Operators:
&
|
<<
>>
~

Encoding
./a.out -e beautiful.bmp secret.txt output.bmp
Decoding
./a.out -d output.bmp decoded
10. Advantages of the Project
Data hidden securely
Image quality almost unchanged
Simple implementation
Efficient memory usage
Supports multiple file types
Fast encoding/decoding

16. Conclusion
This project successfully demonstrates how secret information can be hidden inside a BMP image using LSB steganography. The implementation uses file handling, structures, bitwise operations, and command-line processing in C language to securely encode and decode secret files while preserving image appearance.
The project is simple, efficient, and useful for understanding real-world data hiding techniques and secure communication systems.
*/