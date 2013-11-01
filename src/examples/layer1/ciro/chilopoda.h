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
    mat4t modelToWorld;

    // half the width of the box
    float halfWidth;

    // half the height of the box
    float halfHeight;

    // what texture is on our sprite?
    int texture;

    bool enabled;


  public:
    float x;
    float y;
    float rotation;

    chilo_sprite() { }

    void init(
      float x, float y, float w, float h,
      int _texture=-1
    ) {
      modelToWorld.loadIdentity();

      this->x = x;
      this->y = y;
      this->rotation = 0;

      halfHeight = h * 0.5f;
      halfWidth = w * 0.5f;

      if (_texture != -1) {
        texture = _texture;
      }
      enabled = true;
    }

    void render(texture_palette_shader &shader, mat4t &cameraToWorld, float *color1, float *color2, float *color3, float alpha=1.0f) {

      if (!texture) return;

      modelToWorld.loadIdentity();
      modelToWorld.rotate(rotation, 0, 0, 1);
      modelToWorld.translate(x, y, 0);

      // build a projection matrix: model -> world -> camera -> projection
      // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      // set up the uniforms for the shader
      shader.render(modelToProjection, 0, color1, color2, color3, alpha);

      // this is an array of the positions of the corners of the box in 3D
      // a straight "float" here means this array is being generated here at runtime.
      float vertices[] = {
        -halfWidth, -halfHeight, 0.0f, 0.0f, //-halfWidth * uscale + uoffset, -halfHeight * vscale + voffset,
         halfWidth, -halfHeight, 1.0f, 0.0f, // halfWidth * uscale + uoffset, -halfHeight * vscale + voffset, 
         halfWidth,  halfHeight, 1.0f, 1.0f,// halfWidth * uscale + uoffset,  halfHeight * vscale + voffset,
        -halfWidth,  halfHeight, 0.0f, 1.0f //-halfWidth * uscale + uoffset,  halfHeight * vscale + voffset,
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

    const float SHIP_SPEED;

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
    chilo_sprite gridSprite;
    chilo_sprite playerSprite;
    chilo_sprite fireSprite;
    dynarray<chilo_sprite> fungusGroup;
    dynarray<chilo_sprite> wormGroup;

    typedef double_list<chilo_sprite *> sprite_list;
    double_list< sprite_list > wormsList;
    double_list< sprite_list > fungiList;

    float color1[3];
    float color2[3];
    float color3[3];

    GLuint playerTex;
    GLuint mushroom1Tex;
    GLuint mushroom2Tex;
    GLuint mushroom3Tex;
    GLuint mushroom4Tex;
    GLuint monsterTex;
    GLuint laserTex;
    GLuint blamTex;
    GLuint gridTex;

    // move the objects before drawing
    void simulate() {
      // up and down arrow move the right bat
      if (is_key_down(key_up)) {
        playerSprite.y += SHIP_SPEED;
      } else if (is_key_down(key_down)) {
        playerSprite.y -= SHIP_SPEED;
      }
      if (is_key_down(key_left)) {
        playerSprite.x -= SHIP_SPEED;
      } else if (is_key_down(key_right)) {
        playerSprite.x += SHIP_SPEED;
      }
      if (is_key_down(key_space)) {
        fire();
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
        if (fireSprite.is_enabled()) {
          fireSprite.y += 5.0f;
          printf("Fire Sprite pos: (%.2f, %.2f)\n", fireSprite.x, fireSprite.y);
          for (auto lst = wormsList.begin(); lst != wormsList.end(); ++lst) {
            for (auto w = lst->begin(); w != lst->end(); ++w) {
              if (fireSprite.collides_with(**w)) {
                fireSprite.kill();

              }
            }
          }

          if (fireSprite.y >= 260.0f) {
            fireSprite.kill();
          }
        }

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
    chilopoda_app(int argc, char **argv)
    : app(argc, argv)
    , SHIP_SPEED(2.0f) {

    }

    // this is called once OpenGL is initialized
    void app_init() {
      texture_palette_shader_.init();
      cameraToWorld.loadIdentity();

      srand(time(NULL));

      /*
      color1[0] = 1.0f; color1[1] = 1.0f; color1[2] = 1.0f;
      color2[0] = 0.0f; color2[1] = 1.0f; color2[2] = 1.0f;
      color3[0] = 1.0f; color3[1] = 0.0f; color3[2] = 1.0f;
      */

      color1[0] = float(rand())/RAND_MAX; color1[1] = float(rand())/RAND_MAX; color1[2] = float(rand())/RAND_MAX;
      color2[0] = float(rand())/RAND_MAX; color2[1] = float(rand())/RAND_MAX; color2[2] = float(rand())/RAND_MAX;
      color3[0] = float(rand())/RAND_MAX; color3[1] = float(rand())/RAND_MAX; color3[2] = float(rand())/RAND_MAX;

      cameraToWorld.translate(0, 0, 512.0f/2.0f);
      
      playerTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Ship.gif");
      mushroom1Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom1.gif");
      mushroom2Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom2.gif");
      mushroom3Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom3.gif");
      mushroom4Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom4.gif");
      monsterTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Monster.gif");
      laserTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Laser.gif");
      blamTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Blam.gif");
      gridTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/grid.gif");

      gridSprite.init(0, 0, 512, 512, gridTex);
      playerSprite.init(0, -200, 16.0f, 16.0f, playerTex);
      fireSprite.init(0, 0, 1.0f, 10.0f, laserTex);
      fireSprite.kill();

      for (int i = 0; i != 150; i++) {
        chilo_sprite w;
        w.init(-256.0f+(i%32)*16.0f+8.0f, -32.0f-(16*floor(i/32.0f))+8.0f, 16.0f, 16.0f, monsterTex);
        w.kill();
        wormGroup.push_back(w);
      }

      sprite_list lst;
      for (int i = 0; i != 3; i++) {
        chilo_sprite *w = getFirstAvailableFromGroup(wormGroup);
        printf("Adding sprite address %p\n", w);
        if (!w) {
          printf("ERROR: Out of sprites in wormGroup");
        }
        w->y = 230;
        w->x = -256-(i+1)*16;
        lst.push_back(w);
      }
      printf("List pointer: %p\n", &lst);
      for (auto lstelem = lst.begin(); lstelem != lst.end(); ++lstelem) {
        printf("Accessing x of an element: %.2f\n", (*lstelem)->x);
      }
      wormsList.push_back(lst);
      printf("List inside wormsList pointer: %p\n", *lst.begin());
      
      for (auto welem = wormsList.begin(); welem != wormsList.end(); ++welem) {
        printf("Assign a worm element from wormList, pointer: %p\n", *welem);
        sprite_list lelst = *welem;
        for (auto lstelem = lelst.begin(); lstelem != lelst.end(); ++lstelem) {
          chilo_sprite *lespr = *lstelem;
          printf("Accessing x of an element: %.2f\n", lespr->x);
        }
      }

      state = state_playing;
      score = 0;
    }

    /* Returns the first non-enabled sprite from the specified group.
     * If all sprites in group are enabled, function will return NULL.
     */
    chilo_sprite *getFirstAvailableFromGroup(dynarray<chilo_sprite> &group) {
      for (int i = 0; i != group.size(); i++) {
        if (!group[i].is_enabled()) {
          chilo_sprite *spr = &group[i];
          spr->init(0.0f, 0.0f, 16.0f, 16.0f);
          return spr;
        }
      }

      return NULL;
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      simulate();

      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      gridSprite.render(texture_palette_shader_, cameraToWorld, color1, color2, color3, 0.25f);
      playerSprite.render(texture_palette_shader_, cameraToWorld, color1, color2, color3);
      for (auto worm = wormsList.begin(); worm != wormsList.end(); ++worm) {
        for (auto w = worm->begin(); w != worm->end(); ++w) {
          (**w).render(texture_palette_shader_, cameraToWorld, color1, color2, color3);
        }
      }

      if (fireSprite.is_enabled()) {
        fireSprite.render(texture_palette_shader_, cameraToWorld, color1, color2, color3);
      }
    }

    void fire() {
      if (fireSprite.is_enabled()) {
        return;
      }
      printf("fire! fire!");
      fireSprite.init(playerSprite.x, playerSprite.y+16.0f, 1.0f, 10.0f);
    }
  };
}
