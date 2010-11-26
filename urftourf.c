/**
 * This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @brief Decode URF  to a PDF file
 * @file urf_decode.c
 * @author Neil 'Superna' Armstrong <superna9999@gmail.com> (C) 2010
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "unirast.h"

#define PROGRAM "urftopdf"

#ifdef URF_DEBUG
#define dprintf(format, ...) fprintf(stderr, "DEBUG: (" PROGRAM ") " format, __VA_ARGS__)
#else
#define dprintf(format, ...)
#endif

#define iprintf(format, ...) fprintf(stderr, "INFO: (" PROGRAM ") " format, __VA_ARGS__)

void die(char * str)
{
    fprintf(stderr, "CRIT: (" PROGRAM ") die(%s) [%m]\n", str);
    exit(1);
}

// Data are in network endianness
struct urf_file_header {
    char unirast[8];
    uint32_t page_count;
} __attribute__((__packed__));

struct urf_page_header {
    uint8_t bpp;
    uint8_t colorspace;
    uint8_t duplex;
    uint8_t quality;
    uint32_t unknown0;
    uint32_t unknown1;
    uint32_t width;
    uint32_t height;
    uint32_t dot_per_inch;
    uint32_t unknown2;
    uint32_t unknown3;
} __attribute__((__packed__));

void rgb2cmyk32(uint8_t rgb[], uint8_t /*float*/ cmyk[]) {
    int r = rgb[0];
    int g = rgb[1];
    int b = rgb[2];
    float computedC = 0;
    float computedM = 0;
    float computedY = 0;
    float computedK = 0;

    // BLACK
    if (r==0 && g==0 && b==0) {
        cmyk[0] = 0;
        cmyk[1] = 0;
        cmyk[2] = 0;
        cmyk[3] = 255;
        return;
    }

    computedC = 1 - ((float)r/255);
    computedM = 1 - ((float)g/255);
    computedY = 1 - ((float)b/255);

    float minMY = (computedM < computedY?computedM:computedY);
    float minCMY = (computedC<minMY?computedC:minMY);

    computedC = (computedC - minCMY) / (1.0 - minCMY) ;
    computedM = (computedM - minCMY) / (1.0 - minCMY) ;
    computedY = (computedY - minCMY) / (1.0 - minCMY) ;
    computedK = minCMY;

    cmyk[0] = computedC*255;
    cmyk[1] = computedM*255;
    cmyk[2] = computedY*255;
    cmyk[3] = computedK*255;

    /*printf("RGB %d-%d-%d CMYK %f-%f-%f-%f\n",
            r, g, b, computedC, computedM, computedY, computedK);*/

    return;
}

void rgb2cmyk64(uint8_t rgb[], uint16_t cmyk[]) {
    int r = rgb[0];
    int g = rgb[1];
    int b = rgb[2];
    float computedC = 0;
    float computedM = 0;
    float computedY = 0;
    float computedK = 0;

    // BLACK
    if (r==0 && g==0 && b==0) {
        cmyk[0] = 0;
        cmyk[1] = 0;
        cmyk[2] = 0;
        cmyk[3] = (65535);
        return;
    }

    computedC = 1 - ((float)r/255);
    computedM = 1 - ((float)g/255);
    computedY = 1 - ((float)b/255);

    float minMY = (computedM < computedY?computedM:computedY);
    float minCMY = (computedC<minMY?computedC:minMY);

    computedC = (computedC - minCMY) / (1.0 - minCMY) ;
    computedM = (computedM - minCMY) / (1.0 - minCMY) ;
    computedY = (computedY - minCMY) / (1.0 - minCMY) ;
    computedK = minCMY;

    cmyk[0] = (computedC*65535);
    cmyk[1] = (computedM*65535);
    cmyk[2] = (computedY*65535);
    cmyk[3] = (computedK*65535);

    /*printf("RGB %d-%d-%d CMYK %f-%f-%f-%f\n",
            r, g, b, computedC, computedM, computedY, computedK);*/

    return;
}

int convert_and_write_pixel(int fd_dest, int bpp, int dest_bpp, int dest_cspace, uint8_t pixel_container[])
{
    //static 24bit rgb source
    if(dest_cspace != UNIRAST_COLOR_SPACE_CMYK_32BIT_64BIT)
    {
        switch(dest_bpp)
        {
            case 8:
            {
                uint8_t c = ((int)pixel_container[0] + (int)pixel_container[1] + (int)pixel_container[2])/3;
                write(fd_dest, &c, 1);
            } break;

            case 24:
                write(fd_dest, pixel_container, 3);
                break;

            case 32:
            {
                if(dest_cspace == UNIRAST_COLOR_SPACE_SRGB_32BIT)
                {
                    uint8_t c = 0; //alpha
                    write(fd_dest, pixel_container, 3);
                    write(fd_dest, &c, 1);
                }
                else if(dest_cspace == UNIRAST_COLOR_SPACE_GRAYSCALE_32BIT)
                {
                    uint32_t c = htonl(((int)pixel_container[0]*16843009 + (int)pixel_container[1]*16843009 + (int)pixel_container[2]*16843009)/3);
                    write(fd_dest, &c, 4);
                }
            } break;
        };
    }
    else if(dest_bpp == 32)
    {
        uint8_t /*float*/ cmyk[4];
        rgb2cmyk32(pixel_container, cmyk);
        write(fd_dest, cmyk, 4);
    }
    else if(dest_bpp == 64)
    {
        uint16_t cmyk[4];
        rgb2cmyk64(pixel_container, cmyk);
        write(fd_dest, cmyk, 8);
    }

    return 0;
}

int decode_raster(int fd, int width, int height, int bpp, int dest_bpp, int dest_cspace, int fd_dest)
{
    // We should be at raster start
    int i, j;
    int cur_line = 0;
    int pos = 0;
    uint8_t line_repeat_byte = 0;
    unsigned line_repeat = 0;
    int8_t packbit_code = 0;
    int pixel_size = (bpp/8);
    uint8_t * pixel_container;
    uint8_t * line_container;

    pixel_container = malloc(pixel_size);
    line_container = malloc(pixel_size*width);

    do
    {
        if(read(fd, &line_repeat_byte, 1) < 1)
        {
            dprintf("l%06d : line_repeat EOF at %lu\n", cur_line, lseek(fd, 0, SEEK_CUR));
            return 1;
        }
        write(fd_dest, &line_repeat_byte, 1);

        line_repeat = (unsigned)line_repeat_byte + 1;

        dprintf("l%06d : next actions for %d lines\n", cur_line, line_repeat);

        // Start of line
        pos = 0;

        do
        {
            if(read(fd, &packbit_code, 1) < 1)
            {
                dprintf("p%06dl%06d : packbit_code EOF at %lu\n", pos, cur_line, lseek(fd, 0, SEEK_CUR));
                return 1;
            }
            write(fd_dest, &packbit_code, 1);

            dprintf("p%06dl%06d: Raster code %02X='%d'.\n", pos, cur_line, (uint8_t)packbit_code, packbit_code);

            if(packbit_code == -128)
            {
                dprintf("\tp%06dl%06d : blank rest of line.\n", pos, cur_line);
                memset((line_container+(pos*pixel_size)), 0xFF, (pixel_size*(width-pos)));
                pos = width;
                break;
            }
            else if(packbit_code >= 0 && packbit_code <= 127)
            {
                int n = (packbit_code+1);

                //Read pixel
                if(read(fd, pixel_container, pixel_size) < pixel_size)
                {
                    dprintf("p%06dl%06d : pixel repeat EOF at %lu\n", pos, cur_line, lseek(fd, 0, SEEK_CUR));
                    return 1;
                }

                convert_and_write_pixel(fd_dest, bpp, dest_bpp, dest_cspace, pixel_container);

                dprintf("\tp%06dl%06d : Repeat pixel '", pos, cur_line);
                for(j = 0 ; j < pixel_size ; ++j)
                    dprintf("%02X ", pixel_container[j]);
                dprintf("' for %d times.\n", n);

                for(i = 0 ; i < n ; ++i)
                {
                    //for(j = pixel_size-1 ; j >= 0 ; --j)
                    for(j = 0 ; j < pixel_size ; ++j)
                        line_container[pixel_size*pos + j] = pixel_container[j];
                    ++pos;
                    if(pos >= width)
                        break;
                }

                if(i < n && pos >= width)
                {
                    dprintf("\tp%06dl%06d : Forced end of line for pixel repeat.\n", pos, cur_line);
                }
                
                if(pos >= width)
                    break;
            }
            else if(packbit_code > -128 && packbit_code < 0)
            {
                int n = (-(int)packbit_code)+1;

                dprintf("\tp%06dl%06d : Copy %d verbatim pixels.\n", pos, cur_line, n);

                for(i = 0 ; i < n ; ++i)
                {
                    if(read(fd, pixel_container, pixel_size) < pixel_size)
                    {
                        dprintf("p%06dl%06d : literal_pixel EOF at %lu\n", pos, cur_line, lseek(fd, 0, SEEK_CUR));
                        return 1;
                    }
                    convert_and_write_pixel(fd_dest, bpp, dest_bpp, dest_cspace, pixel_container);
                    //Invert pixels, should be programmable
                    for(j = 0 ; j < pixel_size ; ++j)
                        line_container[pixel_size*pos + j] = pixel_container[j];
                    ++pos;
                    if(pos >= width)
                        break;
                }

                if(i < n && pos >= width)
                {
                    dprintf("\tp%06dl%06d : Forced end of line for pixel copy.\n", pos, cur_line);
                }
                
                if(pos >= width)
                    break;
            }
        }
        while(pos < width);

        dprintf("\tl%06d : End Of line, drawing %d times.\n", cur_line, line_repeat);

        // write lines
        for(i = 0 ; i < line_repeat ; ++i)
        {
            ++cur_line;
        }
    }
    while(cur_line < height);

    return 0;
}

int main(int argc, char **argv)
{
    int fd, page, fd_dest;
    struct urf_file_header head, head_orig, head_dest;
    struct urf_page_header page_header, page_header_orig, page_header_dest;

    FILE * input = NULL;

    if(argc < 5)
    {
        fprintf(stderr, "Usage: %s <dest colorspace> <dest bpp> [src] [dest]\n", argv[0]);
        return 1;
    }

    input = fopen(argv[3], "rb");
    if(input == NULL) die("Unable to open unirast file");

    fd_dest = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0666);

    // Get fd from file
    fd = fileno(input);

    if(read(fd, &head_orig, sizeof(head)) == -1) die("Unable to read file header");

    //Transform
    memcpy(head.unirast, head_orig.unirast, sizeof(head.unirast));
    head.page_count = ntohl(head_orig.page_count);

    if(head.unirast[7])
        head.unirast[7] = 0;

    if(strncmp(head.unirast, "UNIRAST", 7) != 0) die("Bad File Header");

    iprintf("%s file, with %d page(s).\n", head.unirast, head.page_count);

    memcpy(&head_dest, &head_orig, sizeof(head_orig));
    write(fd_dest, &head_dest, sizeof(head_orig));

    for(page = 0 ; page < head.page_count ; ++page)
    {
        if(read(fd, &page_header_orig, sizeof(page_header_orig)) == -1) die("Unable to read page header");

        //Transform
        page_header.bpp = page_header_orig.bpp;
        page_header.colorspace = page_header_orig.colorspace;
        page_header.duplex = page_header_orig.duplex;
        page_header.quality = page_header_orig.quality;
        page_header.unknown0 = 0;
        page_header.unknown1 = 0;
        page_header.width = ntohl(page_header_orig.width);
        page_header.height = ntohl(page_header_orig.height);
        page_header.dot_per_inch = ntohl(page_header_orig.dot_per_inch);
        page_header.unknown2 = 0;
        page_header.unknown3 = 0;

        iprintf("Page %d :\n", page);
        iprintf("Bits Per Pixel : %d\n", page_header.bpp);
        iprintf("Dest Bits Per Pixel : %d\n", atoi(argv[2]));
        iprintf("Colorspace : %d\n", page_header.colorspace);
        iprintf("Dest Colorspace : %d\n", atoi(argv[1]));
        iprintf("Duplex Mode : %d\n", page_header.duplex);
        iprintf("Quality : %d\n", page_header.quality);
        iprintf("Size : %dx%d pixels\n", page_header.width, page_header.height);
        iprintf("Dots per Inches : %d\n", page_header.dot_per_inch);

        if(page_header.colorspace != UNIRAST_COLOR_SPACE_SRGB_24BIT_1)
        {
            die("Invalid ColorSpace, only RGB 24BIT type 1 is supported");
        }
        
        if(page_header.bpp != UNIRAST_BPP_24BIT)
        {
            die("Invalid Bit Per Pixel value, only 24bit is supported");
        }

        memcpy(&page_header_dest, &page_header_orig, sizeof(page_header_orig));
        page_header_dest.bpp = atoi(argv[2]);
        page_header_dest.colorspace = atoi(argv[1]);
        write(fd_dest, &page_header_dest, sizeof(page_header_orig));

        if(decode_raster(fd, page_header.width, page_header.height, page_header.bpp, page_header_dest.bpp, page_header_dest.colorspace, fd_dest) != 0)
            die("Failed to decode Page");
    }

    close(fd_dest);

    return 0;
}
