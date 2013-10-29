////////////////////////////////////////////////////////////////////////////////
//
// (C) Ciro Duran 2013
//
// Chilopoda is the class to which anthropods known as Centipedes belong to.
//
//
//

namespace octet {
  class chilo_sprite {
    // where is our box (overkill for a ping game!)
    mat4t modelToWorld;

    // half the width of the box
    float halfWidth;

    // half the height of the box
    float halfHeight;

    // what texture is on our sprite?
    int texture;

    // uvs are derived fromn positions with these scales.
    float uscale;
    float vscale;
    float uoffset;
    float voffset;

    bool enabled;

  public:
    
    float color1[3];
    float color2[3];
    float color3[3];


    chilo_sprite() { }

    void init(
      float x, float y, float w, float h,
      int _texture, float uscale_ = 1.0f, float vscale_ = 1.0f, float uoffset_ = 0.5f, float voffset_ = 0.5f
    ) {
      modelToWorld.loadIdentity();
      modelToWorld.translate(x, y, 0);

      halfHeight = h * 0.5f;
      halfWidth = w * 0.5f;

      texture = _texture;
      enabled = true;
      uscale = uscale_;
      vscale = vscale_;
      uoffset = uoffset_;
      voffset = voffset_;

      color1[0] = color1[1] = color1[2] = 1.0f;
      color2[0] = color2[1] = color2[2] = 1.0f;
      color3[0] = color3[1] = color3[2] = 1.0f;
    }

    void render(texture_palette_shader &shader, mat4t &cameraToWorld) {
      // build a projection matrix: model -> world -> camera -> projection
      // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      // set up the uniforms for the shader
      shader.render(modelToProjection, 0, color1, color2, color3);

      // this is an array of the positions of the corners of the box in 3D
      // a straight "float" here means this array is being generated here at runtime.
      float vertices[] = {
        -halfWidth, -halfHeight, -halfWidth * uscale + uoffset, -halfHeight * vscale + voffset,
         halfWidth, -halfHeight,  halfWidth * uscale + uoffset, -halfHeight * vscale + voffset, 
         halfWidth,  halfHeight,  halfWidth * uscale + uoffset,  halfHeight * vscale + voffset,
        -halfWidth,  halfHeight, -halfWidth * uscale + uoffset,  halfHeight * vscale + voffset,
      };

      // attribute_pos (=0) is position of each corner
      // each corner has 3 floats (x, y, z)
      // there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
      glVertexAttribPointer(attribute_pos, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)vertices );
      glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(vertices + 2) );
      glEnableVertexAttribArray(attribute_pos);
      glEnableVertexAttribArray(attribute_uv);
    
      // finally, draw the box (4 vertices)
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // move the object
    void translate(float x, float y) {
      modelToWorld.translate(x, y, 0);
    }

    // position the object relative to another.
    void set_relative(chilo_sprite &rhs, float x, float y) {
      modelToWorld = rhs.modelToWorld;
      modelToWorld.translate(x, y, 0);
    }

    // return true if this box collides with another
    // note the "const"s which say we do not modify either box
    bool collides_with(const chilo_sprite &rhs) const {
      float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];
      float dy = rhs.modelToWorld[3][1] - modelToWorld[3][1];

      // both distances have to be under the sum of the halfwidths
      // for a collision
      return
        (fabsf(dx) < halfWidth + rhs.halfWidth) &&
        (fabsf(dy) < halfHeight + rhs.halfHeight)
      ;
    }

    void kill() {
      enabled = false;
    }

    bool &is_enabled() {
      return enabled;
    }
  };

  class chilopoda_app : public octet::app {
    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    mat4t cameraToWorld;

    // shader to draw a solid color
    texture_palette_shader texture_palette_shader_;

    // what state is the game in?
    enum state_t {
      state_idle,
      state_playing,
      state_died,
      state_game_over
    };
    state_t state;

    // counters for scores
    int score;

    // game objects
    chilo_sprite player;
    dynarray<chilo_sprite> fungiGroup;
    dynarray<chilo_sprite> wormGroup;

    double_list< double_list<chilo_sprite *> > worms;


    // move the objects before drawing
    void simulate() {
      // up and down arrow move the right bat
      if (is_key_down(key_up)) {
        player.translate(0, +0.1f);
      } else if (is_key_down(key_down)) {
        player.translate(0, -0.1f);
      }
      if (is_key_down(key_left)) {
        player.translate(-0.1f, 0);
      } else if (is_key_down(key_right)) {
        player.translate(0.1f, 0);
      }
      if (is_key_down(key_space)) {
        //player.fire();
      }

      if (state == state_idle) {
        // little demo, or game title
        
      } else if (state == state_playing) {
        // if any of the fires collides with a part of the worm
        // remove worm and put a fungus
        // for fire in fires
        //  for worm in worms
        //   for w in worm
        //    if fire.collides_with(w)
        //     w.kill()

        // if we are playing, move the worms
        // for worm in worms
        //   for w in worm
        //     w.move()

        // check collision of any worm with player
        // for worm in worms
        //   for w in worm
        //     if player.collides_with(w)
        //       lose life
      } else if (state == state_died) {
        // start wait counter
        // remove one life
        // if life == 0 then go to game over
      } else if (state == state_game_over) {
        // start wait counter
        // reset screen to idle state
      }
    }

  public:

    // this is called when we construct the class
    chilopoda_app(int argc, char **argv) : app(argc, argv) {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      texture_palette_shader_.init();
      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 0, 5);
      
      GLuint playerTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Ship.gif");
      GLuint mushroom1Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom1.gif");
      GLuint mushroom2Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom2.gif");
      GLuint mushroom3Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom3.gif");
      GLuint mushroom4Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom4.gif");
      GLuint monsterTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Monster.gif");
      GLuint laserTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Laser.gif");
      GLuint blamTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Blam.gif");

      player.init(20, 20, 16, 16, playerTex);

      for (int i = 0; i != 50; i++) {
        
      }


      state = state_idle;
      score = 0;
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      simulate();

      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glEnable(GL_DEPTH_TEST);

      // draw the ball
      player.render(texture_palette_shader_, cameraToWorld);
    }
  };
}
