#include <OpenSim/OpenSim.h>

using namespace SimTK;
using namespace OpenSim;

int main(int, char**) {
    static char const* files[] = {
        "/home/adam/Desktop/osim-snippets/opensim-models/Models/Arm26/arm26.osim",
        "/home/adam/Desktop/osim-snippets/opensim-models/Models/BouncingBlock/bouncing_block.osim"
    };

    Model model{files[0]};
    model.setUseVisualizer(true);
    model.finalizeFromProperties();
    model.finalizeConnections();

    // Configure the model.

    model.buildSystem();
    State& state = model.initSystem();
    model.initializeState();
    model.updMatterSubsystem().setShowDefaultGeometry(false);

    //SimTK::Visualizer viz{model.getMultibodySystem()};
    Visualizer& viz = model.updVisualizer().updSimbodyVisualizer();
    viz.setBackgroundType(viz.SolidColor);
    viz.setBackgroundColor(White);
    simulate(model, state, 10.0);

    // Simulate.
    //simulate(model, state, 10.0);
    //viz.setShutdownWhenDestructed(true);
    //viz.setCameraClippingPlanes(.01,100.);
    //viz.setWindowTitle("lol");
    //viz.setBackgroundType(viz.SolidColor);
    //viz.setBackgroundColor(White);
    //viz.drawFrameNow(state);


    using std::chrono_literals::operator""s;
    std::this_thread::sleep_for(100s);
}
