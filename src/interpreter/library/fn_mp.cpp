#include "libpre.h"
    #include "fn_mp.h"
    #include "fn_math.h"
    #include "fn_collision.h"
#include "libpost.h"

#include "ogm/interpreter/Variable.hpp"
#include "ogm/common/error.hpp"
#include "ogm/common/util.hpp"
#include "ogm/interpreter/Executor.hpp"
#include "ogm/interpreter/execute.hpp"
#include "ogm/interpreter/display/Display.hpp"

#include <string>
#include "ogm/common/error.hpp"

#include <cctype>
#include <cstdlib>

using namespace ogm::interpreter;
using namespace ogm::interpreter::fn;

#define frame staticExecutor.m_frame

// NEW FEATURE (signature: annika marie schlögel)
// helpers for the collision functions
enum class MoveMode
{
    Contact,
    Outside
};

enum class CollisionMode
{
    Solid,
    All
};
// NEW FEATURE END

void ogm::interpreter::fn::move_towards_point(VO out, V vx, V vy, V vspeed)
{
    real_t xdst = vx.castCoerce<real_t>();
    real_t ydst = vy.castCoerce<real_t>();
    real_t xsrc = staticExecutor.m_self->m_data.m_position.x;
    real_t ysrc = staticExecutor.m_self->m_data.m_position.y;

    real_t xdiff = xdst - xsrc;
    real_t ydiff = ydst - ysrc;

    real_t len = std::sqrt(xdiff * xdiff + ydiff * ydiff);
    if (len == 0)
    {
        staticExecutor.m_self->m_data.m_speed.x = 0;
        staticExecutor.m_self->m_data.m_speed.y = 0;
    }
    else
    {
        real_t spd = vspeed.castCoerce<real_t>();
        staticExecutor.m_self->m_data.m_speed.x = xdiff / len  * spd;
        staticExecutor.m_self->m_data.m_speed.y = ydiff / len  * spd;
    }
}

void ogm::interpreter::fn::motion_set(VO out, V vdir, V vspeed)
{
    real_t dir = vdir.castCoerce<real_t>();
    real_t spd = vspeed.castCoerce<real_t>();

    real_t xp = std::cos(dir * TAU / 360.0);
    real_t yp = -std::sin(dir * TAU / 360.0);

    staticExecutor.m_self->m_data.m_speed.x = spd * xp;
    staticExecutor.m_self->m_data.m_speed.y = spd * yp;
}

void ogm::interpreter::fn::motion_add(VO out, V vdir, V vspeed)
{
    real_t dir = vdir.castCoerce<real_t>();
    real_t spd = vspeed.castCoerce<real_t>();

    real_t xp = std::cos(dir * TAU / 360.0);
    real_t yp = -std::sin(dir * TAU / 360.0);

    staticExecutor.m_self->m_data.m_speed.x += spd * xp;
    staticExecutor.m_self->m_data.m_speed.y += spd * yp;
}

// NEW FEATURE (signature: annika marie schlögel)
// generalized helper function which is called by:
// move_contact_solid, move_contact_all, move_outside_solid, move_outside_all
static void move_until (MoveMode move_mode, CollisionMode collision_mode, real_t dir, int32_t maxdist)
{
    frame.process_collision_updates();

    if (maxdist <= 0)
    {
        maxdist = 1000;
    }

    Variable dx;
    Variable dy;

    lengthdir_x(dx, Variable(1.0), Variable(dir));
    lengthdir_y(dy, Variable(1.0), Variable(dir));

    real_t stepx = dx.castCoerce<real_t>();
    real_t stepy = dy.castCoerce<real_t>();

    dx.cleanup();
    dy.cleanup();

    bool moved = false;

    for (int32_t i = 0; i < maxdist; ++i)
    {
        real_t xnext = staticExecutor.m_self->m_data.m_position.x + stepx;
        real_t ynext = staticExecutor.m_self->m_data.m_position.y + stepy;

        Variable collision;

        if (collision_mode == CollisionMode::Solid)
        {
            place_free(collision, Variable(xnext), Variable(ynext));
        }
        else
        {
            place_empty(collision, Variable(xnext), Variable(ynext));
        }

        bool free = collision.cond();
        collision.cleanup();

        if (move_mode == MoveMode::Contact)
        {
            if (!free)
            {
                break;
            }

            moved = true;

            staticExecutor.m_self->m_data.m_position.x = xnext;
            staticExecutor.m_self->m_data.m_position.y = ynext;
        }
        else
        {
            moved = true;

            staticExecutor.m_self->m_data.m_position.x = xnext;
            staticExecutor.m_self->m_data.m_position.y = ynext;

            if (free)
            {
                break;
            }
        }
    }

    if (moved)
    {
        frame.queue_update_collision(staticExecutor.m_self);
    }
}

void ogm::interpreter::fn::move_contact_solid(VO out, V vdir, V vmaxdist)
{
    move_until(
        MoveMode::Contact,
        CollisionMode::Solid,
        vdir.castCoerce<real_t>(),
        vmaxdist.castCoerce<int32_t>()
    );
}

void ogm::interpreter::fn::move_contact_all(VO out, V vdir, V vmaxdist)
{
    move_until(
        MoveMode::Contact,
        CollisionMode::All,
        vdir.castCoerce<real_t>(),
        vmaxdist.castCoerce<int32_t>()
    );
}

void ogm::interpreter::fn::move_outside_solid(VO out, V vdir, V vmaxdist)
{
    move_until(
        MoveMode::Outside,
        CollisionMode::Solid,
        vdir.castCoerce<real_t>(),
        vmaxdist.castCoerce<int32_t>()
    );
}

void ogm::interpreter::fn::move_outside_all(VO out, V vdir, V vmaxdist)
{
    move_until(
        MoveMode::Outside,
        CollisionMode::All,
        vdir.castCoerce<real_t>(),
        vmaxdist.castCoerce<int32_t>()
    );
}
// NEW FEATURE END

void ogm::interpreter::fn::mp_linear_step(VO out, V gx, V gy, V vstepsize, V vall)
{
    bool all = vall.cond();
    coord_t stepsize = vstepsize.castCoerce<coord_t>();

    frame.process_collision_updates();
    real_t xdst = gx.castCoerce<real_t>();
    real_t ydst = gy.castCoerce<real_t>();
    real_t xsrc = staticExecutor.m_self->m_data.m_position.x;
    real_t ysrc = staticExecutor.m_self->m_data.m_position.y;

    real_t xdiff = xdst - xsrc;
    real_t ydiff = ydst - ysrc;

    real_t len = std::sqrt(xdiff * xdiff + ydiff * ydiff);
    if (len == 0)
    {
        out = true;
    }
    else
    {
        coord_t xnext = xsrc + xdiff / len * stepsize;
        coord_t ynext = ysrc + ydiff / len * stepsize;
        if (len <= stepsize)
        {
            xnext = xdst;
            ynext = ydst;
        }
        bool take_step = false;
        if (all)
        {
            // check for anything
            place_empty(out, xnext, ynext);
            if (out.cond())
            {
                take_step = true;
            }
        }
        else
        {
            // check for solid
            place_free(out, xnext, ynext);
            if (out.cond())
            {
                take_step = true;
            }
        }

        if (take_step)
        {
            staticExecutor.m_self->m_data.m_position.x = xnext;
            staticExecutor.m_self->m_data.m_position.y = ynext;

            frame.queue_update_collision(staticExecutor.m_self);
        }

        out = (xnext == xdst && ynext == ydst);
    }
}
