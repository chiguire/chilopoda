////////////////////////////////////////////////////////////////////////////////
//
// (C) Ciro Duran 2013
//
// Chilopoda is the class to which anthropods known as Centipedes belong to.
//
//
//

namespace octet {

  /* Converts a tile position to a screen position */
  inline float fromTilePositionToScreenPosition(float xTile, float xTileWidth = 16.0f, float xOffset = -256.0f) {
    return (xOffset + (xTile+0.5f) * xTileWidth);
  }

  class chilo_sprite {
  protected:
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

    int xSpeed;
    int ySpeed;

    static const unsigned int COLLIDE_LEFT = 1;
    static const unsigned int COLLIDE_RIGHT = 2;
    static const unsigned int COLLIDE_TOP = 4;
    static const unsigned int COLLIDE_BOTTOM = 8;

    unsigned int collided_directions;

    chilo_sprite()
    : x(0.0f)
    , y(0.0f)
    , rotation(0.0f)
    , xSpeed(0)
    , ySpeed(0)
    , collided_directions(0)
    { }

    void init(float _x, float _y, float w, float h, int _texture = -1) {
      modelToWorld.loadIdentity();

      x = _x;
      y = _y;
      rotation = 0;
      collided_directions = 0;
      xSpeed = 0;
      ySpeed = 0;

      halfHeight = h * 0.5f;
      halfWidth = w * 0.5f;

      if (_texture != -1) {
        texture = _texture;
      }
      enabled = true;
    }

    void render(texture_palette_shader &shader, mat4t &cameraToWorld, float *color1, float *color2, float *color3, float alpha=1.0f) {

      if (texture == -1) return;

      modelToWorld.loadIdentity();
      modelToWorld.translate(x, y, 0);
      modelToWorld.rotate(rotation, 0, 0, 1);

      // build a projection matrix: model -> world -> camera -> projection
      // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

      // set up the uniforms for the shader
      shader.render(modelToProjection, 0, color1, color2, color3, alpha);

      // this is an array of the positions of the corners of the box in 3D
      // a straight "float" here means this array is being generated here at runtime.
      float vertices[] = {
        -halfWidth, -halfHeight, 0.0f, 0.0f,
        halfWidth, -halfHeight, 1.0f, 0.0f, 
        halfWidth,  halfHeight, 1.0f, 1.0f,
        -halfWidth,  halfHeight, 0.0f, 1.0f
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

    // return true if this box collides with another
    // note the "const"s which say we do not modify either box
    bool collides_with(const chilo_sprite &rhs) {
      static const float OVERLAP = 0.0f;

      float dx = rhs.x - (x + xSpeed);
      float dy = rhs.y - (y + ySpeed);

      collided_directions = 0;

      bool collides_sides = fabsf(dx) < halfWidth + rhs.halfWidth - OVERLAP;
      bool collides_tops = fabsf(dy) < halfHeight + rhs.halfHeight - OVERLAP;

      if (collides_sides && collides_tops) {
        if (collides_sides) {
          if (dx < 0) { //rhs is on left of this sprite
            collided_directions |= COLLIDE_LEFT;
            if (ySpeed == 0) {
              x = rhs.x + rhs.halfWidth + halfWidth;
            }
          } else {
            collided_directions |= COLLIDE_RIGHT;
            if (ySpeed == 0) {
              x = rhs.x - rhs.halfWidth - halfWidth;
            }
          }
        }
        if (collides_tops) {
          if (dy < 0) { //rhs in on bottom of this sprite
            collided_directions |= COLLIDE_BOTTOM;
            if (xSpeed == 0) {
              y = rhs.y + rhs.halfHeight + halfHeight;
            }
          } else {
            collided_directions |= COLLIDE_TOP;
            if (xSpeed == 0) {
              y = rhs.y - rhs.halfHeight - halfHeight;
            }
          }
        }
      }
      // both distances have to be under the sum of the halfwidths
      // for a collision
      return collided_directions? true: false;
    }

    bool collides_with_screen(float left, float right, float top, float bottom) {
      collided_directions = 0;
      if (x + xSpeed < left) {
        collided_directions |= COLLIDE_LEFT;
      }
      if (x + xSpeed > right) {
        collided_directions |= COLLIDE_RIGHT;
      }
      if (y + ySpeed < bottom) {
        collided_directions |= COLLIDE_BOTTOM;
      }
      if (y + ySpeed > top) {
        collided_directions |= COLLIDE_TOP;
      }

      return collided_directions? true: false;
    }

    void move() {
      x += xSpeed;
      y += ySpeed;
    }

    void kill() {
      enabled = false;
    }

    bool &is_enabled() {
      return enabled;
    }

  };

  typedef double_list<chilo_sprite *> sprite_list;

  class fungus_sprite : public octet::chilo_sprite {
  public:
    int health;

    fungus_sprite()
      : chilo_sprite()
      , health(4) {
    }

    void init(float x, float y, float w, float h, int _texture = -1, int _health = 4) {
      chilo_sprite::init(x, y, w, h, _texture);
      health = _health;
    }

    void collide_callback(bool isFire = false) {
      if (isFire) {
        health--;
        if (!health) {
          kill();
        }
      }
    }
  };

  class worm_sprite : public chilo_sprite {
  public:
    enum direction_t {
      direction_left = -1,
      direction_down = 0,
      direction_right = 1,
      direction_up = 2,
    };

    direction_t verticalDirection;
    direction_t previousDirection;
    direction_t direction;
    float yCurrentLine;
    int speed;

    worm_sprite()
      : chilo_sprite()
      , verticalDirection(direction_down)
      , previousDirection(direction_left)
      , direction(direction_left)
      , yCurrentLine(0.0f)
      , speed(4)
    { }

    void init(float x, float y, float w, float h, int _texture = -1,
      direction_t _direction = direction_right, int _speed = 4) {

        chilo_sprite::init(x, y, w, h, _texture);

        yCurrentLine = y;
        verticalDirection = direction_down;
        previousDirection = _direction;
        speed = _speed;

        setDirection(_direction);
    }

    void setDirection(direction_t _d) {
      direction = _d;
      if (direction == direction_right) {
        xSpeed = speed;
        ySpeed = 0;
        rotation = 180.0f;
      } else if (direction == direction_left) {
        xSpeed = -speed;
        ySpeed = 0;
        rotation = 0.0f;
      } else if (direction == direction_up) {
        xSpeed = 0;
        ySpeed = speed;
        rotation = 270.0f;
      } else if (direction == direction_down) {
        xSpeed = 0;
        ySpeed = -speed;
        rotation = 90.0f;
      }
    }

    void followVerticalDirection() {
      if (xSpeed == 0) {
        if ((direction == direction_down && y + ySpeed <= yCurrentLine) ||
            (direction == direction_up && y + ySpeed >= yCurrentLine)) {
          y = yCurrentLine;
          setDirection(previousDirection == direction_left? direction_right: direction_left);
        }
      } else {
        //Try to go further down or up
        previousDirection = direction;
        setDirection(verticalDirection);

        //Try to collide with screen with new direction
        if (collides_with_screen(
          fromTilePositionToScreenPosition(0),
          fromTilePositionToScreenPosition(31), 
          fromTilePositionToScreenPosition(31),
          fromTilePositionToScreenPosition(0))) {
            if (collided_directions & COLLIDE_BOTTOM && verticalDirection == direction_down) {
              verticalDirection = direction_up;
              setDirection(verticalDirection);
            } else if (collided_directions & COLLIDE_TOP && verticalDirection == direction_up) {
              verticalDirection = direction_down;
              setDirection(verticalDirection);
            }
        }

        yCurrentLine += (verticalDirection == direction_down? -1: 1)*halfHeight*2;
      }
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

    // game objects, these are created once at the initialiation
    chilo_sprite gridSprite;
    chilo_sprite playerSprite;
    chilo_sprite fireSprite;
    dynarray<chilo_sprite *> fungusSpriteGroup;
    dynarray<chilo_sprite *> wormSpriteGroup;

    // these objects store pointers to the Group members
    // and they are accessed intensely inside the game loop
    double_list<worm_sprite *> wormList;
    double_list<fungus_sprite *> fungiList;

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
        playerSprite.ySpeed = SHIP_SPEED;
      } else if (is_key_down(key_down)) {
        playerSprite.ySpeed = -SHIP_SPEED;
      } else {
        playerSprite.ySpeed = 0;
      }

      if (is_key_down(key_left)) {
        playerSprite.xSpeed = -SHIP_SPEED;
      } else if (is_key_down(key_right)) {
        playerSprite.xSpeed = SHIP_SPEED;
      } else {
        playerSprite.xSpeed = 0;
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

        // Check player collisions
        for (auto lst = fungiList.begin(); lst != fungiList.end(); ++lst) {
          if (playerSprite.collides_with(**lst)) {
            if (playerSprite.xSpeed > 0 &&
              (playerSprite.collided_directions & chilo_sprite::COLLIDE_RIGHT)) {
                playerSprite.xSpeed = 0;
            } else if (playerSprite.xSpeed < 0 &&
              (playerSprite.collided_directions & chilo_sprite::COLLIDE_LEFT)) {
                playerSprite.xSpeed = 0;
            }

            if (playerSprite.ySpeed > 0 &&
              (playerSprite.collided_directions & chilo_sprite::COLLIDE_TOP)) {
                playerSprite.ySpeed = 0;
            } else if (playerSprite.ySpeed < 0 &&
              (playerSprite.collided_directions & chilo_sprite::COLLIDE_BOTTOM)) {
                playerSprite.ySpeed = 0;
            }
          }
        }

        if (playerSprite.collides_with_screen(
          fromTilePositionToScreenPosition(0),
          fromTilePositionToScreenPosition(31), 
          fromTilePositionToScreenPosition(5),
          fromTilePositionToScreenPosition(0))) {
            if (playerSprite.collided_directions & 
              (chilo_sprite::COLLIDE_LEFT | chilo_sprite::COLLIDE_RIGHT)) {
                playerSprite.xSpeed = 0;
            }
            if (playerSprite.collided_directions & 
              (chilo_sprite::COLLIDE_TOP | chilo_sprite::COLLIDE_BOTTOM)) {
                playerSprite.ySpeed = 0;
            }
        }

        playerSprite.move();

        // Moving fire sprite
        if (fireSprite.is_enabled()) {
          fireSprite.y += 5.0f;
          for (auto w = wormList.begin(); w != wormList.end(); ++w) {
            if (fireSprite.collides_with(**w)) {
              fireSprite.kill();
            }
          }

          if (fireSprite.y >= 260.0f) {
            fireSprite.kill();
          }
        }

        // Moving worms
        bool first = false;
        for (auto w = wormList.begin(); w != wormList.end(); ++w) {
          worm_sprite *wSprite = *w;

          //Collision with screen
          if (wSprite->collides_with_screen(
            fromTilePositionToScreenPosition(0),
            fromTilePositionToScreenPosition(31), 
            fromTilePositionToScreenPosition(31),
            fromTilePositionToScreenPosition(0))) {
              if (wSprite->collided_directions & 
                (chilo_sprite::COLLIDE_LEFT | chilo_sprite::COLLIDE_RIGHT)) {
                  wSprite->followVerticalDirection();
              }
          }

          // Detect possible collisions with fungi 
          for (auto f = fungiList.begin(); f != fungiList.end(); ++f) {

            if (wSprite->direction == worm_sprite::direction_left ||
              wSprite->direction == worm_sprite::direction_right) {
                if (first) printf("Going %s\n", wSprite->direction == worm_sprite::direction_left? "left": "right");
                if (wSprite->collides_with(**f)) {
                  if (wSprite->direction == worm_sprite::direction_left &&
                    wSprite->collided_directions & chilo_sprite::COLLIDE_LEFT) {
                      wSprite->followVerticalDirection();
                      if (first) printf("Collision with fungus on the left\n");
                      break;
                  }
                  if (wSprite->direction == worm_sprite::direction_right &&
                    wSprite->collided_directions & chilo_sprite::COLLIDE_RIGHT) {
                      wSprite->followVerticalDirection();
                      if (first) printf("Collision with fungus on the right\n");
                      break;
                  }
                }
            } else {
              worm_sprite::direction_t previousDirection = wSprite->direction;
              wSprite->followVerticalDirection();
              worm_sprite::direction_t newDirection = wSprite->direction;
              if (previousDirection == newDirection) {
                if (first) printf("Going %s\n", wSprite->direction == worm_sprite::direction_up? "up": "down");
              } else {
                if (first) printf("Reached bottom, going %s\n", wSprite->direction == worm_sprite::direction_left? "left": "right");
              }
            } 
          }                  
          wSprite->move();
          first = false;
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



    /* Returns the first non-enabled sprite from the specified group.
    * If all sprites in group are enabled, function will return NULL.
    */
    chilo_sprite *getFirstSpriteAvailableFromGroup(dynarray<chilo_sprite *> &group) {
      for (int i = 0; i != group.size(); i++) {
        if (!group[i]->is_enabled()) {
          chilo_sprite *spr = group[i];
          spr->init(0.0f, 0.0f, 16.0f, 16.0f);
          return spr;
        }
      }

      return NULL;
    }

  public:

    static const int SHIP_SPEED = 3;

    // this is called when we construct the class
    chilopoda_app(int argc, char **argv)
      : app(argc, argv) {

    }

    ~chilopoda_app() {
      //delete all pointers created in initGame();
    }

    // this is called once OpenGL is initialized
    void app_init() {
      texture_palette_shader_.init();
      cameraToWorld.loadIdentity();

      srand(time(NULL));

      color1[0] = float(rand())/RAND_MAX; color1[1] = float(rand())/RAND_MAX; color1[2] = float(rand())/RAND_MAX;
      color2[0] = float(rand())/RAND_MAX; color2[1] = float(rand())/RAND_MAX; color2[2] = float(rand())/RAND_MAX;
      color3[0] = float(rand())/RAND_MAX; color3[1] = float(rand())/RAND_MAX; color3[2] = float(rand())/RAND_MAX;

      cameraToWorld.translate(0, 0, 512.0f/2.0f);

      initGame(); 
      resetGame();
    }

    /* Initialize game objects, this is initialized once per application */
    void initGame() {
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
        worm_sprite *w = new worm_sprite;
        w->init(-256.0f+(i%32)*16.0f+8.0f, -32.0f-(16*floor(i/32.0f))+8.0f, 16.0f, 16.0f, monsterTex);
        w->kill();
        wormSpriteGroup.push_back(w);
      }

      for (int i = 0; i != 150; i++) {
        fungus_sprite *fung = new fungus_sprite;
        fung->init(-256.0f, -256.0f, 16.0f, 16.0f, mushroom1Tex);
        fung->kill();
        fungusSpriteGroup.push_back(fung);
      }

      state = state_idle;
      score = 0;
    }

    /* Starts a new game */
    void resetGame() {
      //clear game objects

      for (int i = 0; i != 30; i++) {
        worm_sprite *w = static_cast<worm_sprite *>(getFirstSpriteAvailableFromGroup(wormSpriteGroup));
        if (!w) {
          printf("ERROR: Out of sprites in wormSpriteGroup.\n");
        }
        w->init(fromTilePositionToScreenPosition(30.0f-i),
          fromTilePositionToScreenPosition(32.0f),
          16.0f, 16.0f, monsterTex,
          worm_sprite::direction_right);
        wormList.push_back(w);
      }

      for (int i = 0; i != 30; i++) {
        fungus_sprite *f = static_cast<fungus_sprite *>(getFirstSpriteAvailableFromGroup(fungusSpriteGroup));
        float xRandom = floor((float(rand())/RAND_MAX)*31.0f);
        float yRandom = 5.0f + floor((float(rand())/RAND_MAX)*(32.0f-5.0f));
        f->init(fromTilePositionToScreenPosition(xRandom),
          fromTilePositionToScreenPosition(yRandom),
          16.0f, 16.0f, mushroom1Tex, 4);
        if (!f) {
          printf("ERROR: Out of sprites in fungusSpriteGroup.\n");
        }
        fungiList.push_back(f); 
      }

      state = state_playing;
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

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      gridSprite.render(texture_palette_shader_, cameraToWorld, color1, color2, color3, 0.25f);
      playerSprite.render(texture_palette_shader_, cameraToWorld, color1, color2, color3);

      for (auto w = wormList.begin(); w != wormList.end(); ++w) {
        (*w)->render(texture_palette_shader_, cameraToWorld, color1, color2, color3);
      }

      for (auto f = fungiList.begin(); f != fungiList.end(); ++f) {
        (*f)->render(texture_palette_shader_, cameraToWorld, color1, color2, color3);
      }

      if (fireSprite.is_enabled()) {
        fireSprite.render(texture_palette_shader_, cameraToWorld, color1, color2, color3);
      }
    }

    void fire() {
      if (fireSprite.is_enabled()) {
        return;
      }
      fireSprite.init(playerSprite.x, playerSprite.y+16.0f, 1.0f, 10.0f);
    }
  };
}
