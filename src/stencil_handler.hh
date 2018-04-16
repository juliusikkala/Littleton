#ifndef STENCIL_HANDLER_HH
#define STENCIL_HANDLER_HH

// To be used by methods for providing an interface for managing what is
// written to the stencil buffer and what is passed.
class stencil_handler
{
public:
    stencil_handler();

    void set_stencil_draw(unsigned value = 1);
    void set_stencil_cull(unsigned ref = 1);

    // These should only be used by the owner of the handler, such as a method
    // deriving from it or one that holds it.
    void stencil_disable();
    void stencil_draw();
    void stencil_cull();

private:
    unsigned value;
    unsigned ref;
};

#endif
