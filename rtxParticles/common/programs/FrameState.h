// ======================================================================== //
// Copyright 2018-2019 Ingo Wald                                            //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "owl/common/math/box.h"

namespace pkd {
  namespace device {
    
    using namespace owl;
    using namespace owl::common;
  
    /*! keeps all information required for the raygen program to
      render one frame */
    struct FrameState {
      /*! camera setup ...*/
      vec3f camera_screen_du;
      vec3f camera_screen_dv;
      vec3f camera_screen_00;
      vec3f camera_lens_center;
      vec3f camera_lens_du;
      vec3f camera_lens_dv;
      /*! accumulation id, for progressive refinement */
      int   accumID;
      /*! shading mode, in case we support different modes */
      int   shadeMode { 0 };
      /*! num samples per pixel */
      int   samplesPerPixel { 1 };

      bool heatMapEnabled { 0 };
      float heatMapScale = 1.f;
      vec2i dbgPixel { 400,400};
    };

  }
}

