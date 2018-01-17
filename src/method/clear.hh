#ifndef METHOD_CLEAR_HH
#define METHOD_CLEAR_HH
#include "pipeline.hh"
#include <glm/glm.hpp>

namespace method
{
    class clear
    {
    public:
        clear(
            glm::vec4 color = glm::vec4(0),
            double depth = 1,
            int stencil = 0
        );
        ~clear();

        pipeline_method* pipeline_build(pipeline& p) const;

    private:
        glm::vec4 color;
        double depth;
        int stencil;
    };
}

#endif
