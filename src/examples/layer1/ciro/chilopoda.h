////////////////////////////////////////////////////////////////////////////////
//
// (C) Ciro Duran 2013
//
// Chilopoda is the class to which anthropods known as Centipedes belong to.
//
//
//

namespace octet {
  
  class chilopoda_app_config {
  public:
    static const int INITIAL_LIVES = 3;          // Player lives
    static const int SHIP_SPEED = 10;            // Player speed
    static const int FIRE_SPEED = 10;            // Bullet speed
    static const int INITIAL_WORM_SIZE = 10;     // First level worm size, each level one piece is added
    static const int MAX_WORM_SIZE = 150;        // Maximum worms available in pool
    static const int INITIAL_FUNGUS_SIZE = 50;   // Initial number of fungi placed in leve
    static const int MAX_FUNGUS_SIZE = 300;      // Maximum number of fungi in pool
    static const int WORM_SPEED = 4;             // Worm speed, constant throughout all levels
    static const int FUNGUS_HEALTH = 4;          // Fungus initial health

    static const int PLAYER_DIED_TIME = 2;       // Time in seconds displaying player death before restarting
    static const int LEVEL_FINISHED_TIME = 3;    // Time in seconds displaying level finished before advancing
    static const int GAME_OVER_DISPLAY_TIME = 3; // Time in seconds displaying game over sign
    static const int BLAM_DISPLAY_TIME = 1;      // Time in seconds displaying worm explosion sprite

    static const int SPIDER_APPEREANCE_BASE_TIME = 4; // Time in seconds where a spider appears
    static const int SPIDER_APPEREANCE_DEVIATION = 2; // Time in seconds of random deviation of spider time
    static const int SPIDER_SPEED = 2;                // Spider speed
    static const int SPIDER_CHOOSE_TIME = 45;         // Time in ticks when a spider makes a direction decision
    static const int SPIDER_MAX_FUNGUS = 5;           // Maximum number of fungus spider can plant
    static const int SPIDER_FUNGUS_PLANT_PROB = 4*RAND_MAX/5; // rand() must be bigger than this to plant a fungus
    static const int SPIDER_FUNGUS_PLANT_TIME = 90;   // Time in ticks between spider fungus plantation attempt

    // Screen coordinates in game. Change these if changing screen, tile or board dimensions.
    static const int TILE_WIDTH = 16;
    static const int SCREEN_OFFSET = -256;
    static const int MIN_TILE = 0;
    static const int MAX_TILE = 31;
    static const int SCREEN_SIZE = 512;
  };

  /* Converts a tile position to a screen position */
  inline float from_tile_position_to_screen_position(float xTile,
                                                     float xTileWidth = chilopoda_app_config::TILE_WIDTH,
                                                     float xOffset = chilopoda_app_config::SCREEN_OFFSET) {
    return (xOffset + (xTile+0.5f) * xTileWidth);
  }

  inline float from_screen_position_to_tile_position(float xScreen,
                                                     float xTileWidth = chilopoda_app_config::TILE_WIDTH,
                                                     float xOffset = chilopoda_app_config::SCREEN_OFFSET) {
    return floorf(((xScreen - xOffset - 0.5f*xTileWidth)/xTileWidth)+0.5f);
  }

  class color {
  public:
    struct color_hsv_t {
      float h;
      float s;
      float v;
    };

    float r;
    float g;
    float b;
    float a;

    color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}

    color(float _r, float _g, float _b, float _a = 1.0f)
    : r(_r), g(_g), b(_b), a(_a) { }

    color &operator=(const color &rhs) {
      r = rhs.r;
      g = rhs.g;
      b = rhs.b;
      a = rhs.a;
      return *this;
    }

    void cycle_color(int angle) {
      color_hsv_t hsv = to_HSV();
      hsv.h += angle;
      if (hsv.h >= 360.0f) hsv.h -= 360.0f;
      color c = from_HSV(hsv.h, hsv.s, hsv.v);
      r = c.r;
      g = c.g;
      b = c.b;
    }

    color_hsv_t to_HSV() {
      color_hsv_t result;
      
      float min, max, delta;

      min = r < g? r: g;
      min = min < b? min: b;

      max = r > g? r: g;
      max = max > b? max: b;

      result.v = max;
      delta = max - min;
      
      if (max > 0.0f) {
        result.s = (delta / max);
      } else {
        result.s = 0.0f;
        result.h = 0.0f;
        return result;
      }

      if (r >= max) {
        result.h = (g - b)/delta;
      } else if (g >= max) {
        result.h = 2.0f + (b - r)/delta;
      } else {
        result.h = 4.0f + (r - g)/delta;
      }

      result.h *= 60.0f;

      if (result.h < 0.0f) {
        result.h += 360.0f;
      }

      return result;
    }

    static color from_HSV(float h, float s, float v) {
      color c;

      if (s <= 0) {
        c.r = v;
        c.g = v;
        c.b = v;
        return c;
      }

      float hh = h;
      if (hh >= 360.0f) hh = 0.0f;
      hh /= 60.0f;
      int i = (int)hh;
      float ff = hh-i;
      float p = v * (1.0f - s);
      float q = v * (1.0f - (s * ff));
      float t = v * (1.0f - (s * (1.0f - ff)));

      switch (i) {
      case 0:
        c.r = v;
        c.g = t;
        c.b = p;
        break;
      case 1:
        c.r = q;
        c.g = v;
        c.b = p;
        break;
      case 2:
        c.r = p;
        c.g = v;
        c.b = t;
        break;
      case 3:
        c.r = p;
        c.g = q;
        c.b = v;
        break;
      case 4:
        c.r = t;
        c.g = p;
        c.b = v;
        break;
      case 5:
      default:
        c.r = v;
        c.g = p;
        c.b = q;
        break;
      }
        
      return c;
    }
  };
  class chilo_sprite {
  protected:
    mat4t modelToWorld;

    // half the width of the box
    float halfWidth;

    // half the height of the box
    float halfHeight;

    bool enabled;

  public:
    float x;
    float y;
    float rotation;
    int texture;

    int xSpeed;
    int ySpeed;

    static const unsigned int COLLIDE_LEFT = 1;
    static const unsigned int COLLIDE_RIGHT = 2;
    static const unsigned int COLLIDE_TOP = 4;
    static const unsigned int COLLIDE_BOTTOM = 8;

    unsigned int collidedDirections;

    chilo_sprite()
    : x(0.0f)
    , y(0.0f)
    , rotation(0.0f)
    , xSpeed(0)
    , ySpeed(0)
    , collidedDirections(0)
    { }

    void init(float _x, float _y, float w, float h, int _texture = -1) {
      modelToWorld.loadIdentity();

      x = _x;
      y = _y;
      rotation = 0;
      collidedDirections = 0;
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

      collidedDirections = 0;

      bool collides_sides = fabsf(dx) < halfWidth + rhs.halfWidth - OVERLAP;
      bool collides_tops = fabsf(dy) < halfHeight + rhs.halfHeight - OVERLAP;

      if (collides_sides && collides_tops) {
        if (collides_sides) {
          if (dx < 0) { //rhs is on left of this sprite
            collidedDirections |= COLLIDE_LEFT;
            if (ySpeed == 0) {
              x = rhs.x + rhs.halfWidth + halfWidth;
            }
          } else {
            collidedDirections |= COLLIDE_RIGHT;
            if (ySpeed == 0) {
              x = rhs.x - rhs.halfWidth - halfWidth;
            }
          }
        }
        if (collides_tops) {
          if (dy < 0) { //rhs in on bottom of this sprite
            collidedDirections |= COLLIDE_BOTTOM;
            if (xSpeed == 0) {
              y = rhs.y + rhs.halfHeight + halfHeight;
            }
          } else {
            collidedDirections |= COLLIDE_TOP;
            if (xSpeed == 0) {
              y = rhs.y - rhs.halfHeight - halfHeight;
            }
          }
        }
      }
      // both distances have to be under the sum of the halfwidths
      // for a collision
      return collidedDirections? true: false;
    }

    bool collides_with_screen(float left = from_tile_position_to_screen_position(chilopoda_app_config::MIN_TILE), 
                              float right = from_tile_position_to_screen_position(chilopoda_app_config::MAX_TILE), 
                              float top = from_tile_position_to_screen_position(chilopoda_app_config::MAX_TILE), 
                              float bottom = from_tile_position_to_screen_position(chilopoda_app_config::MIN_TILE)) {
      collidedDirections = 0;
      if (x + xSpeed < left) {
        collidedDirections |= COLLIDE_LEFT;
      }
      if (x + xSpeed > right) {
        collidedDirections |= COLLIDE_RIGHT;
      }
      if (y + ySpeed < bottom) {
        collidedDirections |= COLLIDE_BOTTOM;
      }
      if (y + ySpeed > top) {
        collidedDirections |= COLLIDE_TOP;
      }

      return collidedDirections? true: false;
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
      , health(chilopoda_app_config::FUNGUS_HEALTH) {
    }

    void init(float x, float y, float w, float h, int _texture = -1, int _health = chilopoda_app_config::FUNGUS_HEALTH) {
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

    static int texFrame;

    worm_sprite()
      : chilo_sprite()
      , verticalDirection(direction_down)
      , previousDirection(direction_left)
      , direction(direction_left)
      , yCurrentLine(0.0f)
      , speed(chilopoda_app_config::WORM_SPEED)
    { }

    void init(float x, float y, float w, float h, int _texture = -1,
      direction_t _direction = direction_right, int _speed = chilopoda_app_config::WORM_SPEED) {

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
        if (collides_with_screen()) {
            if (collidedDirections & COLLIDE_BOTTOM && verticalDirection == direction_down) {
              verticalDirection = direction_up;
              setDirection(verticalDirection);
            } else if (collidedDirections & COLLIDE_TOP && verticalDirection == direction_up) {
              verticalDirection = direction_down;
              setDirection(verticalDirection);
            }
        }
        
        yCurrentLine += (verticalDirection == direction_down? -1: 1)*halfHeight*2;
      }
    }
  };

  int worm_sprite::texFrame = 0;

  class spider_sprite : public octet::chilo_sprite {

    bool shouldPlantFungus;
    int movementChooseCounter;
    int plantFungusCounter;
    int fungusPlanted;

  public:
    static int texFrame;

    enum direction_t {
      direction_none,
      direction_down,
      direction_left,
      direction_up,
      direction_right,
    };

    bool fromLeft;
    direction_t horizontalDirection;
    direction_t verticalDirection;
    int speed;
    

    spider_sprite()
      : chilo_sprite()
      , fromLeft(false)
      , shouldPlantFungus(false)
      , fungusPlanted(0)
      , verticalDirection(direction_down)
      , horizontalDirection(direction_left)
      , movementChooseCounter(chilopoda_app_config::SPIDER_CHOOSE_TIME)
      , plantFungusCounter(chilopoda_app_config::SPIDER_FUNGUS_PLANT_TIME)
      , speed(chilopoda_app_config::SPIDER_SPEED)
    { }

    void init(float x, float y, float w, float h, int _texture = -1) {
      chilo_sprite::init(x, y, w, h, texture);

      fromLeft = rand() < (RAND_MAX/2);
      y = from_tile_position_to_screen_position(7);
      x = fromLeft? from_tile_position_to_screen_position(-2):
                    from_tile_position_to_screen_position(34);
      horizontalDirection = fromLeft? direction_right: direction_left;
      verticalDirection = direction_down;
      movementChooseCounter = chilopoda_app_config::SPIDER_CHOOSE_TIME;
      plantFungusCounter = chilopoda_app_config::SPIDER_FUNGUS_PLANT_TIME;
      shouldPlantFungus = false;
      fungusPlanted = 0;
    }

    void move() {
      if (horizontalDirection == direction_left) {
        xSpeed = -chilopoda_app_config::SPIDER_SPEED;
      } else if (horizontalDirection == direction_right) {
        xSpeed = chilopoda_app_config::SPIDER_SPEED;
      } else {
        xSpeed = 0;
      }

      if (verticalDirection == direction_up) {
        ySpeed = chilopoda_app_config::SPIDER_SPEED;
      } else if (verticalDirection = direction_down) {
        ySpeed = -chilopoda_app_config::SPIDER_SPEED;
      } else {
        ySpeed = 0;
      }

      // collision with top or bottom
      if (collides_with_screen(from_tile_position_to_screen_position(-10),
                               from_tile_position_to_screen_position(40),
                               from_tile_position_to_screen_position(8),
                               from_tile_position_to_screen_position(0))) {
        makeDecision(true);
      }

      chilo_sprite::move();
      //printf("Spider on (%.2f, %.2f)\n", this->x, this->y);
      // Decide on changing direction
      movementChooseCounter--;
      if (!movementChooseCounter) {
        makeDecision();
        movementChooseCounter = chilopoda_app_config::SPIDER_CHOOSE_TIME;
      }

      // Decide on planting a fungus
      plantFungusCounter--;
      if (!plantFungusCounter) {
        shouldPlantFungus = fungusPlanted < chilopoda_app_config::SPIDER_MAX_FUNGUS &&
                            rand() > chilopoda_app_config::SPIDER_FUNGUS_PLANT_PROB;
        plantFungusCounter = chilopoda_app_config::SPIDER_FUNGUS_PLANT_TIME;
      } else {
        shouldPlantFungus = false;
      }
        
      // Collision on right of screen if coming from left
      if ((fromLeft &&
           collides_with_screen(from_tile_position_to_screen_position(-10),
                               from_tile_position_to_screen_position(34),
                               from_tile_position_to_screen_position(30),
                               from_tile_position_to_screen_position(-2))) ||
         (!fromLeft &&
          collides_with_screen(from_tile_position_to_screen_position(-2),
                               from_tile_position_to_screen_position(34),
                               from_tile_position_to_screen_position(30),
                               from_tile_position_to_screen_position(-2)))) {
        kill();
       }
    }

    void makeDecision(bool hasCollided = false) { //hasCollided with top or bottom
      float s = float(rand())/RAND_MAX;

      if (hasCollided) {
        if (y < from_tile_position_to_screen_position(4)) {
          verticalDirection = direction_up;
        } else {
          verticalDirection = direction_down;
        }
      } else {
        float r = float(rand())/RAND_MAX;

        if (r < 1.0f/3.0f) {
          verticalDirection = direction_down;
        } else if (r < 2.0f/3.0f) {
          verticalDirection = direction_up;
        } else {
          verticalDirection = direction_none;
        }
      }

      if (s < 0.5f) {
        horizontalDirection = fromLeft? direction_right: direction_left;
      } else {
        horizontalDirection = direction_none;
      }
    }

    bool should_plant() {
      if (shouldPlantFungus) fungusPlanted++;
      return shouldPlantFungus;
    }
  };

  int spider_sprite::texFrame = 0;

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
      state_finished_level,
      state_died,
      state_game_over
    };
    state_t state;

    // counters for score, lives, and display time counter (for messages)
    int score;
    int level;
    int lives;
    int displayCounter;
    int blamCounter;
    int spiderCounter;

    // game objects, these are created once at the initialiation
    chilo_sprite gridSprite;
    chilo_sprite playerSprite;
    chilo_sprite fireSprite;
    chilo_sprite blamSprite;
    chilo_sprite explosionSprite;
    chilo_sprite gameoverSprite;
    spider_sprite spiderSprite;
    dynarray<chilo_sprite> livesGroup;
    dynarray<chilo_sprite *> fungusSpriteGroup;
    dynarray<chilo_sprite *> wormSpriteGroup;

    text_overlay overlay;
    bump_shader object_shader;
    bump_shader skin_shader;

    // these objects store pointers to the Group members
    // and they are accessed intensely inside the game loop
    double_list<worm_sprite *> wormList;
    double_list<fungus_sprite *> fungiList;

    color color1;
    color color2;
    color color3;

    GLuint playerTex;
    GLuint mushroom1Tex;
    GLuint mushroom2Tex;
    GLuint mushroom3Tex;
    GLuint mushroom4Tex;
    GLuint monster1Tex;
    GLuint monster2Tex;
    GLuint monster3Tex;
    GLuint monster4Tex;
    GLuint explosion1Tex;
    GLuint explosion2Tex;
    GLuint explosion3Tex;
    GLuint explosion4Tex;
    GLuint laserTex;
    GLuint blamTex;
    GLuint gridTex;
    GLuint gameoverTex;
    GLuint spider1Tex;
    GLuint spider2Tex;
    GLuint spider3Tex;
    GLuint spider4Tex;

    enum {
      num_sound_sources = 32,
    };

    // sounds
    ALuint laserSound;
    ALuint mushroomExplodeSound;
    ALuint playerDiesSound;
    ALuint wormExplodeSound;
    unsigned cur_source;
    ALuint sources[num_sound_sources];

    ALuint get_sound_source() { return sources[cur_source++ % num_sound_sources]; }

    void play_sound(ALuint snd) {
      ALuint source = get_sound_source();
      alSourcei(source, AL_BUFFER, snd);
      alSourcef(source, AL_GAIN, 1.0f);
      alSourcePlay(source);
    }

    // move the objects before drawing
    void simulate() {
      if (state == state_idle) {
        if (is_key_down(key_space)) {
          resetGame(true);  
        }
        game_loop_playing(false);
      } else if (state == state_playing) {
        game_loop_playing(true);
      } else if (state == state_finished_level) {
        color1.cycle_color(2);
        color2.cycle_color(2);
        color3.cycle_color(2);

        displayCounter--;
        if (displayCounter == 0) {
          choose_colors();
          level++;
          resetGame(false);
        }
      } else if (state == state_died) {
        displayCounter--;
        if (displayCounter == 0) {
          lives--;
          livesGroup[lives].kill();
          printf("Remaining lives: %d\n", lives);
          if (lives > 0) {
            resetGame(false);
          } else {
            state = state_game_over;
            color::color_hsv_t hsv1 = color1.to_HSV();
            color::color_hsv_t hsv2 = color2.to_HSV();
            color::color_hsv_t hsv3 = color3.to_HSV();

            hsv1.s = hsv2.s = hsv3.s = 0.0f;

            color1 = color::from_HSV(hsv1.h, hsv1.s, hsv1.v);
            color2 = color::from_HSV(hsv2.h, hsv2.s, hsv2.v);
            color3 = color::from_HSV(hsv3.h, hsv3.s, hsv3.v);

            displayCounter = 60*chilopoda_app_config::GAME_OVER_DISPLAY_TIME;
            gameoverSprite.init(0, 0, 234.0f, 32.0f, gameoverTex);
          }
        }
      } else if (state == state_game_over) {
        displayCounter--;
        if (displayCounter == 0) {
          for (int i = 0; i != chilopoda_app_config::INITIAL_LIVES; i++) {
            livesGroup[i].kill();
          }
          gameoverSprite.kill();
          resetGame(true);
          choose_colors();
          state = state_idle;
        }
      }
    }

    void game_loop_playing(bool hasInteraction = true) {

      if (hasInteraction) { 
        if (is_key_down(key_up)) {
          playerSprite.ySpeed = chilopoda_app_config::SHIP_SPEED;
        } else if (is_key_down(key_down)) {
          playerSprite.ySpeed = -chilopoda_app_config::SHIP_SPEED;
        } else {
          playerSprite.ySpeed = 0;
        }

        if (is_key_down(key_left)) {
          playerSprite.xSpeed = -chilopoda_app_config::SHIP_SPEED;
        } else if (is_key_down(key_right)) {
          playerSprite.xSpeed = chilopoda_app_config::SHIP_SPEED;
        } else {
          playerSprite.xSpeed = 0;
        }

        if (is_key_down(key_space)) {
          fire();
        }
      }

      check_player_collisions(hasInteraction);
      playerSprite.move();

      move_fire_sprite();

      move_worms(hasInteraction);

      worm_sprite::texFrame = (worm_sprite::texFrame + 1) % 60;
      if (blamCounter > 0) {
        blamCounter--;
        if (blamCounter == 0) {
          blamSprite.kill();
        }
      }

      if (spiderSprite.is_enabled()) {
        if (spiderSprite.should_plant()) {
          float xMushroomTile = from_screen_position_to_tile_position(spiderSprite.x);
          float yMushroomTile = from_screen_position_to_tile_position(spiderSprite.y);
          float xMushroom = from_tile_position_to_screen_position(xMushroomTile);
          float yMushroom = from_tile_position_to_screen_position(yMushroomTile);
          fungus_sprite *newFungus = static_cast<fungus_sprite *>(get_first_sprite_available_from_group(fungusSpriteGroup));
          newFungus->init(xMushroom, yMushroom, 16.0f, 16.0f, mushroom1Tex, chilopoda_app_config::FUNGUS_HEALTH);
          fungiList.push_back(newFungus);
        }
        
        spiderSprite.move();

        spider_sprite::texFrame = (spider_sprite::texFrame+1)%120;
        
        switch (spider_sprite::texFrame) {
        case 0:  spiderSprite.texture = spider1Tex; break;
        case 20: spiderSprite.texture = spider2Tex; break;
        case 40: spiderSprite.texture = spider3Tex; break;
        case 60: spiderSprite.texture = spider4Tex; break;
        case 80: spiderSprite.texture = spider3Tex; break;
        case 100: spiderSprite.texture = spider2Tex; break;
        } 

        if (!spiderSprite.is_enabled()) {
          reset_spider_counter();
        }
      } else if (spiderCounter > 0) {
        spiderCounter--;
        if (spiderCounter == 0) {
          spider_sprite::texFrame = 0;
          spiderSprite.init(0, from_tile_position_to_screen_position(7), 48.0f, 20.0f, spider1Tex);
          if (spiderSprite.fromLeft) {
            printf("Spawning spider from left.\n");
            spiderSprite.x = from_tile_position_to_screen_position(-2);
          } else {
            printf("Spawning spider from right.\n");
            spiderSprite.x = from_tile_position_to_screen_position(34);
          }
        }
      }
    }

    void check_player_collisions(bool hasInteraction) {
      for (auto lst = fungiList.begin(); lst != fungiList.end(); ++lst) {
        if (playerSprite.collides_with(**lst)) {
          if (playerSprite.xSpeed > 0 &&
            (playerSprite.collidedDirections & chilo_sprite::COLLIDE_RIGHT)) {
              playerSprite.xSpeed = 0;
          } else if (playerSprite.xSpeed < 0 &&
            (playerSprite.collidedDirections & chilo_sprite::COLLIDE_LEFT)) {
              playerSprite.xSpeed = 0;
          }

          if (playerSprite.ySpeed > 0 &&
            (playerSprite.collidedDirections & chilo_sprite::COLLIDE_TOP)) {
              playerSprite.ySpeed = 0;
          } else if (playerSprite.ySpeed < 0 &&
            (playerSprite.collidedDirections & chilo_sprite::COLLIDE_BOTTOM)) {
            playerSprite.ySpeed = 0;
          }
        }
      }

      if (playerSprite.collides_with_screen(
        from_tile_position_to_screen_position(chilopoda_app_config::MIN_TILE),
        from_tile_position_to_screen_position(chilopoda_app_config::MAX_TILE), 
        from_tile_position_to_screen_position(5),
        from_tile_position_to_screen_position(chilopoda_app_config::MIN_TILE))) {
          if (playerSprite.collidedDirections & 
            (chilo_sprite::COLLIDE_LEFT | chilo_sprite::COLLIDE_RIGHT)) {
              playerSprite.xSpeed = 0;
          }
          if (playerSprite.collidedDirections & 
            (chilo_sprite::COLLIDE_TOP | chilo_sprite::COLLIDE_BOTTOM)) {
              playerSprite.ySpeed = 0;
          }
      }

      if (hasInteraction && spiderSprite.is_enabled() && playerSprite.collides_with(spiderSprite)) {
        kill_player();
      }
    }

    void move_fire_sprite() {
      if (fireSprite.is_enabled()) {
        fireSprite.y += chilopoda_app_config::FIRE_SPEED;
        bool collided = false;

        if (spiderSprite.is_enabled() && fireSprite.collides_with(spiderSprite)) {
          fireSprite.kill();
          play_sound(wormExplodeSound);
          spiderSprite.kill();
          reset_spider_counter();
          blamCounter = 60*chilopoda_app_config::BLAM_DISPLAY_TIME;
          blamSprite.init(spiderSprite.x, spiderSprite.y, 32.0f, 32.0f, blamTex);
        }

        //Collision of fire with any worm
        for (auto w = wormList.begin(); w != wormList.end(); ++w) {
          if (fireSprite.collides_with(**w)) { // Put a new mushroom where body was
            fireSprite.kill();
            play_sound(wormExplodeSound);

            float xMushroomTile = from_screen_position_to_tile_position((*w)->x);
            float yMushroomTile = from_screen_position_to_tile_position((*w)->y);
            float xMushroom = from_tile_position_to_screen_position(xMushroomTile);
            float yMushroom = from_tile_position_to_screen_position(yMushroomTile);
            fungus_sprite *newFungus = static_cast<fungus_sprite *>(get_first_sprite_available_from_group(fungusSpriteGroup));
            newFungus->init(xMushroom, yMushroom, 16.0f, 16.0f, mushroom1Tex, chilopoda_app_config::FUNGUS_HEALTH);
            fungiList.push_back(newFungus);

            blamCounter = 60*chilopoda_app_config::BLAM_DISPLAY_TIME;
            blamSprite.init(xMushroom, yMushroom, 16.0f, 16.0f, blamTex);
            (*w)->kill();
            w = wormList.erase(w);
            collided = true;

            increase_score(10);

            break;
          }
        }

        // Detect level finished
        if (collided) {
          int wormSize = 0;
          for (auto w = wormList.begin(); w != wormList.end(); ++w) {
            wormSize++;
          }
          if (wormSize == 0) {
            state = state_finished_level;
            displayCounter = 60*chilopoda_app_config::LEVEL_FINISHED_TIME;
          }
        }

        // Collision of fire with fungi
        for (auto f = fungiList.begin(); f != fungiList.end(); ++f) {
          if (fireSprite.collides_with(**f)) {
            fireSprite.kill();
            (*f)->collide_callback(true);
            switch ((*f)->health) {
            case 3: (*f)->texture = mushroom2Tex; break;
            case 2: (*f)->texture = mushroom3Tex; break;
            case 1: (*f)->texture = mushroom4Tex; break;
            } 
            if (!(*f)->is_enabled()) {
              increase_score(1);
              f = fungiList.erase(f);
            }
            play_sound(mushroomExplodeSound);
            break;
          }
        }

        if (fireSprite.y >= chilopoda_app_config::SCREEN_SIZE*0.5f+10.0f) {
          fireSprite.kill();
        }
      }
    }

    void move_worms(bool hasInteraction) {
      for (auto w = wormList.begin(); w != wormList.end(); ++w) {
        worm_sprite *wSprite = *w;

        //Collision with screen
        if (wSprite->collides_with_screen()) {
            if (wSprite->collidedDirections & 
              (chilo_sprite::COLLIDE_LEFT | chilo_sprite::COLLIDE_RIGHT)) {
                wSprite->followVerticalDirection();
            }
        }

        // Detect possible collisions with fungi 
        for (auto f = fungiList.begin(); f != fungiList.end(); ++f) {

          if (wSprite->direction == worm_sprite::direction_left ||
            wSprite->direction == worm_sprite::direction_right) {
              if (wSprite->collides_with(**f)) {
                if (wSprite->direction == worm_sprite::direction_left &&
                  wSprite->collidedDirections & chilo_sprite::COLLIDE_LEFT) {
                    wSprite->followVerticalDirection();
                    break;
                }
                if (wSprite->direction == worm_sprite::direction_right &&
                  wSprite->collidedDirections & chilo_sprite::COLLIDE_RIGHT) {
                    wSprite->followVerticalDirection();
                    break;
                }
              }
          } else {
            worm_sprite::direction_t previousDirection = wSprite->direction;
            wSprite->followVerticalDirection();
            worm_sprite::direction_t newDirection = wSprite->direction;
          } 
        }                

        wSprite->move();

        // Kill player if worm collides.
        // hasInteraction allows to show a demo screen when state = state_idle
        if (hasInteraction && playerSprite.collides_with(*wSprite)) {
          kill_player();
        } else if (!hasInteraction && playerSprite.collides_with(*wSprite)) {
          wSprite->followVerticalDirection();
        }

      }
    }

    /* Returns the first non-enabled sprite from the specified group.
    * If all sprites in group are enabled, function will return NULL.
    */
    chilo_sprite *get_first_sprite_available_from_group(dynarray<chilo_sprite *> &group) {
      for (int i = 0; i != group.size(); i++) {
        if (!group[i]->is_enabled()) {
          chilo_sprite *spr = group[i];
          spr->init(0.0f, 0.0f, 16.0f, 16.0f);
          return spr;
        }
      }

      return NULL;
    }

    void choose_colors() {
      float h = float(rand())/RAND_MAX*360.0f;
      float s = 0.5f + float(rand())/RAND_MAX*0.5f;
      float v = 0.8f;

      color1 = color::from_HSV(h, s, v);
      h += 120.0f;
      if (h > 360.0f) h -= 360.0f;
      color2 = color::from_HSV(h, s, v);
      h += 120.0f;
      if (h > 360.0f) h -= 360.0f;
      color3 = color::from_HSV(h, s, v);
    }

    void increase_score(int scr_) {
      score += scr_;
      printf("Score: %d\n", score);
    }

    void reset_spider_counter() {
      spiderCounter = chilopoda_app_config::SPIDER_APPEREANCE_BASE_TIME*60 -
            (chilopoda_app_config::SPIDER_APPEREANCE_DEVIATION*(float(rand())/RAND_MAX - 0.5f));
    }

    void kill_player() {
      playerSprite.kill();
      play_sound(playerDiesSound);
      displayCounter = 60*chilopoda_app_config::PLAYER_DIED_TIME;
      explosionSprite.init(playerSprite.x, playerSprite.y, 29.0f, 15.0f, explosion1Tex);
      state = state_died;
    }

  public:
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

      choose_colors();

      cameraToWorld.translate(0, 0, chilopoda_app_config::SCREEN_SIZE*0.5f);

      //overlay.init();

      initGame(); 
      resetGame(true);
      state = state_idle;
    }

    /* Initialize game objects, this is initialized once per application */
    void initGame() {
      playerTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Ship.gif");
      mushroom1Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom1.gif");
      mushroom2Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom2.gif");
      mushroom3Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom3.gif");
      mushroom4Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Mushroom4.gif");
      monster1Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Monster1.gif");
      monster2Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Monster2.gif");
      monster3Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Monster3.gif");
      monster4Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Monster4.gif");
      explosion1Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Explosion1.gif");
      explosion2Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Explosion2.gif");
      explosion3Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Explosion3.gif");
      explosion4Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Explosion4.gif");
      laserTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Laser.gif");
      blamTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Blam.gif");
      gridTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/grid.gif");
      gameoverTex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/gameover.gif");
      spider1Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Spider1.gif");
      spider2Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Spider2.gif");
      spider3Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Spider3.gif");
      spider4Tex = resources::get_texture_handle(GL_RGBA, "assets/chilopoda/Spider4.gif");

      gridSprite.init(0, 0, 512, 512, gridTex);
      playerSprite.init(0, -200, 16.0f, 16.0f, playerTex);
      fireSprite.init(0, 0, 3.0f, 10.0f, laserTex);
      fireSprite.kill();
      blamSprite.init(0, 0, 16.0f, 16.0f, blamTex);
      blamSprite.kill();
      explosionSprite.init(0, 0, 29.0f, 15.0f, explosion1Tex);
      explosionSprite.kill();
      gameoverSprite.kill();
      spiderSprite.kill();

      for (int i = 0; i != chilopoda_app_config::INITIAL_LIVES; i++) {
        chilo_sprite lSpr;
        lSpr.init(-256+10+i*16.0f, 256-10, 12.0f, 12.0f, playerTex);
        lSpr.kill();
        livesGroup.push_back(lSpr);
      }

      for (int i = 0; i != chilopoda_app_config::MAX_WORM_SIZE; i++) {
        worm_sprite *w = new worm_sprite;
        w->init(-256.0f+(i%32)*16.0f+8.0f, -32.0f-(16*floor(i/32.0f))+8.0f, 16.0f, 16.0f, monster1Tex);
        w->speed = chilopoda_app_config::WORM_SPEED;
        w->kill();
        wormSpriteGroup.push_back(w);
      }

      for (int i = 0; i != chilopoda_app_config::MAX_FUNGUS_SIZE; i++) {
        fungus_sprite *fung = new fungus_sprite;
        fung->init(-256.0f, -256.0f, 16.0f, 16.0f, mushroom1Tex);
        fung->kill();
        fungusSpriteGroup.push_back(fung);
      }

      laserSound = resources::get_sound_handle(AL_FORMAT_MONO16, "assets/chilopoda/laser.wav");
      mushroomExplodeSound = resources::get_sound_handle(AL_FORMAT_MONO16, "assets/chilopoda/mushroomexplode.wav");
      playerDiesSound = resources::get_sound_handle(AL_FORMAT_MONO16, "assets/chilopoda/playerdies.wav");
      wormExplodeSound = resources::get_sound_handle(AL_FORMAT_MONO16, "assets/chilopoda/wormexplode.wav"); 
      cur_source = 0;
      alGenSources(num_sound_sources, sources);

      state = state_idle;
      score = 0;
      level = 0;
      lives = chilopoda_app_config::INITIAL_LIVES;
      displayCounter = 0;
      blamCounter = 0;
      spiderCounter = 1;
    }

    /* Starts a new game */
    void resetGame(bool resetAll = false) {
      //clear game objects
      for (auto w = wormList.begin(); w != wormList.end(); w = wormList.begin()) {
        (*w)->kill();
        w = wormList.erase(w);
      }

      if (resetAll) {
        level = 1;
        for (auto f = fungiList.begin(); f != fungiList.end(); f = fungiList.begin()) {
          (*f)->kill();
          f = fungiList.erase(f);
        }
      }

      playerSprite.init(0, -200, 16.0f, 16.0f, playerTex);

      for (int i = 0; i != chilopoda_app_config::INITIAL_WORM_SIZE+level; i++) {
        worm_sprite *w = static_cast<worm_sprite *>(get_first_sprite_available_from_group(wormSpriteGroup));
        if (!w) {
          printf("ERROR: Out of sprites in wormSpriteGroup.\n");
        }
        w->init(from_tile_position_to_screen_position(30.0f-i),
          from_tile_position_to_screen_position(32.0f),
          16.0f, 16.0f, monster1Tex,
          worm_sprite::direction_right);
        wormList.push_back(w);
      }

      worm_sprite::texFrame = 0;

      if (resetAll) {
        for (int i = 0; i != chilopoda_app_config::INITIAL_FUNGUS_SIZE; i++) {
          fungus_sprite *f = static_cast<fungus_sprite *>(get_first_sprite_available_from_group(fungusSpriteGroup));
          float xRandom = floor((float(rand())/RAND_MAX)*31.0f);
          float yRandom = 5.0f + floor((float(rand())/RAND_MAX)*(32.0f-5.0f));
          f->init(from_tile_position_to_screen_position(xRandom),
            from_tile_position_to_screen_position(yRandom),
            16.0f, 16.0f, mushroom1Tex, chilopoda_app_config::FUNGUS_HEALTH);
          if (!f) {
            printf("ERROR: Out of sprites in fungusSpriteGroup.\n");
          }
          fungiList.push_back(f); 
        }
        score = 0;
        lives = chilopoda_app_config::INITIAL_LIVES;

        for (int i = 0; i != lives; i++) {
          livesGroup[i].init(-256+10+i*16.0f, 256-10, 12.0f, 12.0f, playerTex);
        }
      }
      state = state_playing;
      displayCounter = 0;
      blamCounter = 0;
      reset_spider_counter();
      spiderSprite.kill();

      blamSprite.kill();
      explosionSprite.kill();
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

      float c1[3] = {color1.r, color1.g, color1.b};
      float c2[3] = {color2.r, color2.g, color2.b};
      float c3[3] = {color3.r, color3.g, color3.b};

      gridSprite.render(texture_palette_shader_, cameraToWorld, c1, c2, c3, state == state_idle? 0.35f: 0.05f);
      if (playerSprite.is_enabled()) {
        playerSprite.render(texture_palette_shader_, cameraToWorld, c1, c2, c3);
      }

      for (auto f = fungiList.begin(); f != fungiList.end(); ++f) {
        (*f)->render(texture_palette_shader_, cameraToWorld, c1, c2, c3);
      }

      for (auto w = wormList.begin(); w != wormList.end(); ++w) {
        switch (worm_sprite::texFrame) {
        case 0: (*w)->texture = monster1Tex; break;
        case 10: (*w)->texture = monster2Tex; break;
        case 20: (*w)->texture = monster3Tex; break;
        case 30: (*w)->texture = monster4Tex; break;
        case 40: (*w)->texture = monster3Tex; break;
        case 50: (*w)->texture = monster2Tex; break;
        }
        (*w)->render(texture_palette_shader_, cameraToWorld, c1, c2, c3);
      }

      if (fireSprite.is_enabled()) {
        fireSprite.render(texture_palette_shader_, cameraToWorld, c1, c2, c3);
      }

      if (spiderSprite.is_enabled()) {
        spiderSprite.render(texture_palette_shader_, cameraToWorld, c1, c2, c3);
      }

      if (blamSprite.is_enabled()) {
        blamSprite.render(texture_palette_shader_, cameraToWorld, c1, c2, c3);
      }

      if (explosionSprite.is_enabled()) {
        switch (displayCounter) {
        case 60*chilopoda_app_config::PLAYER_DIED_TIME-15: explosionSprite.texture = explosion2Tex; break;
        case 60*chilopoda_app_config::PLAYER_DIED_TIME-30: explosionSprite.texture = explosion3Tex; break;
        case 60*chilopoda_app_config::PLAYER_DIED_TIME-45: explosionSprite.texture = explosion4Tex; break;
        case 60*chilopoda_app_config::PLAYER_DIED_TIME-60: explosionSprite.kill(); break;
        }
        explosionSprite.render(texture_palette_shader_, cameraToWorld, c1, c2, c3);
      }

      if (gameoverSprite.is_enabled()) {
        gameoverSprite.render(texture_palette_shader_, cameraToWorld, c1, c2, c3);
      }

      if (state != state_idle) {
        for (int i = 0; i != chilopoda_app_config::INITIAL_LIVES; i++) {
          if (livesGroup[i].is_enabled()) {
            livesGroup[i].render(texture_palette_shader_, cameraToWorld, c2, c2, c2);
          }
        }
      }

      //int vx = 0, vy = 0;
      //get_viewport_size(vx, vy);

      //overlay.render(object_shader, skin_shader, vx, vy, get_frame_number());
    }

    void fire() {
      if (fireSprite.is_enabled()) {
        return;
      }
      fireSprite.init(playerSprite.x, playerSprite.y+8.0f, 3.0f, 10.0f);
      play_sound(laserSound);
    }
  };
}
