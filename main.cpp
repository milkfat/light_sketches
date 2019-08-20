// #define WINDOW_WIDTH 1840
// #define WINDOW_HEIGHT 1360
// #define MATRIX_WIDTH 360
// #define MATRIX_HEIGHT 250

#define WINDOW_WIDTH 300
#define WINDOW_HEIGHT 980
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 192
    
//misc libraries
#include <iostream>
#include <string>
#include <memory>
uint8_t debug_flag = 0;
int32_t max_iterations = 0;
int32_t iteration_cnt = 0;
int32_t iteration_calls = 1;
//graphics/audio/keyboard/mouse/joystick library
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

uint32_t debug_micros0 = 0;
uint32_t debug_micros1 = 0;
//     uint32_t debug_time2 = micros();
//     debug_micros1 += micros() - debug_time2;

//load all the magical light sketches
#include "light_sketches.h"

#include "text.h"

#include <server_https.hpp>
#include <fstream>
#include <server_wss.hpp>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
using namespace boost::property_tree;

#include "web_server.h"

uint32_t debug_time = 0;
uint32_t debug_count = 0;



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
			//r = sqrt(r*255);
			//g = sqrt(g*255);
			//b = sqrt(b*255);
			//r = _max(r, MIN_VAL);
			//g = _max(g, MIN_VAL);
			//b = _max(b, MIN_VAL);
            SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
            //0,0 top left
            
            //SDL_RenderDrawLine(renderer, x_offset+x*5, WINDOW_HEIGHT-y_offset-y*5, x_offset+x*5+2, WINDOW_HEIGHT-y_offset-y*5);
            //SDL_RenderDrawLine(renderer, x_offset+x*5, WINDOW_HEIGHT-y_offset-y*5-1, x_offset+x*5+2, WINDOW_HEIGHT-y_offset-y*5-1);
            //SDL_RenderDrawLine(renderer, x_offset+x*5, WINDOW_HEIGHT-y_offset-y*5-2, x_offset+x*5+2, WINDOW_HEIGHT-y_offset-y*5-2);
				SDL_RenderDrawPoint(renderer,x_offset+x*5+1,WINDOW_HEIGHT-y_offset-y*5-2+1);
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

		case SDL_TEXTINPUT:
			if (typing_mode) {
        		display_text += event.text.text;
			}
			break;

		case SDL_KEYDOWN:
			if (typing_mode) {
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE: typing_mode=false; SDL_StopTextInput(); break;
					case SDLK_BACKSPACE: 
						if (display_text.length() > 0) {
							old_display_text = display_text;
							display_text.pop_back();
						}
						break;
				}
			} else {
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE: done=SDL_TRUE; break;
					case SDLK_SPACE: spacebar=true; break;
					case SDLK_f: button2_down=true; text_shake_time = millis(); break;
					case SDLK_g: button1_down=true; button1_click=true; break;
					case SDLK_n: next_sketch=true; break;
					case SDLK_r: reset_sketch=true; break;
					case SDLK_d: debug_flag=true; break;
					case SDLK_t: typing_mode=true; SDL_StartTextInput(); break;
					case SDLK_LEFT:  camera_scaler--; std::cout << "camera: " << (int16_t)camera_scaler << "\n"; break;
					case SDLK_RIGHT: camera_scaler++; std::cout << "camera: " << (int16_t)camera_scaler << "\n"; break;
					case SDLK_UP:    screen_scaler--; std::cout << "screen: " << (int16_t)screen_scaler << "\n"; break;
					case SDLK_DOWN:  screen_scaler++; std::cout << "screen: " << (int16_t)screen_scaler << "\n"; break; 
				}
			}
			break;

		case SDL_KEYUP:
			switch (event.key.keysym.sym)
			{
				case SDLK_f: button2_down=false; break;
				case SDLK_g: button1_down=false; break;
			}
			break;
		}
	}


	
}

int main(int argc, char **argv){	

   https_server.start();
   wss_server.start();

	//clock stuff to set frames-per-second
	using clock = std::chrono::steady_clock;
	using frames = std::chrono::duration<std::int64_t, std::ratio<1, 60>>;
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
            
			while (millis() < 1000) {};
			
            while (!done) {
				
				if (millis()-1000 > debug_time && debug_micros0 > 0) {
						debug_count=1;
						uint32_t debug_micros0_avg = debug_micros0/debug_count;
						uint32_t debug_micros1_avg = debug_micros1/debug_count;
						//std::cout << (debug_micros1_avg/(debug_micros0_avg+1.f)) << " " << debug_micros1_avg << "` " << debug_micros0_avg << "\n";
						//std::cout << "avg iterations: " << (iteration_cnt/iteration_calls) << "\n";
						max_iterations = 0;
						iteration_cnt = 0;
						iteration_calls = 1;
						debug_time = millis();
						debug_micros0 = 0;
						debug_micros1 = 0;
					}

				next_frame = next_frame + frames{1};
				
        uint32_t debug_time = micros();
				light_sketches.loop();
		debug_micros0 += micros() - debug_time;

				handle_text();

				if (spacebar) {
					spacebar = false; 
					light_sketches.next_sketch();
					wss_server.wssCurrentSketch();
        			std::cout << "Current sketch: " << light_sketches.name() << "\n";
				}
				if (next_sketch_name[0]) {
					std::cout << "Next sketch name: " << next_sketch_name << "\n";
					for (int i = 0; i < 20; i++) {
						if (light_sketches.names(i) && strcmp(next_sketch_name, light_sketches.names(i)) == 0) {
							light_sketches.set_sketch(i);
							wss_server.wssCurrentSketch();
							break;
						}
					}
					next_sketch_name[0]='\0';
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

  	https_server.stop();
	wss_server.stop();
	
	socket.close();

    SDL_Quit();

    return 0; 
}