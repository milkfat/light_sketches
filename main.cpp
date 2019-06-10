// #define WINDOW_WIDTH 1840
// #define WINDOW_HEIGHT 1360
// #define MATRIX_WIDTH 360
// #define MATRIX_HEIGHT 250

#define WINDOW_WIDTH 980
#define WINDOW_HEIGHT 980
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 192
    
//misc libraries
#include <iostream>
#include <string>
#include <memory>

//graphics/au o/keyboard/mouse/joystick library
//https://www.libsdl.org/index.php
//SDL2-2.0.9
#include <SDL2/SDL.h>

//clock stuff
#include <chrono>
#include <thread>

//network stuff for UDP
//https://www.boost.org/
//boost_1_69_0
#include <boost/asio.hpp>
 
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

//functions from the FastLED library
#include "fastled_functions.h"

//functions from Arduino
#include "arduino_functions.h"

//load all the magical light sketches
#include "light_sketches.h"

uint32_t debug_time = millis();
uint32_t debug_count = 0;

bool spacebar = false;
bool next_sketch = false;
bool reset_sketch = false;

SDL_bool done = SDL_FALSE;

//this function is called whenever the screen needs to be updated
void update_matrix() {

    uint32_t start_time = micros();


	//draw stuff
	SDL_Event event;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

  
    int32_t x_offset = (WINDOW_WIDTH - (MATRIX_WIDTH * 5))/2;
	int32_t y_offset = (WINDOW_HEIGHT - (MATRIX_HEIGHT * 5))/2;
	
    for (int x = 0; x < MATRIX_WIDTH; x++) {
        for (int y = 0; y < MATRIX_HEIGHT; y++) {
			#define MIN_VAL 8
			int r = (int)leds[XY(x,y)].r;
			int g = (int)leds[XY(x,y)].g;
			int b = (int)leds[XY(x,y)].b;
			r = sqrt(r*255);
			g = sqrt(g*255);
			b = sqrt(b*255);
			r = _max(r, MIN_VAL);
			g = _max(g, MIN_VAL);
			b = _max(b, MIN_VAL);
            SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
            //0,0 top left
            
            //SDL_RenderDrawLine(renderer, x_offset+x*5, WINDOW_HEIGHT-y_offset-y*5, x_offset+x*5+2, WINDOW_HEIGHT-y_offset-y*5);
            //SDL_RenderDrawLine(renderer, x_offset+x*5, WINDOW_HEIGHT-y_offset-y*5-1, x_offset+x*5+2, WINDOW_HEIGHT-y_offset-y*5-1);
            //SDL_RenderDrawLine(renderer, x_offset+x*5, WINDOW_HEIGHT-y_offset-y*5-2, x_offset+x*5+2, WINDOW_HEIGHT-y_offset-y*5-2);
			
				SDL_Rect rect;
				rect.x = x_offset+x*5;
				rect.y = WINDOW_HEIGHT-y_offset-y*5-2;
				rect.w = 3;
				rect.h = 3;
				//create a square
				SDL_RenderDrawRect(renderer, &rect);
			
		}
    }
/*
	//debug stuff
	uint32_t end_time = micros();

	if (end_time - start_time > debug_count) {
		debug_count = end_time - start_time;
	}
	
	if (millis() > 1000 && millis() - 1000 > debug_time) {
		std::cout << "Max frame time: " << debug_count << " Skipped frames: " << skipped_frames << "\n";
		debug_time = millis();
		debug_count = 0;
		skipped_frames = 0;

	}
*/
	//draw stuff
	SDL_RenderPresent(renderer);

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			done = SDL_TRUE;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
				case SDLK_ESCAPE: done=SDL_TRUE; break;
				case SDLK_SPACE: spacebar=true; break;
				case SDLK_r: reset_sketch=true; break;
				case SDLK_n: next_sketch=true; break;
				case SDLK_LEFT:  camera_scaler--; break;
				case SDLK_RIGHT: camera_scaler++; break;
				case SDLK_UP:    screen_scaler--; break;
				case SDLK_DOWN:  screen_scaler++; break;
			}
			break;
		}
	}


	
}

int main(int argc, char **argv){

	//clock stuff to set frames-per-second
	using clock = std::chrono::steady_clock;
	using frames = std::chrono::duration<std::int64_t, std::ratio<1, 120>>;
	auto next_frame = std::chrono::system_clock::now() + frames{0};

	auto time_now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = time_now - time_start;

    std::srand(elapsed.count()*1000); 


	//initialize socket for UDP transmission
	boost::asio::io_service io_service;
	boost::asio::ip::udp::socket socket(io_service); 
	boost::asio::ip::udp::endpoint remote_endpoint;
	socket.open(boost::asio::ip::udp::v4());
	remote_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("192.168.4.101"), 1236);
	boost::system::error_code err;

	//initialize our graphics window and do stuff
	if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        

        if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer) == 0) {
            
			
            while (!done) {

				next_frame = next_frame + frames{1};
				
				light_sketches.loop();
				if (spacebar) {
					spacebar = false; 
					light_sketches.next_sketch();
				}
				if (next_sketch) {
					next_sketch = false; 
					light_sketches.next_effect();
				}
				if (reset_sketch) {
					reset_sketch = false; 
					light_sketches.reset();
				}
				 
				
				/*
				//send RGB data over UDP
				
				//buffer for image data and PPM header
				char my_buffer[8+(NUM_LEDS-1)*3];
				
				//PPM container header
				for (int i = 0; i < 9; i++) {
					my_buffer[i] = "P6 4 4 1 "[i];
				}

				//add image data to buffer
				for (int i = 0; i < NUM_LEDS-1; i++) {
					int j = 9+i*3;
					my_buffer[j+0]=leds[(NUM_LEDS-2)-i].r;
					my_buffer[j+1]=leds[(NUM_LEDS-2)-i].g;
					my_buffer[j+2]=leds[(NUM_LEDS-2)-i].b;
				}
				int buffer_position = 0;
				int bytes_remaining = sizeof(my_buffer);
				//send image data over UDP with max frame size of 1472
				while (bytes_remaining > 1472) {
					socket.send_to(boost::asio::buffer(&my_buffer[buffer_position], 1472), remote_endpoint, 0, err);
					buffer_position += 1472;
					bytes_remaining -= 1472;
					//std::cout << "Bytes sent: 1472\n";
				}
				
				if (bytes_remaining > 0) {
					socket.send_to(boost::asio::buffer(&my_buffer[buffer_position], bytes_remaining), remote_endpoint, 0, err);
					//std::cout << "Bytes sent: " << bytes_remaining << "\n";
				}
				*/

				//wait for the next frame
				std::this_thread::sleep_until(next_frame);
			
            }
        }

        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
	socket.close();
    SDL_Quit();
    return 0; 
}