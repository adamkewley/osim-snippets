/* -------------------------------------------------------------------------- *
 *            Simbody(tm) Adhoc Test: Cable Over Bicubic Surfaces             *
 * -------------------------------------------------------------------------- *
 * This is part of the SimTK biosimulation toolkit originating from           *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org/home/simbody.  *
 *                                                                            *
 * Portions copyright (c) 2012 Stanford University and the Authors.           *
 * Authors: Michael Sherman, Andreas Scholz                                   *
 * Contributors:                                                              *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

/*                     Simbody OpenSimPartyDemoCable
THIS DOESN'T WORK YET */

#include "Simbody.h"

#include <cassert>
#include <iostream>
using std::cout; using std::endl;

using namespace SimTK;

// This gets called periodically to dump out interesting things about
// the cables and the system as a whole. It also saves states so that we
// can play back at the end.
static Array_<State> saveStates;
class ShowStuff : public PeriodicEventReporter {
public:
    ShowStuff(const MultibodySystem& mbs, 
              const CableSpring& cable1, Real interval) 
    :   PeriodicEventReporter(interval), 
        mbs(mbs), cable1(cable1) {}

    static void showHeading(std::ostream& o) {
        printf("%8s %10s %10s %10s %10s %10s %10s %10s %10s %12s\n",
            "time", "length", "rate", "integ-rate", "unitpow", "tension", "disswork",
            "KE", "PE", "KE+PE-W");
    }

    /** This is the implementation of the EventReporter virtual. **/ 
    void handleEvent(const State& state) const override {
        const CablePath& path1 = cable1.getCablePath();
        printf("%8g %10.4g %10.4g %10.4g %10.4g %10.4g %10.4g %10.4g %10.4g %12.6g CPU=%g\n",
            state.getTime(),
            path1.getCableLength(state),
            path1.getCableLengthDot(state),
            path1.getIntegratedCableLengthDot(state),
            path1.calcCablePower(state, 1), // unit power
            cable1.getTension(state),
            cable1.getDissipatedEnergy(state),
            mbs.calcKineticEnergy(state),
            mbs.calcPotentialEnergy(state),
            mbs.calcEnergy(state)
                + cable1.getDissipatedEnergy(state),
            cpuTime());
        saveStates.push_back(state);
    }
private:
    const MultibodySystem&  mbs;
    CableSpring             cable1;
};

int main() {
    // system setup
    MultibodySystem system;
    system.setUseUniformBackground(true);

    // subsystem setup
    SimbodyMatterSubsystem matter(system);
    matter.setShowDefaultGeometry(false);
    CableTrackerSubsystem cables(system);
    GeneralForceSubsystem forces(system);

    // global forces/dampeners
    Force::Gravity gravity(forces, matter, -YAxis, 9.81);
    Force::GlobalDamper(forces, matter, 5);


    // Read in some bones
    //
    // - The femur is joined to the tibia via a Pin joint
    PolygonalMesh femur;
    femur.loadVtpFile("CableOverBicubicSurfaces-femur.vtp");
    femur.scaleMesh(30);
    Body::Rigid pendulumBodyFemur(
                MassProperties(1.0, Vec3(0, -5, 0),
                UnitInertia(1).shiftFromCentroid(Vec3(0, 5, 0))));
    pendulumBodyFemur.addDecoration(
                Transform(),
                DecorativeMesh(femur).setColor(Vec3(0.8, 0.8, 0.8)));
    MobilizedBody::Pin groundToFemurPin(
                matter.updGround(),
                Transform(Vec3(0, 0, 0)),
                pendulumBodyFemur,
                Transform(Vec3(0, 0, 0)));


    PolygonalMesh tibia;
    tibia.loadVtpFile("CableOverBicubicSurfaces-tibia.vtp");
    tibia.scaleMesh(30);
    Body::Rigid pendulumBodyTibia(
                MassProperties(1.0, Vec3(0, -5, 0),
                UnitInertia(1).shiftFromCentroid(Vec3(0, 5, 0))));
    pendulumBodyTibia.addDecoration(
                Transform(),
                DecorativeMesh(tibia).setColor(Vec3(0.8, 0.8, 0.8)));
    MobilizedBody::Pin femurToTibiaPin(
                groundToFemurPin,
                Transform(Rotation(-Pi/4, ZAxis), Vec3(0, -12, 0)),
                pendulumBodyTibia,
                Transform(Vec3(0, 0, 0)));


    Constraint::PrescribedMotion prescribedMotion(
                matter,
                new Function::Sinusoid(0.25*Pi, 0.2*Pi, 0.0),
                femurToTibiaPin,
                MobilizerQIndex(0));

    // Build a wrapping cable path
    CablePath cablePath(
                cables,
                matter.Ground(),
                Vec3(1, 3, 1),      // origin
                femurToTibiaPin,
                Vec3(1, -4, 0));    // termination

    // Create a bicubic surface
    BicubicSurface patch;
    {
        std::array<Real, 4> xdata = {-2, -1, 1, 2};
        std::array<Real, 4> ydata = xdata;
        std::array<Real, xdata.size() * ydata.size()> fData = {
                    2,   3,   3,   1,
                    0,   1.5, 1.5, 0,
                    0,   1.5, 1.5, 0,
                    2,   3,   3,   1,
        };

        Vector x = 2.00 * Vector(xdata.size(), xdata.data());
        Vector y = 2.00 * Vector(ydata.size(), ydata.data());
        Matrix f = 0.75 * Matrix(xdata.size(), ydata.size(), fData.data());

        patch = BicubicSurface(x, y, f, 0);
    }

    Transform patchTransform(
                Rotation(0.5*Pi, ZAxis) * Rotation(0.2*Pi, XAxis) * Rotation(0.5*Pi, ZAxis),
                Vec3(0, -5, -1));

    // Handle surface mesh rendering
    {
        Real highRes = 30;
        Real lowRes  = 1;

        PolygonalMesh highResPatchMesh = patch.createPolygonalMesh(highRes);
        PolygonalMesh lowResPatchMesh = patch.createPolygonalMesh(lowRes);

        groundToFemurPin.addBodyDecoration(
                    patchTransform,DecorativeMesh(highResPatchMesh).setColor(Cyan).setOpacity(.75));

        groundToFemurPin.addBodyDecoration(
                    patchTransform,
                    DecorativeMesh(lowResPatchMesh).setRepresentation(DecorativeGeometry::DrawWireframe));
    }

    // Use the surface as an obstacle, with P and Q as wrapping "hints"
    Vec3 patchP(-0.5, -1, 2);
    groundToFemurPin.addBodyDecoration(
                patchTransform,
                DecorativePoint(patchP).setColor(Green).setScale(2));

    Vec3 patchQ(-0.5,  1, 2);
    groundToFemurPin.addBodyDecoration(
                patchTransform,
                DecorativePoint(patchQ).setColor(Red).setScale(2));

    CableObstacle::Surface patchObstacle(
                cablePath,
                groundToFemurPin,
                patchTransform,
                ContactGeometry::SmoothHeightMap(patch));
    patchObstacle.setContactPointHints(patchP, patchQ);
    patchObstacle.setDisabledByDefault(true);


    // Create a sphere obstacle
    Real sphRadius = 1.5;
    Vec3 sphOffset(0, -0.5, 0);
    Rotation  sphRotation(0*Pi, YAxis);
    Transform sphTransform(sphRotation, sphOffset);

    CableObstacle::Surface tibiaSphere(
                cablePath,
                femurToTibiaPin,
                sphTransform,
                ContactGeometry::Sphere(sphRadius));

    Vec3 sphP(1.5,-0.5,0);
    Vec3 sphQ(1.5,0.5,0);
    tibiaSphere.setContactPointHints(sphP, sphQ);

    femurToTibiaPin.addBodyDecoration(
                sphTransform,
                DecorativeSphere(sphRadius).setColor(Red).setOpacity(0.5));


    // Create a wrapping cable as a spring
    CableSpring cable2(forces, cablePath, 50., 18., 0.1);


    // Model setup complete: initialize rest of system

    Visualizer viz(system);
    viz.setShowFrameNumber(true);
    system.addEventReporter(new Visualizer::Reporter(viz, 1./30));
    system.addEventReporter(new ShowStuff(system, cable2, 0.02));

    system.realizeTopology();
    State state = system.getDefaultState();
    system.realize(state, Stage::Position);
    viz.report(state);
    cout << "path2 init length=" << cablePath.getCableLength(state) << endl;
    cout << "Hit ENTER ...";
    //getchar();

    // path1.setIntegratedCableLengthDot(state, path1.getCableLength(state));

    // Simulate it.
    saveStates.clear(); saveStates.reserve(2000);

    // RungeKutta3Integrator integ(system);
    RungeKuttaMersonIntegrator integ(system);
    // CPodesIntegrator integ(system);
    // integ.setAllowInterpolation(false);
    integ.setAccuracy(1e-5);
    TimeStepper ts(system, integ);
    ts.initialize(state);
    ShowStuff::showHeading(cout);

    const Real finalTime = 10;
    const double startTime = realTime();
    ts.stepTo(finalTime);
    cout << "DONE with " << finalTime
         << "s simulated in " << realTime()-startTime
         << "s elapsed.\n";

    while (true) {
        cout << "Hit ENTER FOR REPLAY, Q to quit ...";
        const char ch = getchar();
        if (ch=='q' || ch=='Q') break;
        for (unsigned i=0; i < saveStates.size(); ++i)
            viz.report(saveStates[i]);
    }
}
