#ifndef LT_METHOD_CLEAR_GBUFFER_HH
#define LT_METHOD_CLEAR_GBUFFER_HH
#include "pipeline.hh"

namespace lt
{

class gbuffer;

}

namespace lt::method
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

} // namespace lt::method

#endif

