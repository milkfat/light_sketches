void poll_inputs(SDL_Event& event) {

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
                    button_mult = (event.key.keysym.mod & KMOD_SHIFT) ? 5 : 1;
                    int mult = (event.key.keysym.mod & KMOD_SHIFT) ? 5 : 1;
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE: done=SDL_TRUE; break;
                        case SDLK_SPACE: spacebar=true; break;
                        case SDLK_f: button2_down=true; text_shake_time = millis(); (current_font >= NUMBER_OF_FONTS) ? current_font = 0 : current_font++; break;
                        case SDLK_g: button1_down=true; button1_click=true; break;
                        case SDLK_n: next_sketch=true; break;
                        case SDLK_r: reset_sketch=true; break;
                        case SDLK_c: debug_flag=true; break;

                        case SDLK_l: button_ra0 = true; break;
                        case SDLK_j: button_ra1 = true; break;
                        case SDLK_i: button_rb0 = true; break;
                        case SDLK_k: button_rb1 = true; break;
                        case SDLK_o: button_rg0 = true; break;
                        case SDLK_u: button_rg1 = true; break;

                        //case SDLK_d: pc_screen.rotation_alpha+=mult; break;
                        //case SDLK_a: pc_screen.rotation_alpha-=mult; break;
                        //case SDLK_w: pc_screen.rotation_beta+=mult; break;
                        //case SDLK_s: pc_screen.rotation_beta-=mult; break;
                        //case SDLK_e: pc_screen.rotation_gamma+=mult; break;

                        case SDLK_t: typing_mode=true; SDL_StartTextInput(); break;
                        // case SDLK_UP:  button_forward = true; break;
                        // case SDLK_DOWN: button_reverse = true; break;
                        // case SDLK_LEFT:  button_left = true; break;
                        // case SDLK_RIGHT: button_right = true; break;
                        case SDLK_w:  button_forward = true; break;
                        case SDLK_s: button_reverse = true; break;
                        case SDLK_a:  button_left = true; break;
                        case SDLK_d: button_right = true; break;
                        case SDLK_MINUS:  button_up = true; break;
                        case SDLK_LEFTBRACKET:  button_down = true; break;
                        case SDLK_p:    led_screen.screen_distance-=256; std::cout << "screen: " << (int32_t)led_screen.screen_distance << "\n"; break;
                        case SDLK_RIGHTBRACKET:  led_screen.screen_distance+=256; std::cout << "screen: " << (int32_t)led_screen.screen_distance << "\n"; break; 
                    }
                }
                break;

            case SDL_KEYUP:
                button_mult = (event.key.keysym.mod & KMOD_SHIFT) ? 5 : 1;
                switch (event.key.keysym.sym)
                {
                    case SDLK_l: button_ra0 = false; break;
                    case SDLK_j: button_ra1 = false; break;
                    case SDLK_i: button_rb0 = false; break;
                    case SDLK_k: button_rb1 = false; break;
                    case SDLK_o: button_rg0 = false; break;
                    case SDLK_u: button_rg1 = false; break;

                    case SDLK_w: button_forward = false; break;
                    case SDLK_s: button_reverse = false; break;
                    case SDLK_MINUS: button_up = false; break;
                    case SDLK_LEFTBRACKET: button_down = false; break;
                    case SDLK_f: button2_down=false; break;
                    case SDLK_g: button1_down=false; break;
                    case SDLK_a:  button_left = false; break;
                    case SDLK_d: button_right = false; break;
                }
			    break;
        }
	}
}