#include <stdint.h>
#include <vector>
#include "mbed.h"
#include "FXOS8700Q.h"
#include "lcd_lib.h"
#include "logo.h"

const int grid_width = 4;
const int W = 320 / grid_width;
const int H = 240 / grid_width;

typedef struct {int x, y; } Vec;

Serial pc(USBTX, USBRX);
I2C i2c(PTE25, PTE24);
FXOS8700QAccelerometer acc(i2c, FXOS8700CQ_SLAVE_ADDR1);

InterruptIn left(PTC9);
InterruptIn down(PTC10);
InterruptIn up(PTC11);
InterruptIn right(PTC12);

std::vector<Vec> snake;
Vec food;
Vec head;
Vec speed;
Vec key_dir;
int eat = 0;

uint16_t color_val(uint8_t r, uint8_t g, uint8_t b) {
	r = r * 0x1F / 255;
	g = g * 0x3F / 255;
	b = b * 0x1F / 255;

	return (r << 11) | (g << 5) | b;
}

const uint16_t RED = color_val(255, 0, 0);
const uint16_t GREEN = color_val(0, 255, 0);
const uint16_t BLUE = color_val(0, 0, 255);
const uint16_t BLACK = color_val(0, 0, 0);


void fill(int x, int y, uint16_t color) {
	 for(int w = 0; w < grid_width; w++) {
		for(int h = 0; h < grid_width; h++) {
			LCD_put_pixel(x * grid_width + w, y * grid_width + h, color);
		}
	}
}

void gen_food() {
	int x = rand() % W;
	int y = rand() % H;
	food = {x, y};

	fill(x, y, RED);
}

bool add(int x, int y) {
	for(Vec p: snake) {
		if(p.x == x && p.y == y) {
			return false;
		}
	}

	snake.push_back({x, y});
	fill(x, y, BLUE);

	return true;
}

void pop() {
	int x = snake[0].x;
	int y = snake[0].y;
	snake.erase(snake.begin());

	fill(x, y, BLACK);
}

void reset_game() {
	snake.clear();
	gen_food();
	speed = {1, 0};
	eat = 0;

	head = {W / 2, H / 2};
	add(head.x, head.y);

	for(int i = 0; i < 5; i++) {
		head.x++;
		add(head.x, head.y);
	}
}

Vec handle_accelerometer() {
	float threshold = 0.2;

	motion_data_units_t acc_data;
	acc.getAxis(acc_data);
	//pc.printf("%f %f %f\r\n", acc_data.x, acc_data.y, acc_data.z);

	Vec dir = {0, 0};
	if(fabs(acc_data.y) > threshold) {
		dir.y = acc_data.y > 0 ? -1 : 1;
	}

	if(fabs(acc_data.x) > threshold) {
		dir.x = acc_data.x > 0 ? 1 : -1;
	}

	return dir;
}

Vec handle_buttons() {
	if(!up) {
		return {0, -1};
	}
	if(!down) {
		return {0, 1};
	}
	if(!left) {
		return {-1, 0};
	}
	if(!right) {
		return {1, 0};
	}

	return {0, 0};
}

Vec handle_async_buttons() {
	return key_dir;
}

void waitkey() {
	while(left && right && up && down);
}

void poll_buttons() {
	key_dir = handle_buttons();
}

void draw_image(int x0, int y0, int w, int h, const uint16_t *image) {
	int j = 0;
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			LCD_put_pixel(x0 + x, y0 + y, image[j++]);
		}
	}
	//LCD_fill_data(x0, y0, w, h, image);
}

int main() {
	LCD_init();
	LCD_clear();

	uint16_t *image = (uint16_t*) raw_rgb565;
	draw_image(320 / 2 - raw_rgb565_width / 2, 140, raw_rgb565_width, raw_rgb565_height, image);

	left.fall(poll_buttons);
	right.fall(poll_buttons);
	up.fall(poll_buttons);
	down.fall(poll_buttons);

	acc.enable();

	LCD_print_centered(80, 4, RED, "The Snake");
	LCD_print_centered(80 + 4*8, 1, BLUE, "Press any key to start a game");
	waitkey();
	LCD_clear();

	reset_game();

	for(;;) {
		Vec dir = handle_accelerometer();
		//Point dir = handle_buttons();
		//Point dir = handle_async_buttons();
    
		speed = dir;

		head.x = ((head.x + speed.x) % W + W) % W;
		head.y = ((head.y + speed.y) % H + H) % H;

		if(!add(head.x, head.y)) {
			LCD_print_centered(80, 4, RED, "Game over!");
			LCD_print_centered(80 + 4*8, 1, BLUE, "Score: %d apples", eat);
			waitkey();
			LCD_clear();
			reset_game();
			continue;
		}

		if(food.x == head.x && food.y == head.y) {
			gen_food();
			eat++;
		} else {
			pop();
		}

		wait_ms(100);
	}
}
