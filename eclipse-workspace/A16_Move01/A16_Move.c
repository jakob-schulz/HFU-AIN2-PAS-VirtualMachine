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

unsigned char *sbp = 0; //Adresse des Shadowbuffer-Speichers.

// Je nach Farbmodell brauchen wir Funktionen, die ein Pixel
// an die "richtige" Stelle "schreiben"

// B: Farbmodell RGB24, d.h. 1 Pixel beansprucht 3 Byte
void put_pixel_RGB24(int x, int y, int r, int g, int b, float a) {
	// calculate the pixel's byte offset inside the buffer
	// note: x * 3 as every pixel is 3 consecutive bytes
	// line_length: in byte
	unsigned int pix_offset = x * 3 + y * finfo.line_length;

	//Hintergrundwerte der Farben bekommen, fuer Softwareberechnung der Transparenz a:
	int backgroundB = *((unsigned char*) (fbp + pix_offset + 2));
	int backgroundG = *((unsigned char*) (fbp + pix_offset + 1));
	int backgroundR = *((unsigned char*) (fbp + pix_offset));

	//Berechnen des alpha-blendings:
	r = (a * r + (1 - a) * backgroundR);
	g = (a * g + (1 - a) * backgroundG);
	b = (a * b + (1 - a) * backgroundB);

	// now this is about the same as 'fbp[pix_offset] = value'
	// Achtung: cast nach char* für byte-weisen Speicherzugriff!!
	*((char*) (fbp + pix_offset)) = r;
	*((char*) (fbp + pix_offset + 1)) = g; //Zuerst wird auf einen Zeiger auf char dereferenziert und dann auf char
	*((char*) (fbp + pix_offset + 2)) = b;

}

// C: Farbmodell RGB565, d.h. 1 Pixel beansprucht 2 Byte
void put_pixel_RGB565(int x, int y, int r, int g, int b, float a) {
	//Berechnet den Byteversatz eins Pixels innerhalb des buffer. finfo.line_length gibt die Breite einer Zeile in Bytes an (Kann mehr als nur die tatsächliche Breite der Bilddaten umfassen und moeglicherweise Auffülbytes enthalten).
	unsigned int pix_offset = x * 2 + y * finfo.line_length;

	//Hintergrundwerte der Farben bekommen, fuer Softwareberechnung der Transparenz a:
	unsigned short c = *((unsigned short*) (fbp + pix_offset));
	int backgroundB = (c & 0x1F) * 8;
	int backgroundG = ((c >> 5) & 0x3F) * 4;
	int backgroundR = ((c >> 11) & 0x1F) * 8;

	//Berechnen des alpha-blendings:
	r = (a * r + (1 - a) * backgroundR);
	g = (a * g + (1 - a) * backgroundG);
	b = (a * b + (1 - a) * backgroundB);

	//Werte setzen:
	c = ((r / 8) << 11) + ((g / 4) << 5) + (b / 8); //Rot, Gruen und Blau werden entsprechend RGB565 umgewandelt
	*((unsigned short*) (fbp + pix_offset)) = c; //Zuerst wird auf einen Zeiger auf short dereferenziert und dann auf short
}

// D: Farbmodell RGBA32, d.h. mit Alphablending
void put_pixel_BGRA32(int x, int y, int b, int g, int r, float a) {
	// calculate the pixel's byte offset inside the buffer
	// note: x * 4 as every pixel is 4 consecutive bytes
	// line_length: in byte
	unsigned int pix_offset = x * 4 + y * finfo.line_length;

	//Hintergrundwerte der Farben bekommen, fuer Softwareberechnung der Transparenz a:
	int backgroundB = *((unsigned char*) (fbp + pix_offset)); //unsigned char muss fuer Abfragen der Werte verwendet werden, weil ansonsten Werte falsch umgerechnet werden (char: -128 - 127; unsigned char: 0 - 255)
	int backgroundG = *((unsigned char*) (fbp + pix_offset + 1));
	int backgroundR = *((unsigned char*) (fbp + pix_offset + 2));
	//Berechnen des alpha-blendings:
	r = (a * r + (1 - a) * backgroundR);
	g = (a * g + (1 - a) * backgroundG);
	b = (a * b + (1 - a) * backgroundB);

	// now this is about the same as 'fbp[pix_offset] = value'
	// mit char* wieder byte-weise auf den Speicher zugreifen
	*((char*) (fbp + pix_offset)) = b;
	*((char*) (fbp + pix_offset + 1)) = g;
	*((char*) (fbp + pix_offset + 2)) = r;
	//*((char*) (fbp + pix_offset + 3)) = a;

}

void drawShape(enum Shape shape, int x_pos, int y_pos, int x_size, int y_size,
		int b, int g, int r, float a) /*drawShape zusaetzlich um Attribut shape erweitert, damit man festlegen kann, welche Figur gezeichnet werden muss*/{

	//Implementierung Formen:
	if (shape == Rectangle) {
		for (int y = y_pos; y < y_pos + y_size; y++) {
			for (int x = x_pos; x < x_pos + x_size; x++) {
				switch (vinfo.bits_per_pixel) {
				case 16:
					put_pixel_RGB565(x, y, r, g, b, a);
					break;
				case 24:
					put_pixel_RGB24(x, y, r, g, b, a);
					break;
				case 32:
					put_pixel_BGRA32(x, y, b, g, r, a);
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
					put_pixel_RGB565(x, y, r, g, b, a);
					break;
				case 24:
					put_pixel_RGB24(x, y, r, g, b, a);
					break;
				case 32:
					put_pixel_BGRA32(x, y, r, g, b, a);
					break;
				default:
					printf("Fehlendes Farbmodell: %d\n", vinfo.bits_per_pixel);
					exit(0);
				}
			}
		}
	}
}

void draw(int x, int y, int r, int g, int b, float a) {
	switch (vinfo.bits_per_pixel) {
	case 16:
		put_pixel_RGB565(x, y, r, g, b, a);
		break;
	case 24:
		put_pixel_RGB24(x, y, r, g, b, a);
		break;
	case 32:
		put_pixel_BGRA32(x, y, r, g, b, a);
		break;
	default:
		printf("Fehlendes Farbmodell: %d\n", vinfo.bits_per_pixel);
		exit(0);
	}
}

void save_pixel_RGB565(int x, int y) {
	unsigned int pix_offset = x * 2 + y * finfo.line_length;

	//r,g,b wert aus dem Framebuffer an der entsprechenden Stelle wird and den r,g,b Wert an der entsprechenden Stelle in den Shadowbuffer geschrieben
	*((unsigned short*) (sbp + pix_offset)) = *((unsigned short*) (fbp + pix_offset));
}

void restore_pixel_RGB565(int x, int y) {
	unsigned int pix_offset = x * 2+ y * finfo.line_length;

	//r,g,b wert aus dem Shadowbuffer an der entsprechenden Stelle wird and den r,g,b Wert an der entsprechenden Stelle in den Framebuffer geschrieben
	*((unsigned short*) (fbp + pix_offset)) = *((unsigned short*) (sbp + pix_offset));

}

void save_pixel_RGB24(int x, int y) {
	unsigned int pix_offset = x * 3 + y * finfo.line_length;

	//r,g,b wert aus dem Framebuffer an der entsprechenden Stelle wird and den r,g,b Wert an der entsprechenden Stelle in den Shadowbuffer geschrieben
	*((char*) (sbp + pix_offset)) = *((char*) (fbp + pix_offset));
	*((char*) (sbp + pix_offset + 1)) = *((char*) (fbp + pix_offset + 1));
	*((char*) (sbp + pix_offset + 2)) = *((char*) (fbp + pix_offset + 2));
}

void restore_pixel_RGB24(int x, int y) {
	unsigned int pix_offset = x * 3+ y * finfo.line_length;

	//r,g,b wert aus dem Shadowbuffer an der entsprechenden Stelle wird and den r,g,b Wert an der entsprechenden Stelle in den Framebuffer geschrieben
	*((char*) (fbp + pix_offset)) = *((char*) (sbp + pix_offset));
	*((char*) (fbp + pix_offset + 1)) = *((char*) (sbp + pix_offset + 1));
	*((char*) (fbp + pix_offset + 2)) = *((char*) (sbp + pix_offset + 2));
}



void save_pixel_BGRA32(int x, int y) {
	unsigned int pix_offset = x * 4 + y * finfo.line_length;

	//b, g, r wert aus dem Framebuffer an der entsprechenden Stelle wird and den b, g, r Wert an der entsprechenden Stelle in den Shadowbuffer geschrieben
	*((char*) (sbp + pix_offset)) = *((char*) (fbp + pix_offset));
	*((char*) (sbp + pix_offset + 1)) = *((char*) (fbp + pix_offset + 1));
	*((char*) (sbp + pix_offset + 2)) = *((char*) (fbp + pix_offset + 2));
}

void restore_pixel_BGRA32(int x, int y) {
	unsigned int pix_offset = x * 4 + y * finfo.line_length;

	//b, g, r wert aus dem Shadowbuffer an der entsprechenden Stelle wird and den b, g, r Wert an der entsprechenden Stelle in den Framebuffer geschrieben
	*((char*) (fbp + pix_offset)) = *((char*) (sbp + pix_offset));
	*((char*) (fbp + pix_offset + 1)) = *((char*) (sbp + pix_offset + 1));
	*((char*) (fbp + pix_offset + 2)) = *((char*) (sbp + pix_offset + 2));
}

void saveShape(enum Shape shape, int x_pos, int y_pos, int x_size, int y_size) {
	if (shape == Rectangle) {
		for (int y = y_pos; y < y_pos + y_size; y++) {
			for (int x = x_pos; x < x_pos + x_size; x++) {
				switch (vinfo.bits_per_pixel) {
				case 16:
					save_pixel_RGB565(x, y);
					break;
				case 24:
					save_pixel_RGB24(x, y);
					break;
				case 32:
					save_pixel_BGRA32(x, y);
					break;
				default:
					printf("Fehlendes Farbmodell: %d\n", vinfo.bits_per_pixel);
					exit(0);
				}
			}
		}
	}
}

void restoreShape(enum Shape shape, int x_pos, int y_pos, int x_size, int y_size) {
	if (shape == Rectangle) {
			for (int y = y_pos; y < y_pos + y_size; y++) {
				for (int x = x_pos; x < x_pos + x_size; x++) {
					switch (vinfo.bits_per_pixel) {
					case 16:
						restore_pixel_RGB565(x, y);
						break;
					case 24:
						restore_pixel_RGB24(x, y);
						break;
					case 32:
						restore_pixel_BGRA32(x, y);
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

		//"Verknuepfen":
		fbp = (unsigned char*) mmap(0, screensize,
		PROT_READ | PROT_WRITE,
		MAP_SHARED, fbfd, 0);

		//Shadowbuffer in der groesse des Framebuffers anlegen
		sbp = (unsigned char*) malloc(screensize);

		if (fbp == (unsigned char*) -1) {
			printf("Failed to mmap.\n");
		} else {
			for (int yLoop = 0; yLoop < vinfo.yres - 192; yLoop++) { //Die 192 verstehe ich nicht, eigentlich dürfte ohne -192 kein Fehler kommen
				for (int xLoop = 0; xLoop < vinfo.xres; xLoop++) {
					int r = (xLoop * 4) % 256;
					int g = (xLoop * 4) % 256;
					int b = (xLoop * 4) % 256;

					draw(xLoop, yLoop, r, g, b, 1);
				}
			}
			int startposition = 0;
			int iterations = 12;
			for (int pos = startposition; pos < iterations; pos++) {
				if(pos > startposition)
				{
					restoreShape(Rectangle, 50 + 50 * (pos -1), 100, 70, 30);
				}
				if(pos < iterations)
				{
					saveShape(Rectangle, 50 + 50 * pos, 100, 70, 30);
				}
				drawShape(Rectangle, 50 + 50 * pos, 100, 70, 30, 255, 0,
						0, 0.5);
				usleep(200000);
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
