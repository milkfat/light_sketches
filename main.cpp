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

uint32_t debug_time = 0;
uint32_t debug_count = 0;

bool spacebar = false;
bool next_sketch = false;
bool reset_sketch = false;
bool typing_mode = false;

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

	
  using namespace std;
  using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

  HttpsServer https_server("server.crt", "server.key");

  https_server.config.port = 8080;

  https_server.resource["^/draw$"]["GET"] = [](shared_ptr<HttpsServer::Response> response, shared_ptr<HttpsServer::Request> request) {
    fstream infile; 
    infile.open("draw.html");
    response->write(infile);
    infile.close();
  };

  https_server.default_resource["GET"] = [](shared_ptr<HttpsServer::Response> response, shared_ptr<HttpsServer::Request> request) {
        response->write("hello");
  };

  https_server.on_error = [](shared_ptr<HttpsServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
    // Handle errors here
    // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
  };

  thread https_server_thread([&https_server]() {
  	https_server.start();
  });




  using WssServer = SimpleWeb::SocketServer<SimpleWeb::WSS>;
  // WebSocket Secure (WSS)-server at port 8080 using 1 thread
  WssServer wss_server("server.crt", "server.key");
  wss_server.config.port = 8081;

  auto &echo = wss_server.endpoint["^/socket$"];
  

  echo.on_message = [](shared_ptr<WssServer::Connection> connection, shared_ptr<WssServer::InMessage> in_message) {

    try {
      ptree pt;
      read_json(*in_message, pt);
	  int cnt = 0;
	  try {
		for(auto &i : pt.get_child("a")) {
			switch (cnt) {
				case 0:
					rotation_alpha = i.second.get_value<float>();
					break;
				case 1:
					rotation_beta = i.second.get_value<float>();
					break;
				case 2:
					rotation_gamma = i.second.get_value<float>();
					break;
			}
			cnt++;
		}
	  } catch (const exception &e) {

	  }

	  try {
        uint8_t offset = 0;
        int cnt = 0;

        int x = 0;
		int y = 0;
		int x1 = 0;
		int y1 = 0;
		uint8_t p = 0;
		uint8_t id = 0;
        //if (clientNum == 0) {
        //  offset = 16;
        //}
        for ( auto &i : pt.get_child("l") ) {
          for ( auto &j : i.second.get_child("") ) {
            switch (cnt) {
              case 0:
                x = j.second.get_value<int>();
                break;
              case 1:
                y = j.second.get_value<int>();
                break;
              case 2:
                x1 = j.second.get_value<int>();
                break;
              case 3:
                y1 = j.second.get_value<int>();
                break;
              case 4:
                p = j.second.get_value<uint8_t>();
                break;
              case 5:
                id = j.second.get_value<uint8_t>();
                break;
		    }
		    cnt++;
			button2_down = true;
		  }

		  if (drawing_enabled) {
			draw_line_fine(temp_canvas, (x+offset)*256, y*256, (x1+offset)*256, y1*256, 255, 255, p);
		  }
	
		  update_pointer(x+offset,y,x1+offset,y1,p,id);


		}
	    
	  } catch (const exception &e) {

	  }


	  try {
        for ( auto &i : pt.get_child("e") ) {
          _remove_pointer( i.second.get_value<uint8_t>() );
		  button2_down = false;
        }
      } catch (const exception &e) {

	  }

	  try {
        drawing_enabled = pt.get<int>("dr");
      } catch (const exception &e) {
      }

	  try {
        next_sketch = pt.get<int>("skalt");
      } catch (const exception &e) {
      }
	  
	  try {
        spacebar = pt.get<int>("sknext");
      } catch (const exception &e) {
      }

	  
      //clear canvas
	  try {
        if (pt.get<int>("cc")) {
          for (int i = 0; i < NUM_LEDS; i++) {
            temp_canvas[i] = CRGB::Black;
          }
	    }
      } catch (const exception &e) {
      }

	
	
    } catch (const exception &e) {
    }

  };

  echo.on_open = [](shared_ptr<WssServer::Connection> connection) {
    cout << "Server: Opened connection " << connection.get() << endl;
  };

  // See RFC 6455 7.4.1. for status codes
  echo.on_close = [](shared_ptr<WssServer::Connection> connection, int status, const string & /*reason*/) {
    cout << "Server: Closed connection " << connection.get() << " with status code " << status << endl;
  };

  // Can modify handshake response header here if needed
  echo.on_handshake = [](shared_ptr<WssServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap & /*response_header*/) {
    return SimpleWeb::StatusCode::information_switching_protocols; // Upgrade to websocket
  };

  // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
  echo.on_error = [](shared_ptr<WssServer::Connection> connection, const SimpleWeb::error_code &ec) {
    cout << "Server: Error in connection " << connection.get() << ". "
         << "Error: " << ec << ", error message: " << ec.message() << endl;
  };

  thread wss_server_thread([&wss_server]() {
    // Start WSS-server
    wss_server.start();
  });







	
	// for (int i = -120000; i > -190000; i-=1024) {
	// 	uint16_t c = matt_compress8(i);
	// 	int d = matt_decompress8(c);
	// 	std::cout << i << "  matt_compress " << " = " << (c) << "  matt_decompress " << " = " << (d) << "\n";
	// }

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
						std::cout << (debug_micros1_avg/(debug_micros0_avg+1.f)) << " " << debug_micros1_avg << " " << debug_micros0_avg << "\n";
						std::cout << "avg iterations: " << (iteration_cnt/iteration_calls) << "\n";
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

	//wait for server threads to finish
	https_server_thread.join();
	wss_server_thread.join();
    return 0; 
}