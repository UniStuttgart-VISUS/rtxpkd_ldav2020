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

// ======================================================================== //
// Modified 2019-2020 VISUS - University of Stuttgart                       //
// ======================================================================== //

#include "rtxParticles/OptixParticles.h"
#include "rtxParticles/common/programs/raygen.h"
#include <chrono>
#include <numeric>

namespace pkd {

    extern "C" const char embedded_common_programs[];

    using device::RayGenData;

    int OptixParticles::rec_depth = 3;

    OptixParticles::OptixParticles()
    {
        // create an optix context to render with, and do basic set-up
        context = owlContextCreate();
        module = owlModuleCreate(context, embedded_common_programs);
        
        std::cout << "todo: set stack size" << std::endl;
        
        frameStateBuffer = owlDeviceBufferCreate(context,
            OWL_USER_TYPE(device::FrameState),
            1, nullptr);

        // =======================================================
        // now, let's build up the world, innermost first
        // =======================================================

        // -------------------------------------------------------
        // set up miss prog 
        // -------------------------------------------------------
        OWLVarDecl missProgVars[] = {
          { /* sentinel to mark end of list */ }
        };
        // ........... create object  ............................
        PING; PRINT(module);
        OWLMissProg missProg
            = owlMissProgCreate(context, module, "miss_program", 0,//sizeof(MissProgData),
                missProgVars, -1);


        // -------------------------------------------------------
        // set up ray gen program
        // -------------------------------------------------------
        OWLVarDecl rayGenVars[] = {
          { "deviceIndex",     OWL_DEVICE, OWL_OFFSETOF(RayGenData,deviceIndex)},
          { "deviceCount",     OWL_INT,    OWL_OFFSETOF(RayGenData,deviceCount)},
          { "colorBuffer",     OWL_BUFPTR, OWL_OFFSETOF(RayGenData,colorBufferPtr)},
          { "accumBuffer",     OWL_BUFPTR, OWL_OFFSETOF(RayGenData,accumBufferPtr)},
          { "particleBuffer",  OWL_BUFPTR, OWL_OFFSETOF(RayGenData,particleBuffer)},
          { "frameStateBuffer",OWL_BUFPTR, OWL_OFFSETOF(RayGenData,frameStateBuffer)},
          { "fbSize",          OWL_INT2,   OWL_OFFSETOF(RayGenData,fbSize)},
          { "world",           OWL_GROUP,  OWL_OFFSETOF(RayGenData,world)},
          { "rec_depth",       OWL_INT,    OWL_OFFSETOF(RayGenData,rec_depth)},
          { /* sentinel to mark end of list */ }
        };

        // ........... create object  ............................
        this->rayGen
            = owlRayGenCreate(context, module, "raygen_program",
                sizeof(RayGenData),
                rayGenVars, -1);

        std::cout << "Setting rec depth: " << rec_depth << std::endl;
        owlRayGenSet1i(rayGen, "rec_depth", rec_depth);
        owlRayGenSetBuffer(rayGen, "frameStateBuffer", frameStateBuffer);

        resizeFrameBuffer(vec2i(100));
    }

    void OptixParticles::setModel(Model::SP model, bool override_model)
    {
        buildModel(model, override_model);
        owlRayGenSetGroup(rayGen, "world", world);
        owlRayGenSetBuffer(rayGen, "particleBuffer", particleBuffer);

        std::cout << "building programs" << std::endl;
        owlBuildPrograms(context);
        std::cout << "building pipeline" << std::endl;
        owlBuildPipeline(context);
        owlBuildSBT(context);
    }

    void OptixParticles::render()
    {
        owlRayGenLaunch2D(rayGen, fbSize.x, fbSize.y);
    }

    void OptixParticles::resizeFrameBuffer(const vec2i& newSize)
    {
        fbSize = newSize;
        if (!accumBuffer)
            accumBuffer = owlDeviceBufferCreate(context, OWL_FLOAT4, fbSize.x * fbSize.y, nullptr);
        
        owlBufferResize(accumBuffer, fbSize.x * fbSize.y);
        
        owlRayGenSetBuffer(rayGen, "accumBuffer", accumBuffer);
        owlRayGenSet1i(rayGen, "deviceCount", owlGetDeviceCount(context));

        if (!colorBuffer)
            colorBuffer = owlHostPinnedBufferCreate(context, OWL_INT, fbSize.x * fbSize.y);
        
        owlBufferResize(colorBuffer, fbSize.x * fbSize.y);
        
        owlRayGenSetBuffer(rayGen, "colorBuffer", colorBuffer);
        owlRayGenSet2i(rayGen, "fbSize", fbSize.x, fbSize.y);
    }

    void OptixParticles::updateFrameState(device::FrameState& fs)
    {
        owlBufferUpload(frameStateBuffer, &fs);
    }

    uint32_t* OptixParticles::mapColorBuffer()
    {
        if (!colorBuffer) return nullptr;
        return (uint32_t*)owlBufferGetPointer(colorBuffer, 0);
    }

    void OptixParticles::unmapColorBuffer()
    {
        assert(colorBuffer);
    }
}
