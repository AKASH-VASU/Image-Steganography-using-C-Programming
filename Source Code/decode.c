#include <stdio.h>
#include <string.h>

#include "decode.h"



/* This function opens the stego image file and output secret file
   required for the decoding process */
Status open_img_file(DecodeInfo *decodeInfo)
{
    /* Open stego image file in read mode */
    decodeInfo->stego_image_file_pointer =
    fopen(decodeInfo->stego_image_filename, "r");

    /* Check whether stego image opened successfully */
    if(decodeInfo->stego_image_file_pointer == NULL)
    {
        printf("INFO: \033[1;31mFailure\033[0m : Unable to open stego image file %s\n",
               decodeInfo->stego_image_filename);

        return failure;
    }

    return success;
}



/* This function validates the command line arguments given
   for the decoding operation */
Status validate_decode_args(char *argv[], DecodeInfo *decodeInfo)
{
    printf("INFO: Validating decode arguments\n");

    /* Check whether stego image is BMP format */
    char *bmp_extension = strstr(argv[2], ".bmp");

    /* If .bmp extension not found */
    if(bmp_extension == NULL)
    {
        printf("INFO: \033[1;31mFailure\033[0m : Stego image must be a .bmp file\n");

        return failure;
    }

    /* Open stego image to check file existence */
    FILE *stego_image_file = fopen(argv[2], "r");

    /* Check whether stego image exists */
    if(stego_image_file == NULL)
    {
        printf("INFO: \033[1;31mFailure\033[0m : Stego image file does not exist\n");

        return failure;
    }

    /* Close validation file pointer */
    fclose(stego_image_file);

    /* Store stego image filename in structure */
    decodeInfo->stego_image_filename = argv[2];

    /* Check whether output filename is given */
    if(argv[3] != NULL)
    {
        /* Store output filename */
        strcpy(decodeInfo->secret_output_filename, argv[3]);
    }
    else
    {
        /* Default output filename */
        strcpy(decodeInfo->secret_output_filename,
               "decoded_secret");
    }

    printf("INFO: Done\n");

    return success;
}



/* This function extracts one character from the least significant
   bits of 8 image bytes */
char decode_1byte_from_lsb(char *image_buffer)
{
    /* Variable to store decoded character */
    char decoded_character = 0;

    /* Loop through 8 image bytes */
    for(int bit_index = 0; bit_index < 8; bit_index++)
    {
        /* Extract LSB bit */
        int extracted_bit = image_buffer[bit_index] & 1;

        /* Left shift decoded character */
        decoded_character = decoded_character << 1;

        /* Store extracted bit */
        decoded_character = decoded_character | extracted_bit;
    }

    /* Return decoded character */
    return decoded_character;
}



/* This function extracts a 4-byte integer value from the least
   significant bits of 32 image bytes */
int decode_4bytes_from_lsb(char *image_buffer_32)
{
    /* Variable to store decoded integer */
    int decoded_data = 0;

    /* Loop through 32 image bytes */
    for(int bit_index = 0; bit_index < 32; bit_index++)
    {
        /* Left shift integer data */
        decoded_data = decoded_data << 1;

        /* Extract LSB bit */
        int extracted_bit = image_buffer_32[bit_index] & 1;

        /* Store extracted bit */
        decoded_data = decoded_data | extracted_bit;
    }

    /* Return decoded integer */
    return decoded_data;
}



/* This function decodes the magic string from the stego image
   and verifies whether the image contains hidden data */
Status decode_magic_string(DecodeInfo *decodeInfo)
{
    printf("INFO: Decoding Magic String Signature\n");

    /* Find magic string length */
    int magic_string_length = strlen(MAGIC_STRING);

    /* Array to store decoded magic string */
    char decoded_magic_string[magic_string_length + 1];

    /* Buffer to store 8 image bytes */
    char image_buffer[8];

    /* Move file pointer after BMP header */
    fseek(decodeInfo->stego_image_file_pointer,
          54,
          SEEK_SET);

    /* Decode each character of magic string */
    for(int character_index = 0;
        character_index < magic_string_length;
        character_index++)
    {
        /* Read 8 image bytes */
        fread(image_buffer,
              sizeof(unsigned char),
              8,
              decodeInfo->stego_image_file_pointer);

        /* Decode one character */
        decoded_magic_string[character_index] =
        decode_1byte_from_lsb(image_buffer);
    }

    /* Add null character at end */
    decoded_magic_string[magic_string_length] = '\0';

    /* Compare decoded string with original magic string */
    if(strcmp(decoded_magic_string, MAGIC_STRING) == 0)
    {
        printf("INFO: Done\n");

        return success;
    }
    else
    {
        printf("INFO: \033[1;31mFailure\033[0m : Magic string mismatch\n");

        return failure;
    }
}



/* This function decodes the secret file extension from the image
   and creates the correct output file name */
Status decode_secret_file_extn(char *argv[],
                               DecodeInfo *decodeInfo)
{
    printf("INFO: Decoding Secret File Extension Size\n");

    /* Buffer to store 32 image bytes */
    char image_buffer_32[32];

    /* Read 32 image bytes */
    fread(image_buffer_32,
          sizeof(unsigned char),
          32,
          decodeInfo->stego_image_file_pointer);

    /* Decode extension size */
    int extension_size =
    decode_4bytes_from_lsb(image_buffer_32);

    printf("INFO: Done\n");

    printf("INFO: Decoding Secret File Extension\n");

    /* Array to store decoded extension */
    char decoded_extension[extension_size + 1];

    /* Decode extension characters */
    for(int extension_index = 0;
        extension_index < extension_size;
        extension_index++)
    {
        /* Buffer to store 8 image bytes */
        char image_buffer[8];

        /* Read 8 image bytes */
        fread(image_buffer,
              sizeof(unsigned char),
              8,
              decodeInfo->stego_image_file_pointer);

        /* Decode one extension character */
        decoded_extension[extension_index] =
        decode_1byte_from_lsb(image_buffer);
    }

    /* Add null character at end */
    decoded_extension[extension_size] = '\0';

    /* Store extension in structure */
    strcpy(decodeInfo->secret_file_extension,
           decoded_extension);

    /* Update output filename with decoded extension */
    update_output_filename(decodeInfo);

    /* Open output secret file */
    decodeInfo->secret_output_file_pointer =
    fopen(decodeInfo->secret_output_filename,
          "w");

    /* Check whether output file created successfully */
    if(decodeInfo->secret_output_file_pointer == NULL)
    {
        printf("INFO: \033[1;31mFailure\033[0m : Unable to create output secret file\n");

        return failure;
    }

    printf("INFO: Done\n");

    return success;
}



/* This function decodes the secret file size stored inside
   the stego image */
Status decode_secret_file_size(DecodeInfo *decodeInfo)
{
    printf("INFO: Decoding Secret File Size\n");

    /* Buffer to store 32 image bytes */
    char image_buffer_32[32];

    /* Read 32 image bytes */
    fread(image_buffer_32,
          sizeof(char),
          32,
          decodeInfo->stego_image_file_pointer);

    /* Decode secret file size */
    decodeInfo->secret_data_size =
    decode_4bytes_from_lsb(image_buffer_32);

    printf("INFO: Done\n");

    printf("INFO: Secret File Size = %d bytes\n",
           decodeInfo->secret_data_size);

    return success;
}



/* This function decodes the complete secret file data from
   the stego image and writes it into the output file */
Status decode_secret_file_data(DecodeInfo *decodeInfo)
{
    printf("INFO: Decoding Secret File Data\n");

    /* Loop through complete secret data */
    for(int byte_index = 0;
        byte_index < decodeInfo->secret_data_size;
        byte_index++)
    {
        /* Buffer to store 8 image bytes */
        char image_buffer[8];

        /* Variable to store decoded character */
        char decoded_character;

        /* Read 8 image bytes from stego image */
        fread(image_buffer,
              sizeof(unsigned char),
              8,
              decodeInfo->stego_image_file_pointer);

        /* Decode one character from LSB bits */
        decoded_character =
        decode_1byte_from_lsb(image_buffer);

        /* Write decoded character into output file */
        fwrite(&decoded_character,
               sizeof(unsigned char),
               1,
               decodeInfo->secret_output_file_pointer);
    }

    printf("INFO: Done\n");

    return success;
}



/* This function manages the entire decoding operation */
Status do_decoding(char *argv[], DecodeInfo *decodeInfo)
{
    printf("INFO: ## Decoding Procedure Started ##\n");

    /* Opening files */
    printf("INFO: Opening required files\n");

    if(open_img_file(decodeInfo) != success)
    {
        printf("INFO: \033[1;31mFailure\033[0m : Opening files failed\n");

        return failure;
    }

    printf("INFO: Done\n");

    /* Decode magic string */
    if(decode_magic_string(decodeInfo) != success)
    {
        printf("INFO: \033[1;31mFailure\033[0m : Decoding magic string failed\n");

        return failure;
    }

    /* Decode secret file extension */
    if(decode_secret_file_extn(argv,
                               decodeInfo) != success)
    {
        printf("INFO: \033[1;31mFailure\033[0m : Decoding secret file extension failed\n");

        return failure;
    }

    /* Decode secret file size */
    if(decode_secret_file_size(decodeInfo) != success)
    {
        printf("INFO: \033[1;31mFailure\033[0m : Decoding secret file size failed\n");

        return failure;
    }

    /* Decode secret file data */
    if(decode_secret_file_data(decodeInfo) != success)
    {
        printf("INFO: \033[1;31mFailure\033[0m : Decoding secret file data failed\n");

        return failure;
    }

    printf("INFO: ## Decoding Done Successfully ##\n");

    return success;
}



/* This function removes the old extension from the output
   filename and appends the decoded secret file extension */
Status update_output_filename(DecodeInfo *decodeInfo)
{
    /* Variable to traverse filename */
    int filename_index = 0;

    /* Remove old extension if present */
    while(decodeInfo->secret_output_filename[filename_index] != '\0')
    {
        /* Check for dot symbol */
        if(decodeInfo->secret_output_filename[filename_index] == '.')
        {
            /* Remove old extension */
            decodeInfo->secret_output_filename[filename_index] = '\0';

            break;
        }

        filename_index++;
    }

    /* Append decoded secret extension */
    strcat(decodeInfo->secret_output_filename,
           decodeInfo->secret_file_extension);

    printf("INFO: Output File Name = %s\n",
           decodeInfo->secret_output_filename);

    return success;
}