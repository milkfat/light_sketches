#ifndef LIGHTS_VIDEO_STREAM_H
#define LIGHTS_VIDEO_STREAM_H


class VIDEO_STREAM: public LIGHT_SKETCH {
  public:
    VIDEO_STREAM () {setup();}
    ~VIDEO_STREAM () {}
    IMAGE_BUFFER img_buf;

  private:

  public:

    void setup ()
    {   
    }

    void reset ()
    {

    }

    void next_effect()
    {

    }

    void loop()
    {
        if ( image_buffer ) {
          memcpy(leds, image_buffer, MATRIX_WIDTH * MATRIX_HEIGHT * 3);
        }
        LED_show();
        LED_black();
    }

};


LIGHT_SKETCHES::REGISTER<VIDEO_STREAM> video_stream("video_stream");


#endif