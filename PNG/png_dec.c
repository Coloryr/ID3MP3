#include "png_dec.h"
#include "upng.h"
#include <stdio.h>
#include <malloc.h>
#include <stdint.h>

#define PNG_PATH "F:\\c_Project\\upng\\codeblocks\\testpng\\testpng.png"
#define BMP_PATH "F:\\c_Project\\upng\\codeblocks\\out.bmp"

#define HI(w) (((w) >> 8) & 0xFF)
#define LO(w) ((w) & 0xFF)

static uint8_t decoder_buffer[1000*1024];

void encodeToBMP(uint8_t *bmp_header, uint32_t xsize, uint32_t ysize)
{
	uint8_t header[54] =
	{
    	0x42, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    uint32_t file_size = (uint32_t)xsize * (uint32_t)ysize * 3 + 54;

    header[2] = (unsigned char)(file_size &0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    uint32_t width = xsize;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    uint32_t height = ysize;
    header[22] = height & 0x000000ff;
    header[23] = (height >> 8) & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

	memmove(bmp_header, header, sizeof(header));
}


int main()
{
	FILE* fh;
	upng_t* upng;
	unsigned width, height, depth;
	unsigned x, y, d;

	upng = upng_new_from_file(PNG_PATH);
    if(upng != NULL)
    {
//        upng_decode(upng);
        upng_decode_to_buffer(upng, (unsigned char *)decoder_buffer, sizeof(decoder_buffer));
	    if (upng_get_error(upng) != UPNG_EOK) 
        {
		    printf("error: %u %u\n", upng_get_error(upng), upng_get_error_line(upng));
    		return 0;
    	}
    }

	width = upng_get_width(upng);
	height = upng_get_height(upng);
	depth = upng_get_bpp(upng) / 8;

	printf("size:	%ux%ux%u (%u)\n", width, height, upng_get_bpp(upng), upng_get_size(upng));
	printf("format:	%u\n", upng_get_format(upng));

	if (upng_get_format(upng) == UPNG_RGB8 || upng_get_format(upng) == UPNG_RGBA8) 
    {
		fh = fopen(BMP_PATH, "wb");

        uint8_t header[54];
        encodeToBMP(header, width, height);
    
        fwrite(header, 1, 54, fh);
        
		for (y = 0; y != height; ++y) 
        {
			for (x = 0; x != width; ++x) 
            {
				for (d = 0; d != depth; ++d) 
                {
					putc(upng_get_buffer(upng)[(height - y - 1) * width * depth + x * depth + (depth - d - 1)], fh);
				}
			}
		}

		fclose(fh);
	}

	upng_free(upng);
	return 0;
}

