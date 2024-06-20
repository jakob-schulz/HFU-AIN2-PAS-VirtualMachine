/*
 * A16_Farbbalken01.c
 *
 * http://raspberrycompote.blogspot.ie/2013/03/low-level-graphics-on-raspberry-pi-part_8.html
 *
 * Original work by J-P Rosti (a.k.a -rst- and 'Raspberry Compote')
 *
 * Licensed under the Creative Commons Attribution 3.0 Unported License
 * (http://creativecommons.org/licenses/by/3.0/deed.en_US)
 *
 * Distributed in the hope that this will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

// default framebuffer palette
// das ist keine Hardware-Palette, wie in der VL erklärt, sondern
// hier werden die 16 Farbwerte definiert:
typedef enum {
	BLACK = 0, /*   0,   0,   0 */
	BLUE = 1, /*   0,   0, 172 */
	GREEN = 2, /*   0, 172,   0 */
	CYAN = 3, /*   0, 172, 172 */
	RED = 4, /* 172,   0,   0 */
	PURPLE = 5, /* 172,   0, 172 */
	ORANGE = 6, /* 172,  84,   0 */
	LTGREY = 7, /* 172, 172, 172 */
	GREY = 8, /*  84,  84,  84 */
	LIGHT_BLUE = 9, /*  84,  84, 255 */
	LIGHT_GREEN = 10, /*  84, 255,  84 */
	LIGHT_CYAN = 11, /*  84, 255, 255 */
	LIGHT_RED = 12, /* 255,  84,  84 */
	LIGHT_PURPLE = 13, /* 255,  84, 255 */
	YELLOW = 14, /* 255, 255,  84 */
	WHITE = 15 /* 255, 255, 255 */
} COLOR_INDEX_T;

// hier 3 Arrays mit je 16 Einträgen für die
// Rot-, Grün- und Blau-Anteile der 16 Farben
/*static unsigned short def_r[] =
 { 0,   0,   0,   0, 172, 172, 172, 168,
 84,  84,  84,  84, 255, 255, 255, 255};
 static unsigned short def_g[] =
 { 0,   0, 168, 168,   0,   0,  84, 168,
 84,  84, 255, 255,  84,  84, 255, 255};
 static unsigned short def_b[] =
 { 0, 172,   0, 168,   0, 172,   0, 168,
 84, 255,  84, 255,  84, 255,  84, 255};
 */

typedef enum Shape {
	Rectangle, Triangle
} Shape;

// 'global' variables to store screen info
unsigned char *fbp = 0;			// Adresse unseres Framebuffer-Speichers
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

// helper function to 'plot' a pixel in given color
// Je nach Farbmodell brauchen wir Funktionen, die ein Pixel
// an die "richtige" Stelle "schreiben"

// A: Farbmodell "8Bit" - brauchen wir hier nicht
// void put_pixel(int x, int y, int c)

// B: Farbmodell RGB24, d.h. 1 Pixel beansprucht 3 Byte
void put_pixel_RGB24(int x, int y, int r, int g, int b) {
	// calculate the pixel's byte offset inside the buffer
	// note: x * 3 as every pixel is 3 consecutive bytes
	// line_length: in byte
	unsigned int pix_offset = x * 3 + y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	// Achtung: cast nach char* für byte-weisen Speicherzugriff!!
	*((char*) (fbp + pix_offset)) = r;
	*((char*) (fbp + pix_offset + 1)) = g;
	*((char*) (fbp + pix_offset + 2)) = b;

}

// C: Farbmodell RGB565, d.h. 1 Pixel beansprucht 2 Byte
void put_pixel_RGB565(int x, int y, int r, int g, int b) {
	unsigned int pix_offset = x * 2 + y * finfo.line_length;
	unsigned int short c = ((r / 8) << 11) + ((g / 4) << 5) + (b / 8);
	*((unsigned short*) (fbp + pix_offset)) = c;
}

// D: Farbmodell RGBA32, d.h. mit Alphablending
void put_pixel_BGRA32(int x, int y, int b, int g, int r, int a) {
	// calculate the pixel's byte offset inside the buffer
	// note: x * 4 as every pixel is 4 consecutive bytes
	// line_length: in byte
	unsigned int pix_offset = x * 4 + y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	// mit char* wieder byte-weise auf den Speicher zugreifen
	*((char*) (fbp + pix_offset)) = b;
	*((char*) (fbp + pix_offset + 1)) = g;
	*((char*) (fbp + pix_offset + 2)) = r;
	*((char*) (fbp + pix_offset + 3)) = a;

}

void drawShape(enum Shape shape, int x_pos, int y_pos, int x_size, int y_size,
		int b, int g, int r, int a) /*drawShape zusaetzlich um Attribut shape erweitert, damit man festlegen kann, welche Figur gezeichnet werden muss*/{
	if (shape == Rectangle) {
		for (int y = y_pos; y < y_pos + y_size; y++) {
			for (int x = x_pos; x < x_pos + x_size; x++) {
				switch (vinfo.bits_per_pixel) {
				case 16:
					put_pixel_RGB565(x, y, r, g, b);
					break;
				case 24:
					put_pixel_RGB24(x, y, r, g, b);
					break;
				case 32:
					put_pixel_BGRA32(x, y, r, g, b, 255);
					break;
				default:
					printf("Fehlendes Farbmodell: %d\n", vinfo.bits_per_pixel);
					exit(0);
				}
			}
		}
	}
	if (shape == Triangle) {
		int xstartpos = x_pos + (x_size / 2);
		int currentwidth = 0;
		for (int y = y_pos; y < y_pos + y_size; y++) {
			currentwidth = ((y - y_pos) * x_size) / y_size;
			for (int x = xstartpos - currentwidth / 2;
					x < xstartpos + currentwidth / 2; x++) {
				switch (vinfo.bits_per_pixel) {
				case 16:
					put_pixel_RGB565(x, y, r, g, b);
					break;
				case 24:
					put_pixel_RGB24(x, y, r, g, b);
					break;
				case 32:
					put_pixel_BGRA32(x, y, r, g, b, 255);
					break;
				default:
					printf("Fehlendes Farbmodell: %d\n", vinfo.bits_per_pixel);
					exit(0);
				}
			}
		}
	}
}

int main(int argc, char *argv[]) {
	int fbfd = 0;			// File-Handle für Framebuffer-Device
	struct fb_var_screeninfo orig_vinfo;
	long int screensize = 0;

	// Open the Framebuffer pseudo-file for reading and writing
	fbfd = open("/dev/fb0", O_RDWR);
	if (!fbfd) {
		printf("Error: cannot open framebuffer device.\n");
		return (1);
	}
	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
		printf("Error reading variable information.\n");
	}
	printf("Original %dx%d, %dbpp\n", vinfo.xres, vinfo.yres,
			vinfo.bits_per_pixel);
	// Store for reset (copy vinfo to vinfo_orig)
	memcpy(&orig_vinfo, &vinfo, sizeof(struct fb_var_screeninfo));

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
		printf("Error reading fixed information.\n");
	}
	// Jetzt Framebuffer-Dev mit User-Memory "verknüpfen" (map)
	// Wie groß muß der Speicher sein (in Byte)?
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
	fbp = (unsigned char*) mmap(0, screensize,
	PROT_READ | PROT_WRITE,
	MAP_SHARED, fbfd, 0);
	if (fbp == (unsigned char*) -1) {
		printf("Failed to mmap.\n");
	} else {

		int color = 0;
		for (int yLoop = 0; yLoop < vinfo.yres ; yLoop++) {
			for (int xLoop = 0; xLoop < vinfo.xres; xLoop++) {
				if(vinfo.bits_per_pixel == 16)
				{
			        int color = (xLoop * 4) % 256;

			        // Umwandeln in RGB565
			        unsigned int r = (color >> 3) & 0x1F; // 5 Bits
			        unsigned int g = (color >> 2) & 0x3F; // 6 Bits
			        unsigned int b = (color >> 3) & 0x1F; // 5 Bits

			        unsigned rgb565 = (r << 11) | (g << 5) | b;

			        // Platzierung im Framebuffer
			        unsigned int* pixel = (unsigned int*) (fbp + yLoop * finfo.line_length + xLoop * 2);
			        *pixel = rgb565;
				}
				else
				{
				color = (xLoop * 4) % 256;
				color = (color << 16) + (color << 8) + color;
				*((unsigned int*) (fbp + yLoop * finfo.line_length + xLoop * 4)) = color;
				}
			}
		}
		sleep(5);
	}
	// cleanup, d.h. Original-Werte wieder restaurieren
	munmap(fbp, screensize);
	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vinfo)) {
		printf("Error re-setting variable information.\n");
	}
	close(fbfd);
	return 0;
}
