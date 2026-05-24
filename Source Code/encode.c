#include <stdio.h>
#include<string.h>
#include "encode.h"


/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
unsigned int get_image_size_for_bmp(FILE *fptr_image)
{
    unsigned int width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: success or failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    encInfo->src_image_fptr = fopen(encInfo->src_image_fname, "r");

    if (encInfo->src_image_fptr == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return failure;
    }

    encInfo->secret_fptr = fopen(encInfo->secret_fname, "r");

    if (encInfo->secret_fptr == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return failure;
    }

    encInfo->output_image_fptr = fopen(encInfo->output_image_fname, "w");

    if (encInfo->output_image_fptr == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->output_image_fname);

    	return failure;
    }

    return success;
}


// This Function to calculate the size of a file in bytes
// Moves the file pointer to the end, gets the position using ftell(), and returns the file size
unsigned int get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    unsigned int file_size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    return file_size;
}


/* This function checks whether the source image, secret file, 
   and output file are valid or not. 
 * It also verifies if the files exist and stores 
   the valid file names in the EncodeInfo structure for further processing.
*/

Status validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Source file validation
    char *bmp_extension = strstr(argv[2], ".bmp");

    if(bmp_extension == NULL)
    {
        printf("INFO : Source image file must be a .bmp file\n");
        return failure;
    }

    FILE *source_file = fopen(argv[2], "r");

    if(source_file == NULL)
    {
        printf("INFO : Source image file does not exist\n");
        return failure;
    }

    encInfo->src_image_fname = argv[2];


    // Secret file validation
    char *allowed_extensions[] = {".txt", ".c", ".cpp", ".sh"};

    int extension_count = 4;
    int is_valid_extension = 0;

    for(int index = 0; index < extension_count; index++)
    {
        if(strstr(argv[3], allowed_extensions[index]) != NULL)
        {
            strcpy(encInfo->secret_extn, allowed_extensions[index]);

            is_valid_extension = 1;
            break;
        }
    }

    if(!is_valid_extension)
    {
        printf("INFO : Secret file must be .txt, .c, .cpp or .sh\n");
        return failure;
    }

    encInfo->secret_fname = argv[3];

    FILE *secret_file = fopen(argv[3], "r");

    if(secret_file == NULL)
    {
        printf("INFO : Secret file does not exist\n");
        return failure;
    }

    // Output file validation
    if(argv[4] != NULL)
    {
        char *output_extension = strstr(argv[4], ".bmp");

        if(output_extension == NULL)
        {
            printf("INFO : Output file must be a .bmp file\n");
            return failure;
        }

        // Store output filename directly
        encInfo->output_image_fname = argv[4];
    }
    else
    {
        // Default output filename
        encInfo->output_image_fname = "Output.bmp";
    }

    return success;
   
}


// This function checks whether the source image has enough capacity
// to store all the secret data during encoding.

Status check_capacity(EncodeInfo *encInfo)
{
    unsigned int image_capacity = get_image_size_for_bmp(encInfo->src_image_fptr);

    encInfo->secret_file_size = get_file_size(encInfo->secret_fptr);

    unsigned long long int required_encoding_size =
    (strlen(MAGIC_STRING) + 4 + strlen(encInfo->secret_extn) + 4 + encInfo->secret_file_size) * 8;

    if(image_capacity > required_encoding_size)
    {
        printf("INFO : Source image has enough capacity for encoding\n");

        return success;
    }
    else
    {
        printf("INFO : Source image does not have enough capacity for encoding\n");

        return failure;
    }
}


// This function copies the first 54 bytes of the BMP image
// from the source file to the output file

Status copy_bmp_header(EncodeInfo *encInfo)
{
    unsigned char bmp_header[54];

    fseek(encInfo->src_image_fptr, 0, SEEK_SET);

    fread(bmp_header, 54, sizeof(unsigned char), encInfo->src_image_fptr);

    fwrite(bmp_header, 54, sizeof(unsigned char), encInfo->output_image_fptr);

    return success;
}

// This function hides one character of secret data
// into the LSB bits of 8 image bytes

Status encode_1byte_to_lsb(char secret_character, char *image_buffer)
{
    for(int bit_index = 0; bit_index < 8; bit_index++)
    {
        // Clear LSB bit
        image_buffer[bit_index] = image_buffer[bit_index] & ~(1 << 0);

        int extracted_bit = (secret_character >> (7 - bit_index)) & 1;

        // Set secret bit into LSB
        image_buffer[bit_index] = image_buffer[bit_index] | extracted_bit;
    }


    return success;
}

// This function encodes the magic string into the image
// by hiding each character inside the LSB bits of image bytes

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    int magic_string_length = strlen(magic_string);

    for(int char_index = 0; char_index < magic_string_length; char_index++)
    {
        char image_buffer[8];

        fread(image_buffer, sizeof(unsigned char),8,encInfo->src_image_fptr);

        encode_1byte_to_lsb(magic_string[char_index],image_buffer);

        fwrite(image_buffer, sizeof(unsigned char), 8, encInfo->output_image_fptr);
    }


    return success;
}

// This function encodes the secret file extension and its length
// into the image by hiding the data inside LSB bits

Status encode_secret_file_extn(EncodeInfo *encInfo)
{
    // Get secret file extension
    char *secret_extension = encInfo->secret_extn;

    // Find extension length
    int extension_length = strlen(secret_extension);

    // Store extension length
    encInfo->secret_extn_size = extension_length;

    // Encode extension size
    char extension_size_buffer[32];

    fread(extension_size_buffer, sizeof(unsigned char), 32, encInfo->src_image_fptr);

    encode_4byte_to_lsb(extension_length, extension_size_buffer);

    fwrite(extension_size_buffer, sizeof(unsigned char), 32, encInfo->output_image_fptr);

    // Encode extension characters
    for(int extension_index = 0; extension_index < extension_length; extension_index++)
    {
        char image_buffer[8];

        fread(image_buffer, sizeof(unsigned char), 8, encInfo->src_image_fptr);

        encode_1byte_to_lsb(secret_extension[extension_index], image_buffer);

        fwrite(image_buffer, sizeof(unsigned char), 8, encInfo->output_image_fptr);
    }


    return success;
}

/* This function encodes a 4-byte integer value into the image
   by storing each bit inside the LSB of 32 image bytes using 
   bitwise operations
*/

Status encode_4byte_to_lsb(int secret_data, char *image_buffer)
{
    for(int bit_index = 0; bit_index < 32; bit_index++)
    {
        // Clear LSB bit
        image_buffer[bit_index] = image_buffer[bit_index] & ~(1 << 0);

        // Extract each bit from integer data
        int extracted_bit = (secret_data >> (31 - bit_index)) & 1;

        // Store bit into image byte LSB
        image_buffer[bit_index] = image_buffer[bit_index] | extracted_bit;
    }

  

    return success;
}


// This function encodes the secret file size into the image
// by hiding the 4-byte integer value inside LSB bits

Status encode_secret_file_size(EncodeInfo *encInfo)
{
    char image_buffer[32];

    // Read 32 bytes from source image
    fread(image_buffer, 1, 32, encInfo->src_image_fptr);

    // Encode secret file size into image bytes
    encode_4byte_to_lsb(encInfo->secret_file_size, image_buffer);

    // Write -> modified bytes into output image
    fwrite(image_buffer, 1, 32, encInfo->output_image_fptr);


    return success;
}

// This function encodes the complete secret file data into the image
// by reading one byte from the secret file at a time and hiding
// its 8 bits inside the least significant bits (LSB) of 8 image bytes

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    for(int byte_index = 0; byte_index < encInfo->secret_file_size; byte_index++)
    {
        // 8 image bytes are needed to encode 1 secret byte
        char image_buffer[8];

        // Variable to store one byte from secret file
        char secret_character;

        // Read 8 bytes from source image
        fread(image_buffer, sizeof(unsigned char), 8, encInfo->src_image_fptr);

        // Read 1 byte from secret file
        fread(&secret_character, sizeof(unsigned char), 1, encInfo->secret_fptr);

        // Encode secret byte into image bytes
        encode_1byte_to_lsb(secret_character, image_buffer);

        // Write encoded bytes into output image
        fwrite(image_buffer, sizeof(unsigned char), 8, encInfo->output_image_fptr);
    }



    return success;
}

// This function copies all the remaining image data
// from the source image file to the output image file

Status copy_remaining_img_data(EncodeInfo *encInfo)
{
    char image_byte;

    while(fread(&image_byte,
                sizeof(unsigned char),
                1,
                encInfo->src_image_fptr) == 1)
    {
        fwrite(&image_byte,
               sizeof(unsigned char),
               1,
               encInfo->output_image_fptr);
    }

    
    return success;
}

// This function controls the complete encoding process step by step,
// including opening files, checking image capacity, encoding secret data,
// and creating the final output stego image

Status do_encoding(char *argv[], EncodeInfo *encInfo)
{
    printf("INFO : Opening required files\n");

    if(open_files(encInfo) != success)
    {
        printf("INFO : \033[1;31mFailure\033[0m : Failed to open required files\n");
        return failure;
    }

    printf("INFO : Files opened successfully\n");


    printf("INFO : Checking image capacity\n");

    if(check_capacity(encInfo) != success)
    {
        printf("INFO : \033[1;31mFailure\033[0m : Image does not have enough capacity\n");
        return failure;
    }

    printf("INFO : Image capacity check completed\n");


    printf("INFO : Copying BMP header\n");

    if(copy_bmp_header(encInfo) != success)
    {
        printf("INFO : \033[1;31mFailure\033[0m : Failed to copy BMP header\n");
        return failure;
    }

    printf("INFO : BMP header copied successfully\n");


    printf("INFO : Encoding magic string\n");

    if(encode_magic_string(MAGIC_STRING, encInfo) != success)
    {
        printf("INFO : \033[1;31mFailure\033[0m : Failed to encode magic string\n");
        return failure;
    }

    printf("INFO : Magic string encoded successfully\n");


    printf("INFO : Encoding secret file extension\n");

    if(encode_secret_file_extn(encInfo) != success)
    {
        printf("INFO : \033[1;31mFailure\033[0m : Failed to encode secret file extension\n");
        return failure;
    }

    printf("INFO : Secret file extension encoded successfully\n");


    printf("INFO : Encoding secret file size\n");

    if(encode_secret_file_size(encInfo) != success)
    {
        printf("INFO : \033[1;31mFailure\033[0m : Failed to encode secret file size\n");
        return failure;
    }

    printf("INFO : Secret file size encoded successfully\n");


    printf("INFO : Encoding secret file data\n");

    if(encode_secret_file_data(encInfo) != success)
    {
        printf("INFO : \033[1;31mFailure\033[0m : Failed to encode secret file data\n");
        return failure;
    }

    printf("INFO : Secret file data encoded successfully\n");


    printf("INFO : Copying remaining image data\n");

    if(copy_remaining_img_data(encInfo) != success)
    {
        printf("INFO : \033[1;31mFailure\033[0m : Failed to copy remaining image data\n");
        return failure;
    }

    printf("INFO : Remaining image data copied successfully\n");

    printf("INFO : \033[1;32mSuccess\033[0m : Encoding completed successfully\n");

    return success;
}