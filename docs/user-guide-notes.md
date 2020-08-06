- low-level system could be described by cartesian coordinates, 6
  degrees of freedom (3x translation, 3x rotation) + constraints

- simbody uses "internal coordinates"
  
  - describe things in a high level

  - e.g. only describe the way in which a skeleton can actually move
  
- homepage: https://simtk.org/home/simbody
- API docs: https://simtk.org/api_docs/simbody/api_docs22/Simbody/html/index.html

- "state vector" e.g. current angle and angular velocity of every
  joint
  
vars:

    q    generalized coordinates    angles for all joints, orientation + posn of torso
    u    generalized speeds    angular/linear velocity of those objects
    z    auxiliary variables    total energy used while walking (accumulator)
    y    [q, u, z]
    
    f    eqns of motion    dy/dt = f(d; t, y)
    c    constraint function    c(d; t, q, u) == 0
    e    event trigger function    e(d; t, y)
    
    d    discrete variables
    
additional constraint e.g. disulfide bond in protein (long-range
linker)

constraint (c) is essentially an algebraic equation which must be
satisfied at each timestep:

   c(t; t, q, u) == 0
   
All of this assumes continuous equations. Real systems are
discontinuous. e.g. collision detection when someone is walking. Their
motion is only continuous if they were floating in space - they must
*hit* the floor, which introduces a discontinuity.

"event trigger functions": functions that trigger a change
(e.g. enable a constraint) when the trigger function passes through 0:

    if (e(d; t, y) `crosses` 0)
        mutate(d, y)
    
"discrete variables": separate variables that do not participate
*directly* in equations of motion step

    # d is held constant while t and y change
    dy/dt == f(d; t, y)
    
Simbody concepts

   system    immutable    everything that is constant during a sim (incl. code)
   state     mutable      time, y, d
   
`state` is a data container. `system` is a logic container. `system`
defines what data is stored in `state` and provides `f`, `c`, and `e`,
event handlers, etc.

`SimTK::System` is actually a blank container that holds
subsystems. e.g. `SimbodyMatterSubsystem` (for making multibody
systems out of collections of joint types), or
`DuMMForceFieldSubsystem` (for molecular force fields).
   

## cache variables

- you might want to calculate derived values from `t`, `y`, and `d`

- that calculation might be expensive

- so you might want to avoid recalculating them more often than
  necessary
  
- The `SimTK::State` object also has a "realization cache", which can
  hold those variables. "realizing the state" is the process of
  computing variables in the "realization cache" from the state.
  
## scheduled events

"scheduled events": special case where `e(d; t, y) == e(t) == t-tevent

- e.g. the event only depends on time

- in contrast to "triggered" events

## event reporter

- An event that does not modify `y` (state) at all


All of the above are inheritable as `TriggeredEventHandler`,
`ScheduledEventHandler`, `TriggeredEventReporter` and
`ScheduledEventReporter`, added via `addEventHandler` or
`addEventReporter`.


# 
