#include "Simbody.h"

using namespace SimTK;

int main() {
    auto system = MultibodySystem{};
    auto matter = SimbodyMatterSubsystem{system};
    auto forces = GeneralForceSubsystem{system};
    auto cables = CableTrackerSubsystem{system};

    auto gravity = Force::UniformGravity{
            forces,
            matter,
            Vec3{0, -9.8, 0}
    };

    auto body = Body::Rigid{
        MassProperties{
            1.0,  // kg
            Vec3{0, 0, 0},  // center of mass
            Inertia{1}
        }
    };
    body.addDecoration(
        Transform{},
        DecorativeSphere{0.1}
    );

    auto pendulum_1 = MobilizedBody::Pin{
        matter.Ground(),
        Transform{Vec3{0, 0, 0}},
        body,
        Transform{Vec3{1, 0, 0}}
    };
    auto pendulum_2 = MobilizedBody::Pin{
        pendulum_1,
        Transform{Vec3{0, 0, 0}},
        body,
        Transform{Vec3{1, 0, 0}}
    };


    auto cable_path = CablePath{
        cables,
        pendulum_1,
        Vec3{0, 0, 0},
        pendulum_2,
        Vec3{0, 0, 0}
    };
    auto obstable_geometry = ContactGeometry::Cylinder{0.1};
    auto obstable = CableObstacle::Surface{
        cable_path,
        pendulum_1,
        Rotation(Pi/2, Vec3{1.0, 0, 1.0}),
        obstable_geometry
    };


    // set up visualization
    system.setUseUniformBackground(true);
    auto visualizer = Visualizer{system};
    {
        auto visualizer_reporter = new Visualizer::Reporter{
                visualizer,
                0.01  // sampling rate
        };
        system.addEventReporter(visualizer_reporter);
    }


    system.realizeTopology();
    State state = system.getDefaultState();

    // set rotational velocity of the 2nd pendulum
    pendulum_2.setRate(state, 5.0);

    // Simulate it.
    RungeKuttaMersonIntegrator integ(system);
    TimeStepper ts(system, integ);
    ts.initialize(state);
    ts.stepTo(50.0);

    return 0;
}
