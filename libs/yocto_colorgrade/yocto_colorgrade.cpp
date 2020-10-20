//
// Implementation for Yocto/Grade.
//

//
// LICENSE:
//
// Copyright (c) 2020 -- 2020 Fabio Pellacini
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include "yocto_colorgrade.h"
#include <iostream>
#include <yocto/yocto_color.h>
#include <yocto/yocto_sampling.h>
using namespace std;
// -----------------------------------------------------------------------------
// COLOR GRADING FUNCTIONS
// -----------------------------------------------------------------------------
namespace yocto {

image<vec4f> grade_image(const image<vec4f>& img, const grade_params& params) {
  // PUT YOUR CODE HERE
    // 1855877

  auto  rng    = make_rng(172784);
  auto  graded = img;
  float x, y, z;
  auto  size   = img.imsize();
  int   height = size[1];
  int   width  = size[0];

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {

    graded[{i, j}] = graded[{i,j}] * pow(2,params.exposure); // exposure compensation 
    if (params.filmic) {
        graded[{i, j}] *= 0.6;
        graded[{i, j}] = (pow(graded[{i, j}], 2) * 2.51 + graded[{i, j}] *0.03) / (pow(graded[{i, j}], 2) * 2.43 + graded[{i,j}] * 0.59 + 0.14);
    }
    if (params.srgb) {
      graded[{i, j}]    = pow(graded[{i, j}], (1 / 2.2));  // srgb colorspace
    }
    graded[{i, j}] = clamp(graded[{i, j}], 0, 1); // clamp

    vec4f tinta = {params.tint[0], params.tint[1], params.tint[2], 1};
    graded[{i, j}] = graded[{i, j}] * tinta; // color tint

    float g = (graded[{i, j}][0] + graded[{i, j}][1] + graded[{i, j}][2]) / 3;
    graded[{i, j}] = g + (graded[{i, j}] - g) * (params.saturation * 2); //saturation 

    x = graded[{i, j}].x;
    y = graded[{i, j}].y;
    z = graded[{i, j}].z;

    graded[{i, j}].x = gain(x, 1 - params.contrast);
    graded[{i, j}].y = gain(y, 1 - params.contrast);
    graded[{i, j}].z = gain(z, 1 - params.contrast); // contrast
    float vr = 1 - params.vignette;
    vec2f  ij = {i, j};
    vec2f  wh = {width, height};
    float r = length(ij - (wh / 2)) / length(wh / 2);
    graded[{i, j}]   = graded[{i, j}] * (1 - smoothstep(vr, 2 * vr, r));

    graded[{i, j}] = graded[{i, j}] + (rand1f(rng) - 0.5) * params.grain;
    graded[{i, j}][3] = 1;
    if (params.mosaic != 0) {
      graded[{i, j}] = graded[{i - i % params.mosaic, j - j % params.mosaic}];
    }
    // --- FILTRI AGGIUNTIVI ---
    if (params.heatmap) {
      // HEATMAP
      vec2f size       = {width, height};
      vec2f ij         = {i, j};
      vec2f uv         = ij / size;
      vec3f color      = xyz(eval_image(graded, uv));
      float greyValue  = color.x * 0.29 + color.y * 0.6 + color.z * 0.11;
      float multiplier = 1.1;
      vec3f heat;
      heat.x = smoothstep(0.5, 0.8, greyValue * multiplier);
      if (greyValue * multiplier >= 0.90) {
        heat.x *= (1.1 - greyValue * multiplier) * 5.0;
      }
      if (greyValue * multiplier > 0.7) {
        heat.y = smoothstep(1.0, 0.7, greyValue * multiplier);
      } else {
        heat.y = smoothstep(0.0, 0.7, greyValue * multiplier);
      }
      heat.z = smoothstep(1.0, 0.0, greyValue * multiplier);
      if (greyValue * multiplier <= 0.3) {
        heat.z *= greyValue * multiplier / 0.3;
      }
      graded[{i, j}] = {heat.x, heat.y, heat.z, 1};
    }
    if (params.workshop1) {
      float temp = (graded[{i, j}][0] + graded[{i, j}][1] + graded[{i, j}][2]) / 2.5;
      graded[{i, j}] = temp - (graded[{i, j}] - temp) * 3;
    }
    if (params.workshop2) {
      float g = (graded[{i, j}][0] + graded[{i, j}][1] + graded[{i, j}][2]) / 3;
      graded[{i, j}] = g + (graded[{i, j}] - g) * (params.saturation * 5);
    }
    if (params.people1) {
      graded[{i, j}] = gain(graded[{i, j}], 0.2 - params.saturation);
    }
    if (params.negative) {
       graded[{i, j}].x = 1 - graded[{i, j}].x;
       graded[{i, j}].y = 1 - graded[{i, j}].y;
       graded[{i, j}].z = 1 - graded[{i, j}].z;
    }
      
     
    }
  }
  
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      if (params.grid != 0) {
        graded[{i, j}] = (0 == i % params.grid || 0 == j % params.grid)
                             ? 0.5 * graded[{i, j}]
                             : graded[{i, j}];  // grid
        graded[{i, j}][3] = 1;
      }
    }





  }

  return graded;
}


}  // namespace yocto
