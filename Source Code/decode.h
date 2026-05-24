#ifndef DECODE_H
#define DECODE_H

#include "common.h"

typedef struct DecodeInfo
{
    char *stego_image_filename;
    char secret_output_filename[100];

    FILE *stego_image_file_pointer;
    FILE *secret_output_file_pointer;
    
    char secret_file_extension[10];
    int secret_extension_size;
    int secret_data_size;

} DecodeInfo;


// This function validates decoding command line arguments
Status validate_decode_args(char *argv[], DecodeInfo *decodeInfo);

// This function controls the complete decoding process
Status do_decoding(char *argv[], DecodeInfo *decodeInfo);

// This function opens the stego image file
Status open_img_file(DecodeInfo *decodeInfo);

// This function decodes and verifies the magic string
Status decode_magic_string(DecodeInfo *decodeInfo);

// This function decodes the secret file extension
Status decode_secret_file_extn(char *argv[], DecodeInfo *decodeInfo);

// This function decodes the secret file size
Status decode_secret_file_size(DecodeInfo *decodeInfo);

// This function decodes the secret file data
Status decode_secret_file_data(DecodeInfo *decodeInfo);

// This function extracts one byte from LSB bits of image bytes
char decode_1byte_from_lsb(char *image_buffer);

// This function extracts a 4-byte integer from LSB bits
int decode_4bytes_from_lsb(char *image_buffer_32);

// This function updates output filename with decoded extension
Status update_output_filename(DecodeInfo *decodeInfo);

#endif