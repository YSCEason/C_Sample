#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* image size */
#define IMG_WIDTH   1200
#define IMG_HEIGHT  960

/* tg color bar output format */
typedef enum {
    FORMAT_16BIT = sizeof(uint16_t)
} MY_FORMAT;


/* tg buffer to store original tg color bar data */
typedef void *tg_img;
#define get_tg_buf(a)   (a)


/* YCC color bar definition */
#define BAR_NUMBER      8
#define Y_POS           0
#define Cb_POS          1
#define Cr_POS          2
const static YCC_COLOR_BAR[BAR_NUMBER][3] = {
    {77,   85, 255},
    {149,  43,  21},
    {29,  255, 107},
    {0,   128, 128},
    {255, 128, 128},
    {178, 171,   0},
    {106, 213, 235},
    {226,   0, 149},
};

/* The input format for SIF */
typedef enum {
    OUT_FORMAT_LSB_8BIT,
    OUT_FORMAT_MSB_8BIT,
    OUT_FORMAT_LSB_10BIT,
    OUT_FORMAT_MSB_10BIT,
    OUT_FORMAT_12BIT,
} OUTPUT_FORMAT;

#define GET_LSB_8B(a)       ((a) & 0x00FF)
#define GET_MSB_8B(a)       (((a) & 0x0FF0) >> 4)
#define GET_12B(a)          ((a) & 0x0FFF)
#define GET_LSB_10B(a)      ((a) & 0x03FF)
#define GET_MSB_10B(a)      (((a) & 0x0FFC) >> 2)

static int write_image_to_file(void *buf, uint16_t width, uint16_t height, uint8_t pixel_bytes, const char *filename)
{
    FILE *fp;

    fp = fopen(filename, "w");
    fwrite(buf, pixel_bytes, width * height, fp);
    fclose(fp);

    return 0;
}

static int fillin_color_bar(void *buf, uint16_t width, uint16_t height)
{
    char *img = (char*) buf;
    /* normal color bar width, for bar number 0 to number (BAR_NUMBER - 1) */
    uint16_t normal_width = width / BAR_NUMBER;
    /* for last color bar width */
    uint16_t last_width = (width / BAR_NUMBER) + (width % BAR_NUMBER);
    uint16_t bar_width;
    uint16_t i, j, k;
    uint32_t pixel_offset;

    for (j = 0; j < height; j++) {
        for (k = 0; k < BAR_NUMBER; k++) {
            if (k < BAR_NUMBER - 1)
                bar_width = normal_width;
            else
                bar_width = last_width;

            for (i = 0; i < bar_width; i++) {
                pixel_offset = (i + k*normal_width)*sizeof(uint16_t) + j*width*sizeof(uint16_t);
                *(img + pixel_offset) = YCC_COLOR_BAR[k][Y_POS];
                *(img + pixel_offset + 1) = (i%2 == 0 ? YCC_COLOR_BAR[k][Cb_POS] : YCC_COLOR_BAR[k][Cr_POS]);
            }
        }
    }
    return 0;
}
static int create_image_buffer(tg_img *img,  uint16_t width, uint16_t height, MY_FORMAT fmt)
{
    *img = calloc(width*height, fmt);

    fillin_color_bar(*img, width, height);

    return 0;
}

static int destroy_image(tg_img *img)
{
    free(*img);
    return 0;
}

static int output_image_file(void *buf, uint16_t width, uint16_t height, const char *filename, OUTPUT_FORMAT fmt)
{
    uint8_t *img_buf, *original_ptr;
    uint16_t *source_buf = buf;
    uint16_t i, j;
    uint8_t pixel_size;             //unit is byte

    if ((fmt == OUT_FORMAT_LSB_8BIT) || (fmt == OUT_FORMAT_MSB_8BIT))
        pixel_size = 1;
    else
        pixel_size = 2;

    img_buf = calloc(width * height, pixel_size);
    original_ptr = img_buf;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            switch (fmt)
            {
                case OUT_FORMAT_LSB_8BIT:
                    *(img_buf) = GET_LSB_8B(*(source_buf));
                break;
                case OUT_FORMAT_MSB_8BIT:
                    *(img_buf) = GET_MSB_8B(*(source_buf));
                break;
                case OUT_FORMAT_LSB_10BIT:
                    *(img_buf) = GET_LSB_10B(*(source_buf)) & 0xFF;
                    *(img_buf + 1) = (GET_LSB_10B(*(source_buf)) & 0xFF00) >> 8;
                break;
                case OUT_FORMAT_MSB_10BIT:
                    *(img_buf) = GET_MSB_10B(*(source_buf)) & 0xFF;
                    *(img_buf + 1) = (GET_MSB_10B(*(source_buf)) & 0xFF00) >> 8;
                break;
                case OUT_FORMAT_12BIT:
                    *(img_buf) = GET_12B(*(source_buf)) & 0xFF;
                    *(img_buf + 1) = (GET_12B(*(source_buf)) & 0xFF00) >> 8;
                break;
                default:
                break;
            }
            img_buf += pixel_size;
            source_buf++;
        }
    }

    write_image_to_file(original_ptr, width, height, pixel_size, filename);

    free(original_ptr);

    return 0;
}

static int output_lsb_10bit_image_file(tg_img *img, uint16_t width, uint16_t height, const char *filename)
{
    return output_image_file(*img, width, height, filename, OUT_FORMAT_LSB_10BIT);
}

static int output_msb_10bit_image_file(tg_img *img, uint16_t width, uint16_t height, const char *filename)
{
    return output_image_file(*img, width, height, filename, OUT_FORMAT_MSB_10BIT);
}

static int output_12bit_image_file(tg_img *img, uint16_t width, uint16_t height, const char *filename)
{
    return output_image_file(*img, width, height, filename, OUT_FORMAT_12BIT);
}

static int output_lsb_8bit_image_file(tg_img *img, uint16_t width, uint16_t height, const char *filename)
{
    return output_image_file(*img, width, height, filename, OUT_FORMAT_LSB_8BIT);
}

static int output_msb_8bit_image_file(tg_img *img, uint16_t width, uint16_t height, const char *filename)
{
    return output_image_file(*img, width, height, filename, OUT_FORMAT_MSB_8BIT);
}



int main(int argc, char *argv[])
{
    tg_img my_img;

    create_image_buffer(&my_img, IMG_WIDTH, IMG_HEIGHT, FORMAT_16BIT);

    write_image_to_file(get_tg_buf(my_img), IMG_WIDTH, IMG_HEIGHT, sizeof(uint16_t), "source_16bit.raw");
    output_msb_8bit_image_file(&my_img, IMG_WIDTH, IMG_HEIGHT, "msb_8bit.raw");
    output_lsb_8bit_image_file(&my_img, IMG_WIDTH, IMG_HEIGHT, "lsb_8bit.raw");
    output_msb_10bit_image_file(&my_img, IMG_WIDTH, IMG_HEIGHT, "msb_10bit.raw");
    output_lsb_10bit_image_file(&my_img, IMG_WIDTH, IMG_HEIGHT, "lsb_10bit.raw");
    output_12bit_image_file(&my_img, IMG_WIDTH, IMG_HEIGHT, "12bit.raw");

    destroy_image(&my_img);
    return 0;
}
