/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LT_PIPELINE_HH
#define LT_PIPELINE_HH
#include "api.hh"
#include "timer.hh"
#include <vector>
#include <string>

namespace lt
{

class LT_API pipeline_method
{
public:
    virtual ~pipeline_method();

    virtual void execute() = 0;
    virtual std::string get_name() const;
};

class render_target;
class LT_API target_method: public pipeline_method
{
public:
    explicit target_method(render_target& target);

    render_target& get_target();

    // Call this from the deriving method with target_method::execute().
    // You can also bind the target yourself if you want to.
    virtual void execute();
private:
    render_target* target;
};

namespace method
{
template<typename For>
struct method_options;
}

#define LT_OPTIONS(For) template<> struct LT_API method_options<class For>

template<typename Derived>
class LT_API options_method
{
public:
    using options = method::method_options<Derived>;

private:
    struct member_detector_general {};
    struct member_detector_special: member_detector_general {};
    template<typename> struct member_detector { typedef int type; };

    struct accessor: Derived
    {
        template<
            typename Self,
            typename member_detector<
                decltype(&Self::options_will_update)
            >::type = 0
        > static void will_update(
            Derived& method,
            const options& opt,
            member_detector_special
        );

        template<typename Self>
        static void will_update(
            Derived&,
            const options&,
            member_detector_general
        );
    };

public:
    options_method(const options& opt);

    void set_options(const options& opt);
    const options& get_options() const;

protected:
    // If you get a compiler warning here about an incomplete type, you've
    // probably derived from options_method without creating an LT_OPTIONS for
    // your method. Also, make sure LT_OPTIONS is created _before_ your method
    // class.
    options opt;
};

class LT_API animated_method
{
public:
    animated_method();
    virtual ~animated_method();

    void set_animation_multiplier(double multiplier);
    double get_animation_multiplier(double multiplier);
    void update(duration delta);

    duration get_time() const;
    double get_time_sec() const;

protected:
    // Be aware that on_update may later be called in parallel; don't use OpenGL
    // and use synchronization if the on_update of other methods may affect
    // this method.
    virtual void on_update(duration delta);

private:
    double multiplier;
    duration total_time;
};

class LT_API pipeline: public pipeline_method
{
public:
    explicit pipeline(const std::vector<pipeline_method*>& methods);
    pipeline(pipeline&& other);
    ~pipeline();

    void execute();
    void execute(std::vector<double>& timing);

    std::string get_name(size_t i) const;
    std::string get_name() const override;
private:
    std::vector<pipeline_method*> methods;
};

} // namespace lt

#include "pipeline.tcc"

#endif
