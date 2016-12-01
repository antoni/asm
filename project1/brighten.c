#include <netpbm/pam.h>
#include <netpbm/ppm.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define DEBUG 1

/* Do not write PPM file header (used for writing PPM to stdout) */
#define TYPE_SWITCH 0

typedef enum Color {
    RED   = 1,
    GREEN = 2,
    BLUE  = 3,
} Color;

static const char* COLOR_NAMES[] = {"red", "green", "blue"};

void usage(const char* prog_name) {
    fprintf(stderr,
            "Usage: %s input_file_name color change\n\n"
            "\tcolor: [R: 1, G: 2, B: 3]\n"
            "\tbrightness change: <-128,127>\n",
            prog_name);
}

/* ASM procedure (see: brighten.asm) */
extern int change_brightness(uint8_t* input_matrix, int nrows, int ncols,
                             Color color, int8_t change);

static inline pixval change_color(unsigned int c, int8_t change_by) {
    if (change_by > 0 && (c + 127) >= 127)
        return 127;
    else if (change_by < 0 && (unsigned int)abs(change_by) >= c)
        return 0;
    return (c + change_by);
}

/* C version of the ASM procedure, used for testing only */
void change_brightness_test(FILE* input_file, FILE* out_fp, Color color,
                            uint8_t change_by) {
    int x1, x2, y;
    int cols, rows, format;
    pixval maxval, r, g, b;

    ppm_readppminit(input_file, &cols, &rows, &maxval, &format);

    pixel *src_row, *dst_row;
    src_row = ppm_allocrow(cols);

    ppm_writeppminit(out_fp, cols, rows, maxval, 0);
    dst_row = ppm_allocrow(cols);

    for (y = 0; y < rows; y++) {
        ppm_readppmrow(input_file, src_row, cols, maxval, format);

        for (x1 = 0, x2 = cols - 1; x1 < cols; x1++, x2--) {
            r = PPM_GETR(src_row[x1]);
            g = PPM_GETG(src_row[x1]);
            b = PPM_GETB(src_row[x1]);

            switch (color) {
                case RED:
                    r = change_color(r, change_by);
                    break;
                case GREEN:
                    g = change_color(g, change_by);
                    break;
                case BLUE:
                    b = change_color(b, change_by);
                    break;
                default:
                    break;
            }
            PPM_ASSIGN(dst_row[x1], r, g, b);
        }
        ppm_writeppmrow(out_fp, dst_row, cols, maxval, TYPE_SWITCH);
    }
    ppm_freerow(src_row);
    ppm_freerow(dst_row);
}

/* Helper function wrapping actual brightness correction in libnetpbm functions
 */
void convert(FILE* input_file, FILE* out_fp, Color color, int change_by) {
    int cols, rows, format;
    pixval maxval, r, g, b;

    ppm_readppminit(input_file, &cols, &rows, &maxval, &format);

    uint8_t* image_data = (uint8_t*)malloc(3 * cols * rows * sizeof(uint8_t));

    pixel* src_row;
    src_row = ppm_allocrow(cols);

    fprintf(stderr,
            "\tcolor:\t\t%s [%d]\n"
            "\tchange:\t\t%d\n"
            "\tmaxval:\t\t%d\t\n"
            "\tcols x rows:\t%d x %d\t\n",
            COLOR_NAMES[color - 1], color, change_by, maxval, cols, rows);

    /* Rewrite libnetpbm format into uint8_t*, which stores
     * red, green and blue values (in that order) */
    for (int i = 0; i < rows; ++i) {
        ppm_readppmrow(input_file, src_row, cols, maxval, format);
        for (int j = 0; j < cols; j++) {
            image_data[i * cols + j] = PPM_GETR(src_row[j]);
            image_data[i * cols + j + rows * cols] = PPM_GETG(src_row[j]);
            image_data[i * cols + j + rows * cols * 2] = PPM_GETB(src_row[j]);
        }
    }

    change_brightness(image_data, rows, cols, color, change_by);

    pixel* dst_row;
    ppm_writeppminit(out_fp, cols, rows, maxval, 0);
    dst_row = ppm_allocrow(cols);

    /* Write data back into libnetpbm format */
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; j++) {
            r = image_data[i * cols + j];
            g = image_data[i * cols + j + rows * cols];
            b = image_data[i * cols + j + rows * cols * 2];
            PPM_ASSIGN(dst_row[j], r, g, b);
        }
        ppm_writeppmrow(out_fp, dst_row, cols, maxval, TYPE_SWITCH);
    }

    free(image_data);

    ppm_freerow(src_row);
    ppm_freerow(dst_row);
}

int main(int argc, char* argv[]) {
    int8_t change_by = 0;
    Color color = RED;

    FILE *input_file, *out_fp;

    if (argc < 3) {
        usage(argv[0]);
        exit(1);
    }

    input_file = pm_openr(argv[1]);
    if (input_file == NULL) {
        fprintf(stderr, "Error opening input file: %s\n", argv[1]);
        exit(1);
    }

    color = atoi(argv[2]);
    change_by = atoi(argv[3]);

    /* Use 3rd argument as filename instead of stdout (for testing only) */
    if (argc == 4) {
        out_fp = stdout;
    } else {
        out_fp = pm_openw(argv[4]);
        if (out_fp == NULL) {
            fprintf(stderr, "Error opening output file: %s\n", argv[2]);
            exit(1);
        }
    }

    convert(input_file, out_fp, color, change_by);

    pm_close(input_file);

    if (out_fp != stdout) pm_close(out_fp);

    return 0;
}
