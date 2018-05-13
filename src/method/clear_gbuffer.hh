#ifndef METHOD_CLEAR_GBUFFER_HH
#define METHOD_CLEAR_GBUFFER_HH
#include "pipeline.hh"

class gbuffer;

namespace method
{
    class clear_gbuffer: public pipeline_method
    {
    public:
        clear_gbuffer(gbuffer& gbuf);
        ~clear_gbuffer();

        void execute() override;

        std::string get_name() const override;

    private:
        gbuffer* gbuf;
    };
}

#endif

