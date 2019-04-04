#include <allegro5/allegro.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

bool running = true;
long double reLeft = -2.4L;
long double imagTop = -1.5L;
long double complexScale = 0.005L;
ALLEGRO_BITMAP *mandelbrot;
ALLEGRO_EVENT_QUEUE *queue;

int main() {
	al_init();
	al_install_keyboard();
	al_install_mouse();

	al_set_new_window_title("Mandelbrot set");
	ALLEGRO_DISPLAY *display = al_create_display(960, 600);
	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_mouse_event_source());
	mandelbrot = al_create_bitmap(960, 600);
	ALLEGRO_BITMAP *tempMandelbrot = al_create_bitmap(960, 600);
	int *xArray, *yArray;
	double *pixelisation;
	ALLEGRO_COLOR *colors;
	xArray = new int[576000];
	yArray = new int[576000];
	pixelisation = new double[576000];
	colors = new ALLEGRO_COLOR[2040];
	{
		int r = 0;
		int g = 0;
		int b = 0;
		for (int i = 0; i < 8; i++) {
			int dr = ((i + 1) & 1) - (i & 1);
			int dg = (((i + 1) & 2) - (i & 2)) / 2;
			int db = (((i + 1) & 4) - (i & 4)) / 4;
			for (int j = 0; j < 255; j++) {
				colors[j + i * 255] = al_map_rgb(r, g, b);
				r += dr;
				g += dg;
				b += db;
			}
		}
	}
	for (int i = 0; i < 576000; i++) {
		xArray[i] = i % 960;
		yArray[i] = i / 960;
		pixelisation[i] = 0.0;
	}
	for (int i = 0; i < 576000; i++) {
		int index = i + (rand() + rand() * (RAND_MAX + 1)) % (576000 - i);
		int temp;
		temp = xArray[i];
		xArray[i] = xArray[index];
		xArray[index] = temp;
		temp = yArray[i];
		yArray[i] = yArray[index];
		yArray[index] = temp;
	}

	int i = 0;
	double maxIterations = 256.0;

	ALLEGRO_EVENT event;
	while (running) {
		while (al_get_next_event(queue, &event)) {
			switch (event.type) {
				case ALLEGRO_EVENT_DISPLAY_CLOSE:
					running = false;
					break;
				case ALLEGRO_EVENT_MOUSE_AXES: {
					if (event.mouse.dz) {
						long double relativeScale = powl(2.0L, -event.mouse.dz);
						long double newComplexScale = complexScale * relativeScale;
						long double newReLeft = reLeft + (complexScale - newComplexScale) * event.mouse.x;
						long double newImagTop = imagTop + (complexScale - newComplexScale) * event.mouse.y;
						al_lock_bitmap(mandelbrot, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
						al_lock_bitmap(tempMandelbrot, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
						al_set_target_bitmap(tempMandelbrot);

						for (int x = 0; x < 960; x++) {
							for (int y = 0; y < 600; y++) {
								int prevX = (x - event.mouse.x) * relativeScale + event.mouse.x;
								int prevY = (y - event.mouse.y) * relativeScale + event.mouse.y;
								if (prevX >= 0 && prevX < 960 && prevY >= 0 && prevY < 600) {
									al_put_pixel(x, y, al_get_pixel(mandelbrot, prevX, prevY));
									int index = x + y * 960;
									pixelisation[index] /= relativeScale;
								}
								else {
									pixelisation[x + y * 960] = 0.0;
								}
							}
						}

						al_unlock_bitmap(mandelbrot);
						al_unlock_bitmap(tempMandelbrot);
						ALLEGRO_BITMAP *temp = mandelbrot;
						mandelbrot = tempMandelbrot;
						tempMandelbrot = temp;
						reLeft = newReLeft;
						imagTop = newImagTop;
						complexScale = newComplexScale;
					}
					break;
				}
			}
		}
		al_lock_bitmap(mandelbrot, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
		al_set_target_bitmap(mandelbrot);

		long double re;
		long double imag;
		long double reTemp;
		long double imagTemp;
		int iterations;
		long double reTempTemp;
		clock_t timeout = clock() + 0.1 * CLOCKS_PER_SEC;
		printf_s("iterations: %d\n", (int)maxIterations);
		do {
			int x = xArray[i];
			int y = yArray[i];
			re = reLeft + x * complexScale;
			imag = imagTop + y * complexScale;

			reTemp = 0.0L;
			imagTemp = 0.0L;
			iterations = 0;
			for (; iterations < maxIterations; iterations++) {
				reTempTemp = reTemp * reTemp - imagTemp * imagTemp + re;
				imagTemp = 2.0L * reTemp * imagTemp + imag;
				reTemp = reTempTemp;
				if (reTemp * reTemp + imagTemp * imagTemp >= 4.0L) break;
			}
			if (iterations < maxIterations) {
				maxIterations *= pow((iterations + 1) / maxIterations * 4.0, 1.0 / 65536.0);
				if (maxIterations < 256.0) maxIterations = 256.0;
			}
			else iterations = 1785;
			ALLEGRO_COLOR color = colors[iterations % 2040];
			al_put_pixel(x, y, color);
			pixelisation[x + y * 960] = 1.0;
			int pix = 2;
			while (pix < 8) {
				for (int tx = x / pix * pix; tx < x / pix * pix + pix; tx++) {
					if (tx < 0) continue;
					if (tx >= 960) break;
					for (int ty = y / pix * pix; ty < y / pix * pix + pix; ty++) {
						if (ty < 0) continue;
						if (ty >= 600) break;
						int index = tx + ty * 960;
						if (pixelisation[index] > pix || pixelisation[index] == 0.0) {
							al_put_pixel(tx, ty, color);
							pixelisation[index] = pix;
						}
					}
				}
				pix *= 2;
			}
			
			i++;
			if (i == 576000) i = 0;
		} while (clock() < timeout);

		al_unlock_bitmap(mandelbrot);
		al_set_target_backbuffer(display);
		al_draw_bitmap(mandelbrot, 0, 0, 0);
		al_flip_display();
	}

	delete[] xArray;
	delete[] yArray;
	delete[] pixelisation;
	delete[] colors;

	return 0;
}