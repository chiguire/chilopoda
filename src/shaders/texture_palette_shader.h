////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Single texture shader for textures with separate colors for 
// palette switching. Maximum 3 colors.
//

namespace octet {
  class texture_palette_shader : public shader {
    // indices to use with glUniform*()

    // index for model space to projection space matrix
    GLuint modelToProjectionIndex_;

    // index for texture sampler
    GLuint samplerIndex_;

    // index for color palette
    GLuint color1Index_;
    GLuint color2Index_;
    GLuint color3Index_;

  public:
    void init() {
      // this is the vertex shader.
      // it is called for each corner of each triangle
      // it inputs pos and uv from each corner
      // it outputs gl_Position and uv_ to the rasterizer
      const char vertex_shader[] = SHADER_STR(
        varying vec2 uv_;

        attribute vec4 pos;
        attribute vec2 uv;

        uniform mat4 modelToProjection;

        void main() { gl_Position = modelToProjection * pos; uv_ = uv; }
      );

      // this is the fragment shader
      // after the rasterizer breaks the triangle into fragments
      // this is called for every fragment
      // it outputs gl_FragColor, the color of the pixel and inputs uv_
      const char fragment_shader[] = SHADER_STR(
        varying vec2 uv_;
        uniform sampler2D sampler;
        uniform vec3 color1;
        uniform vec3 color2;
        uniform vec3 color3;

        void main() { 
          vec4 texColor = texture2D(sampler, uv_);
          gl_FragColor = vec4(
            (texColor.r*color1.r)+(texColor.g*color2.r)+(texColor.b*color3.r),
            (texColor.r*color1.g)+(texColor.g*color2.g)+(texColor.b*color3.g),
            (texColor.r*color1.b)+(texColor.g*color2.b)+(texColor.b*color3.b),
            texColor.a);
        }
      );
    
      // use the common shader code to compile and link the shaders
      // the result is a shader program
      shader::init(vertex_shader, fragment_shader);

      // extract the indices of the uniforms to use later
      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
      samplerIndex_ = glGetUniformLocation(program(), "sampler");
      color1Index_ = glGetUniformLocation(program(), "color1");
      color2Index_ = glGetUniformLocation(program(), "color2");
      color3Index_ = glGetUniformLocation(program(), "color3");
    }

    void render(const mat4t &modelToProjection, int sampler, float color1[3], float color2[3], float color3[3]) {
      // tell openGL to use the program
      shader::render();

      // customize the program with uniforms
      glUniform1i(samplerIndex_, sampler);
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
      glUniform3f(color1Index_, color1[0], color1[1], color1[2]);
      glUniform3f(color2Index_, color2[0], color2[1], color2[2]);
      glUniform3f(color3Index_, color3[0], color3[1], color3[2]);
    }
  };
}
