#include <allegro5/allegro.h>
#include <math.h>
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
	int *xArray, *yArray;
	xArray = new int[576000];
	yArray = new int[576000];
	for (int i = 0; i < 576000; i++) {
		xArray[i] = i % 960;
		yArray[i] = i / 960;
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
	double maxIterations = 128.0;

	ALLEGRO_EVENT event;
	while (running) {
		while (al_get_next_event(queue, &event)) {
			switch (event.type) {
				case ALLEGRO_EVENT_DISPLAY_CLOSE:
					running = false;
					break;
				case ALLEGRO_EVENT_MOUSE_AXES: {
					if (event.mouse.dz) {
						long double newComplexScale = complexScale * powl(2.0L, -event.mouse.dz);
						reLeft += (complexScale - newComplexScale) * event.mouse.x;
						imagTop += (complexScale - newComplexScale) * event.mouse.y;
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
				if (sqrtl(reTemp * reTemp + imagTemp * imagTemp) >= 2.0L) break;
			}
			iterations *= 768.0 / maxIterations;
			if (iterations < 766) {
				maxIterations *= pow((iterations + 1) / 128.0, 1.0 / 16384.0);
				if (maxIterations < 128.0) maxIterations = 128.0;
			}
			unsigned char r = (iterations >= 255 ? 255 : iterations <= 0 ? 0 : iterations);
			unsigned char g = (iterations >= 511 ? 255 : iterations <= 256 ? 0 : iterations - 256);
			unsigned char b = (iterations >= 767 ? 255 : iterations <= 512 ? 0 : iterations - 512);
			al_put_pixel(x, y, al_map_rgb(r, g, b));
			
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

	return 0;
}