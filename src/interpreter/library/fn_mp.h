FNDEF3(move_towards_point, x, y, speed)
FNDEF2(motion_set, dir, speed)
FNDEF2(motion_add, dir, speed)

// NEW FEATURE (signature: annika marie schlögel)
FNDEF2(move_contact_solid, dir, maxdist)
FNDEF2(move_contact_all, dir, maxdist)
FNDEF2(move_outside_solid, dir, maxdist)
FNDEF2(move_outside_all, dir, maxdist)
// NEW FEATURE END

// TODO: mp_* functions.
FNDEF4(mp_linear_step, xgoal, ygoal, stepsize, all)
