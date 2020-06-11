#include <iostream>
#include <chrono>

using namespace std;

#include <Windows.h>

chrono::time_point<chrono::system_clock> timer_1;
chrono::time_point<chrono::system_clock> timer_2;
float f_elapsed;

const auto n_screen_width = 80;
const auto n_screen_height = 30;

SMALL_RECT r_console;

HANDLE h_console;
CHAR_INFO* p_screen_buffer;

float f_text_pos_x = 0;
float f_text_pos_y = 10;
float f_text_delta = 10;

float f_block_pos_x = static_cast<float>(n_screen_width - 1);
float f_block_pos_y = 10;
float f_block_delta = -10;

enum colour
{
	fg_black = 0x0000,
	fg_white = 0x000f,
	fg_blue = 0x0009,
	fg_green = 0x000a,
	fg_red = 0x000c,

	bg_black = 0x0000,
	bg_white = 0x00F0,
	bg_blue = 0x0090,
	bg_green = 0x00a0,
	bg_red = 0x00c0
};

void console_initialise()
{
	h_console = GetStdHandle(STD_OUTPUT_HANDLE);
	r_console = { 0, 0, static_cast<short>(n_screen_width) - 1, static_cast<short>(n_screen_height) - 1 };
}

void buffer_clear()
{
	memset(p_screen_buffer, 0, sizeof(CHAR_INFO) * n_screen_width * n_screen_height);
}

void buffer_initialise()
{
	p_screen_buffer = new CHAR_INFO[n_screen_width * n_screen_height];
	buffer_clear();
}

void buffer_cleanup()
{
	delete[] p_screen_buffer;
}

void buffer_present()
{
	WriteConsoleOutput(h_console, p_screen_buffer, { static_cast<short>(n_screen_width), static_cast<short>(n_screen_height) }, { 0,0 }, &r_console);
}

void buffer_draw(const float x, const float y, const short c = 0x2588, const short col = 0x000f)
{
	p_screen_buffer[static_cast<int>(y) * n_screen_width + static_cast<int>(x)].Char.UnicodeChar = c;
	p_screen_buffer[static_cast<int>(y) * n_screen_width + static_cast<int>(x)].Attributes = col;
}

void buffer_draw_text(const int x, const int y, std::wstring c, const short col = 0x000F)
{
	for (size_t i = 0; i < c.size(); i++)
	{
		p_screen_buffer[y * n_screen_width + x + i].Char.UnicodeChar = c[i];
		p_screen_buffer[y * n_screen_width + x + i].Attributes = col;
	}
}

void buffer_draw_text_hc(const int y, std::wstring c, const short col = 0x000F)
{
	const auto x = (n_screen_width - c.size()) / 2;
	buffer_draw_text(x, y, c, col);
}

void timer_initialise()
{
	timer_1 = std::chrono::system_clock::now();
	timer_2 = std::chrono::system_clock::now();
}

void timer_update()
{
	timer_2 = std::chrono::system_clock::now();
	const std::chrono::duration<float> elapsed = timer_2 - timer_1;
	timer_1 = timer_2;
	f_elapsed = elapsed.count();
}

CHAR_INFO* buffer_peek(const float x, const float y)
{
	return &p_screen_buffer[static_cast<int>(y) * n_screen_width + static_cast<int>(x)];
}

int main()
{
	try
	{
		SetConsoleTitle(L"Colour Console Output Test");

		console_initialise();
		buffer_initialise();
		timer_initialise();

		while (!(0x8000 & GetAsyncKeyState(VK_ESCAPE)))
		{
			timer_update();
			buffer_clear();

			buffer_draw_text_hc(n_screen_height - 1, L"Press ESC to exit.", fg_green);
			
			buffer_draw(f_block_pos_x, f_block_pos_y, L' ', bg_red);
			auto* const chr = buffer_peek(f_text_pos_x, f_text_pos_y);
			buffer_draw(f_text_pos_x, f_text_pos_y, L'X', fg_blue | chr->Attributes);

			if (f_block_pos_x <= 0)
			{
				f_block_delta *= -1;
				f_block_pos_x = 0;
			}
			else if (f_block_pos_x >= n_screen_width) 
			{
				f_block_delta *= -1;
				f_block_pos_x = n_screen_width;
			}
			
			if (f_text_pos_x <= 0)
			{
				f_text_delta *= -1;
				f_text_pos_x = 0;
			}
			else if (f_text_pos_x >= n_screen_width) 
			{
				f_text_delta *= -1;
				f_text_pos_x = n_screen_width;
			}

			f_block_pos_x += f_block_delta * f_elapsed;
			f_text_pos_x += f_text_delta * f_elapsed;

			buffer_present();
		}

		buffer_cleanup();

		return EXIT_SUCCESS;
	}
	catch (...)
	{
		return EXIT_FAILURE;
	}
}
