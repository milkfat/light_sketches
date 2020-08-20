/*

TO DO:
	*in progress* Client/Peer system, chips find eachother over WiFi
	
	*done* Make window size dynamic on PC

	*done* fill_shape() to add interpolation
	*done* fill_shape() to support z buffer
	make fill_shape() better


	Refactor fonts
	3d fonts
	vector fonts
	
	*in progress/mostly done* 3d travel through grid (flight.h) -- got a nice start
	3d travel through perlin noise?
	3d falling coins

	random block assembly sketch, kind of like dominoes

	tree sketch

	phone competitive game where you control the camera to follow a dot
	  dot starts slow and slowly accelerates
	  you lose once the dot leaves the screen

	different phone control games?

	new clock styles/effects

	realtime information
	  trending tweets
	  display images
	  weather
	  other fun things
*/

#define NATIVE_RES_WINDOW
//#define LEAS_ROOM


#ifdef NATIVE_RES_WINDOW
	//native resolution
	#define MATRIX_WIDTH 1280	
	#define MATRIX_HEIGHT 960
	#define WINDOW_WIDTH MATRIX_WIDTH
	#define WINDOW_HEIGHT MATRIX_HEIGHT
#else 
	#ifdef LEAS_ROOM
		//Lea's room simulation
		#define MATRIX_WIDTH 32
		#define MATRIX_HEIGHT 32
		#define WINDOW_WIDTH 980
		#define WINDOW_HEIGHT 980
	#else
		//led matrix "simulation"
		#define MATRIX_WIDTH 256
		#define MATRIX_HEIGHT 196
		#define WINDOW_WIDTH ((MATRIX_WIDTH)*5)
		#define WINDOW_HEIGHT ((MATRIX_HEIGHT)*5)
	#endif
#endif
     
#define SCREEN_WIDTH MATRIX_WIDTH
#define SCREEN_HEIGHT MATRIX_HEIGHT
//misc libraries
#include <iostream>
#include <string>
#include <memory>

uint8_t debug_flag = 0;
int32_t max_iterations = 0;
int32_t iteration_cnt = 0;
int32_t iteration_calls = 1;
//graphics/audio/keyboard/mouse/joystick library6
//https://www.libsdl.org/index.php
//SDL2-2.0.9
#include <SDL2/SDL.h>
#include <openssl/conf.h>
//clock stuff
#include <chrono>
#include <thread>

//network stuff for UDP
//https://www.boost.org/
//boost_1_69_0
#include <boost/asio.hpp>
 
SDL_Window* window = NULL;

SDL_Renderer *renderer = NULL;

SDL_Texture *bitmapTex = NULL;

SDL_Surface *bitmapSurface = NULL;

SDL_Surface *screen = NULL; // even with SDL2, we can still bring ancient code back

//functions from the FastLED library
#include "fastled_functions.h"

//functions from Arduino
#include "arduino_functions.h"

#include "debug_helpers.h"

uint32_t debug_micros0 = 0;
uint32_t debug_micros1 = 0;
uint32_t debug_micros2 = 0;
//     uint32_t debug_time2 = micros();
//     debug_micros1 += micros() - debug_time2;
uint8_t button_mult = 1;
bool button_forward = false;
bool button_reverse = false;
bool button_up = false;
bool button_down = false;
bool button_left = false;
bool button_right = false;
bool button_ra0 = false;
bool button_ra1 = false;
bool button_rb0 = false;
bool button_rb1 = false;
bool button_rg0 = false;
bool button_rg1 = false;

void update_matrix();

//load all the magical light sketches
#define NUM_LEDS (MATRIX_WIDTH*MATRIX_HEIGHT+1)
#include "draw_functions.h"
VECTOR3 tree_coords[NUM_LEDS];
uint8_t tree_radians[NUM_LEDS];
CRGB * screen_buffer = new CRGB[SCREEN_WIDTH*SCREEN_HEIGHT+1];

#include "light_sketches.h"

PERSPECTIVE pc_screen(SCREEN_WIDTH, SCREEN_HEIGHT);
void tree_thing() {
    
	pc_screen.update();

	for (int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++) {
			screen_buffer[i].r = 0;
			screen_buffer[i].g = 0;
			screen_buffer[i].b = 0;
	}

    for (int i = 0; i < NUM_LEDS; i++) {
        VECTOR3 p = tree_coords[i];
        p.y-=150*256;
        pc_screen.matrix.rotate(p);
        if (p.z >= 0) {
            scale_z(p);
            pc_screen.perspective(p);
            blendXY(pc_screen,p,led_screen.screen_buffer[i]);
        }
    }        

}

#include "text.h"

#include <server_https.hpp>
#include <fstream>
#include <server_wss.hpp>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
using namespace boost::property_tree;

#include "web_server.h"

uint32_t debug_time = 0;
uint32_t debug_count0 = 0;
uint32_t debug_count1 = 0;
uint32_t debug_count2 = 0;



SDL_bool done = SDL_FALSE;
#include "main_helpers.h"

void log_camera_coordinates() {
	std::cout << "camera: " << (int32_t)led_screen.camera_offset.x << " " << (int32_t)led_screen.camera_offset.y << " " << (int32_t)led_screen.camera_offset.z << " rotation: " << (int32_t)led_screen.camera_rotate_x << " " << (int32_t)led_screen.camera_rotate_y << " " << (int32_t)led_screen.camera_rotate_z << "\n";;
}
//this function is called whenever the screen needs to be updated
CRGB alt_led_buffer[NUM_LEDS]; //alternate buffer necessary for multi-threading
CRGB* active_buffer;
int led_buffer_available = 0;

void update_matrix_func() {

	//draw stuff

	//tree_thing();
	uint32_t * pixel = (uint32_t*)bitmapSurface->pixels;
	uint8_t * pixel_location = (uint8_t*)&active_buffer[led_screen.XY(0,SCREEN_HEIGHT-1)].r;
#ifdef NATIVE_RES_WINDOW

	//draw screen at native resolution
	for (int i = 0; i < SCREEN_HEIGHT*SCREEN_WIDTH; i++) {
			//memcpy(pixel++, pixel_location+=3,3);
			*pixel++ = pixel_location[2] << 16 | pixel_location[1] << 8 | pixel_location[0];
			pixel_location+=3;
	}

#else
#ifdef LEAS_ROOM
	pixel += 10;
	pixel += (10*(bitmapSurface->pitch))/4;
	for (int i = 0; i < 762; i++) {
		uint8_t * active_pixel_location = pixel_location + (i/6)*3;
		uint32_t * active_pixel = pixel;
		for (int y = 0; y < 4; y++) {
			//draw one line of an LED pixel
			for (int x = 0; x < 4; x++) {
				*active_pixel++ = active_pixel_location[0] | active_pixel_location[1] << 8 | active_pixel_location[2] << 16;
			}
			//avance to the next line
			active_pixel+=(bitmapSurface->pitch)/4 - 4;
		}

		//advance to the next pixel
		if (i < 191) {
			pixel += 5;
		} else if (i < 191+150) {
			pixel += (5 * (bitmapSurface->pitch)/4);
		} else if (i < 191+150+40) {
			pixel -= 5;
		} else if (i < 191+150+40+40) {
			pixel += (5 * (bitmapSurface->pitch)/4);
		} else if (i < 191+150+40+40+151) {
			pixel -= 5;
		} else {
			pixel -= (5 * (bitmapSurface->pitch)/4);
		} 
	}

#else

	//draw led pixel simulation
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		
		//write one line of pixels
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			//memcpy(pixel++, pixel_location+=3,3);
			*pixel++ = pixel_location[2] | pixel_location[1] << 8 | pixel_location[0] << 16;
			*pixel++ = pixel_location[2] | pixel_location[1] << 8 | pixel_location[0] << 16;
			*pixel++ = pixel_location[2] | pixel_location[1] << 8 | pixel_location[0] << 16;
			*pixel++ = pixel_location[2] | pixel_location[1] << 8 | pixel_location[0] << 16;
			pixel++;
			pixel_location+=3;
		}
		
		//copy the line of pixels down to the next few lines
		for (int i = 0; i < 3; i++) {
			memcpy(pixel, pixel-(bitmapSurface->pitch)/4, WINDOW_WIDTH*4);
			pixel+=(bitmapSurface->pitch)/4;
		}

		//advance to the next line of pixels
		pixel+=(bitmapSurface->pitch)/4;
		//pixel+=2;
	}

#endif
#endif



	uint32_t start_time = micros();
	uint32_t debug_before = micros();
	//bitmapTex = SDL_CreateTextureFromSurface(renderer, bitmapSurface);
	//SDL_RenderClear(renderer);
	SDL_UpdateTexture(bitmapTex, NULL, bitmapSurface->pixels, bitmapSurface->pitch);
	SDL_RenderCopy(renderer, bitmapTex, NULL, NULL);
	SDL_RenderPresent(renderer);
	//SDL_UpdateWindowSurface(window);

	debug_count0++;
	debug_micros0 += micros() - debug_before;
	led_buffer_available = 0;

}
void update_matrix() {
	active_buffer = leds;
	update_matrix_func();
}
/*
std::thread _update_matrix_thread;
void update_matrix() {
	if (_update_matrix_thread.joinable()) {
		_update_matrix_thread.join();
	}

#ifdef DOUBLE_BUFFER
	if ( leds == main_led_buffer) {
		active_buffer = main_led_buffer;
		leds = alt_led_buffer;
		led_screen.screen_buffer = leds;
	} else {
		active_buffer = alt_led_buffer;
		leds = main_led_buffer;
		led_screen.screen_buffer = leds;
	}
#else
	active_buffer = main_led_buffer;
	leds = main_led_buffer;
	led_screen.screen_buffer = leds;
#endif

	led_buffer_available = 1;
	_update_matrix_thread = std::thread(update_matrix_func);
}
*/

int main(int argc, char **argv){
	srand(time(nullptr));

	pc_screen.screen_buffer = screen_buffer;

	// std::cout << "\n\n\n";
	// for (int i = 0; i < 256; i++) {
	// 	std::cout << i << (int)sin8(i) << " " << (int)cos8(i) << " " << (int)sin16(i*256) << " " << (int)cos16(i*256) << "\n";
	// }
	// std::cout << "\n\n\n";

	// std::cout << "\n\n\n";
	// for (int i = 0; i < 65536; i++) {
	// 	std::cout << i << ": " << bellCurve16(bellCurve16(bellCurve16(i))) << "\n";
	// }
	// std::cout << "\n\n\n";
	
	for (int i = 0; i < NUM_LEDS; i++) {
	    tree_coords[i] = VECTOR3(0, -150*256, 30*256);
	    rotate_y(tree_coords[i], i*17);
	    tree_radians[i] = i*17;
	    tree_coords[i].y = (i*65535)/(NUM_LEDS-1);
	    //std::cout << tree_coords[i].x << " " << tree_coords[i].y << " " << tree_coords[i].z << "\n";
	}

	// uint32_t bt = millis();
	// int asdf = 0;
	// for (int i = 0; i < 10000000; i++) {
	// 	asdf += random(1000);
	// }
	// std::cout << "\n\nRandom time: " << millis() - bt << "(" << asdf << ")\n\n";

	// for (int i = 10000000-1000; i < 10000000; i++) {
	// 	half_t num = i;6
	// 	num+=.61f;
	// 	num*=2.15f;
	// 	float num2 = i;
	// 	num2+=.61f;
	// 	num2*=2.15f;
	// 	std::cout << (float)((float)(i+.61f)*2.15f);
	// 	std::cout << " = ";
	// 	std::cout << (float)num;
	// 	std::cout << " = ";
	// 	std::cout << (float)num2;
	// 	std::cout << "\n";
	// }

	// for (uint i = 0; i < 4000; i++) {
	// 	std::cout << "MSB: " << i << " " << msb(i) << "\n";
	// }

	std::cout << "Y_BUF size: " << sizeof(Y_BUF) << "\n";

   https_server.start();
   wss_server.start(); 


   light_sketches.send_sketch_controls_reg(wss_send);

	//clock stuff to set frames-per-second
	using clock = std::chrono::steady_clock;
	using frames = std::chrono::duration<std::int64_t, std::ratio<1, 60>>;
	auto next_frame = std::chrono::system_clock::now() + frames{0};

	auto time_now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = time_now - time_start;

    std::srand(elapsed.count()*1000); 


	//initialize socket for UDP transmission
	// boost::asio::io_service io_service;
	// boost::asio::ip::udp::socket socket(io_service); 
	// boost::asio::ip::udp::endpoint remote_endpoint;
	// socket.open(boost::asio::ip::udp::v4());
	// remote_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("192.168.4.101"), 1236);
	// boost::system::error_code err;
	
	//create io service
	boost::asio::io_service io_service;
	
	//start UDP server
	UDP_SERVER server(io_service);	
	std::thread network_io_object([&io_service]() { io_service.run(); });
	
	//initialize our graphics window and do stuff
	if (SDL_Init(SDL_INIT_VIDEO) == 0) {

    // create the window like normal
    window = SDL_CreateWindow("SDL2 Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    bitmapSurface = SDL_CreateRGBSurface(0,WINDOW_WIDTH,WINDOW_HEIGHT,32,0,0,0,0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	bitmapTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, bitmapSurface->w, bitmapSurface->h);
    // but instead of creating a renderer, we can draw directly to the screen
    //screen = SDL_GetWindowSurface(window);

	{
		uint32_t * pixel = (uint32_t*)bitmapSurface->pixels;
		for (int i = 0; i < WINDOW_WIDTH*WINDOW_HEIGHT; i++) {
			*pixel++ = 0x00202020;
		}
	}

        if (window) {

			while (millis() < 1000) {};
			light_sketches.loop ();	
			while (millis() < 1000) {};

			// for (uint32_t l = 0; l < 65536; l++) {
			// 	static uint16_t shift = 0;
			// 	static uint16_t og = 0;
			// 	static uint16_t cnt = 0;
			// 	static uint16_t threshold = 2;
			// 	uint16_t g = round(255 * pow((l>>shift)/(float)(((uint16_t)65535)>>shift),1.f/2.2f));
			// 	std::cout << l << " " << shift << " " << g << " " << (uint16_t)gamma16_encode(l) << "\n";
			// 	if (og == g) {
			// 		cnt++;
			// 	} else {
			// 		cnt = 0;
			// 	}
			// 	// if (cnt > threshold) {
			// 	// 	shift++;
			// 	// 	threshold<<=1;
			// 	// 	cnt = 0;
			// 	// }
			// 	og = g;
			// }
			
            while (!done) {
            	static int fps = 0;
				fps++;

				if (millis()-1000 > debug_time && debug_micros0 > 0) {
					uint32_t debug_micros0_avg = debug_micros0/_max(debug_count0,1);
					uint32_t debug_micros1_avg = debug_micros1/_max(debug_count1,1);
					uint32_t debug_micros2_avg = debug_micros2/_max(debug_count1,1);
					debug_count0=0;
					debug_count1=0;
					debug_count2=0;
					debug_micros0=0;
					debug_micros1=0;
					debug_micros2=0;
					//std::cout << (debug_micros1_avg/(debug_micros0_avg+1.f)) << " " << debug_micros1_avg << "` " << debug_micros0_avg << "\n";
					//std::cout << "avg iterations: " << (iteration_cnt/iteration_calls) << "\n";
					std::cout << "Draw time: " << debug_micros0_avg << " Sketch time: " << (debug_micros1_avg-debug_micros0_avg) << " Text time: " << debug_micros2_avg << " FPS: " << fps << "\n";
					// char buff[500];
					// generate_client_list(buff);
					// std::cout << buff << "\n";
					fps = 0;
					//std::cout << "avg iterations: " << (iteration_cnt/iteration_calls) << "\n";
					max_iterations = 0;
					iteration_cnt = 0;
					iteration_calls = 1;
					debug_time = millis();
				}

				next_frame = next_frame + frames{1};
				
				uint32_t time_before = micros();
				light_sketches.loop();

				SDL_Event event;
				poll_inputs(event);

				debug_micros1 += micros() - time_before;
				debug_count1++;
				bool button_pushed = false;
				if (button_up) {led_screen.camera_move(VECTOR3(0,512*button_mult,0));button_pushed=true;}
				if (button_down) {led_screen.camera_move(VECTOR3(0,-512*button_mult,0));button_pushed=true;}
				if (button_forward) {led_screen.camera_move(VECTOR3(0,0,-512*button_mult));button_pushed=true;}
				if (button_reverse) {led_screen.camera_move(VECTOR3(0,0,512*button_mult));button_pushed=true;}
				if (button_left) {led_screen.camera_move(VECTOR3(-512*button_mult,0,0));button_pushed=true;}
				if (button_right) {led_screen.camera_move(VECTOR3(512*button_mult,0,0));button_pushed=true;}
				button_up_pressed = button_forward;
				button_down_pressed = button_reverse;
				button_left_pressed = button_left;
				button_right_pressed = button_right;
				if (button_ra0) {led_screen.camera_rotate_y+=button_mult*75;button_pushed=true;}
				if (button_ra1) {led_screen.camera_rotate_y-=button_mult*75;button_pushed=true;}
				if (button_rb1) {led_screen.camera_rotate_x-=button_mult*75;button_pushed=true;}
				if (button_rb0) {led_screen.camera_rotate_x+=button_mult*75;button_pushed=true;}
				// if (button_ra0) {led_screen.rotation_alpha+=button_mult;button_pushed=true;}
				// if (button_ra1) {led_screen.rotation_alpha-=button_mult;button_pushed=true;}
				// if (button_rb1) {led_screen.rotation_beta+=button_mult;button_pushed=true;}
				// if (button_rb0) {led_screen.rotation_beta-=button_mult;button_pushed=true;}
				if (button_rg0) {led_screen.camera_rotate_z-=button_mult*100;button_pushed=true;}
				if (button_rg1) {led_screen.camera_rotate_z+=button_mult*100;button_pushed=true;}
				if (button_pushed) log_camera_coordinates();
				
				// uint32_t time_before2 = micros();
				// handle_text();
				// debug_micros2 += micros() - time_before2;
				// debug_count2++;

				if (spacebar) {
					spacebar = false; 
					light_sketches.next_sketch();
					light_sketches.loop();
					wss_server.wssCurrentSketch();
        			std::cout << "Current sketch: " << light_sketches.name() << "\n";
				}
				if (next_sketch_name[0]) {
					std::cout << "Next sketch name: " << next_sketch_name << "\n";
					light_sketches.set_sketch(next_sketch_name);
					wss_server.wssCurrentSketch();
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
		SDL_FreeSurface(bitmapSurface);
		SDL_DestroyTexture(bitmapTex);
		SDL_DestroyRenderer(renderer);
        if (window) {
            SDL_DestroyWindow(window);
        }
    }

	led_buffer_available = -1;

#ifdef ENABLE_MULTITHREAD
	worker_thread_stop();

	if ( _update_matrix_thread.joinable() ) {
		_update_matrix_thread.join();
	}
#endif

  	https_server.stop();
	wss_server.stop();
	
	//socket.close();

    SDL_Quit();

	//stop UDP server
	io_service.stop();
	//wait for thread to finish
	network_io_object.join();

    return 0; 
}