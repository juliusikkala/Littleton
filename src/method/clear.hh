#ifndef METHOD_CLEAR_HH
#define METHOD_CLEAR_HH
#include "pipeline.hh"
#include "math.hh"

namespace method
{
    class clear: public target_method
    {
    public:
        clear(
            render_target& target,
            glm::vec4 color = glm::vec4(0),
            double depth = 1,
            int stencil = 0
        );
        ~clear();

        void execute() override;

        std::string get_name() const override;

    private:
        glm::vec4 color;
        double depth;
        int stencil;
    };
}

#endif
