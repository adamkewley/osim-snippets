#include <OpenSim/OpenSim.h>

using namespace SimTK;
using namespace OpenSim;

int main(int, char**) {
    static char const* files[] = {
        "resources/opensim-models/Models/Arm26/arm26.osim",
        "resources/opensim-models/Models/BouncingBlock/bouncing_block.osim"
    };

    Model model{files[0]};
    try {
        Model model{files[0]};
        model.setUseVisualizer(true);
        model.finalizeFromProperties();
        model.finalizeConnections();

        // Configure the model.

        model.buildSystem();
        State& state = model.initSystem();
        model.initializeState();
        model.updMatterSubsystem().setShowDefaultGeometry(false);

        Visualizer& viz = model.updVisualizer().updSimbodyVisualizer();
        viz.setBackgroundType(viz.SolidColor);
        viz.setBackgroundColor(White);
        simulate(model, state, 10.0);


        using std::chrono_literals::operator""s;
        std::this_thread::sleep_for(100s);
    } catch (...) {
        std::cerr << "Visualizing the model failed. This can happen when `simbody-visualizer` cannot find libraries. To fix this, set LD_LIBRARY_PATH of the calling process" << std::endl;
        throw;
    }

    // Simulate.
    //simulate(model, state, 10.0);
    //viz.setShutdownWhenDestructed(true);
    //viz.setCameraClippingPlanes(.01,100.);
    //viz.setWindowTitle("lol");
    //viz.setBackgroundType(viz.SolidColor);
    //viz.setBackgroundColor(White);
    //viz.drawFrameNow(state);
}
