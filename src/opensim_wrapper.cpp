#include "opensim_wrapper.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <OpenSim/OpenSim.h>

using namespace SimTK;
using namespace OpenSim;

namespace {
    /*
    struct MutableModel : public Model {
        MutableModel() : Model{} {}
        void load_file(std::string const& s) {
            XMLDocument* d = new XMLDocument(s);
            setDocument(d);
            const std::string saveWorkingDirectory = IO::getCwd();
            const std::string directoryOfXMLFile = IO::getParentDirectory(s);
            IO::chDir(directoryOfXMLFile);
            try {
                updateFromXMLDocument();
            } catch (...) {
                IO::chDir(saveWorkingDirectory);
                throw; // re-issue the exception
            }
            OPENSIM_THROW_IF(getDocument()->getDocumentVersion() < 10901,
                OpenSim::Exception,
                "Model file " + s+ " is using unsupported file format"
                ". Please open model and save it in OpenSim version 3.3 to upgrade.");

            log_info("Loaded model {} from file {}", getName(), getInputFileName());

            try {
                finalizeFromProperties();
            }
            catch(const InvalidPropertyValue& err) {
                log_error("Model was unable to finalizeFromProperties."
                          "Update the model file and reload OR update the property and "
                          "call finalizeFromProperties() on the model."
                          "(details: {}).",
                        err.what());
                IO::chDir(saveWorkingDirectory);
            }
            IO::chDir(saveWorkingDirectory);
        }
    };

    void show_osim_file(std::string const& path) {
        Model model{path};
        model.finalizeFromProperties();
        model.finalizeConnections();

        // Configure the model.

        model.buildSystem();
        State& state = model.initSystem();
        model.initializeState();
        model.updMatterSubsystem().setShowDefaultGeometry(false);
        SimTK::Visualizer viz{model.getMultibodySystem()};
        viz.setShutdownWhenDestructed(true);
        viz.setCameraClippingPlanes(.01,100.);
        viz.setWindowTitle("lol");

        DynamicDecorationGenerator dg{&model};
        viz.addDecorationGenerator(&dg);

        Array_<DecorativeGeometry> tmp;
        dg.generateDecorations(state, tmp);
        GeomVisitor v{model, state};
        for (DecorativeGeometry& dg : tmp) {
            dg.implementGeometry(v);
        }

        viz.setBackgroundType(viz.SolidColor);
        viz.setBackgroundColor(White);
        viz.drawFrameNow(state);


        using std::chrono_literals::operator""s;
        std::this_thread::sleep_for(5s);
    }

    std::string to_string(Transform const& t) {
        std::stringstream ss;
        ss << "rotation = " << t.R()[0] << t.R()[1] << t.R()[2];
        ss << " translation = " << t.T()[0] << "x " << t.T()[1] << "y " << t.T()[2] << "z";
        return ss.str();
    }

    struct GeomVisitor : public DecorativeGeometryImplementation {
        Model& model;
        State& state;

        GeomVisitor(Model& _model, State& _state) : model{_model}, state{_state} {
        }

        Transform ground_to_decoration_xform(DecorativeGeometry const& geom) {
            SimbodyMatterSubsystem const& ms = model.getSystem().getMatterSubsystem();
            MobilizedBody const& mobod = ms.getMobilizedBody(MobilizedBodyIndex(geom.getBodyId()));
            Transform const& ground_to_body_xform = mobod.getBodyTransform(state);
            Transform const& body_to_decoration_xform = geom.getTransform();

            return ground_to_body_xform * body_to_decoration_xform;
        }

        void implementPointGeometry(const DecorativePoint& geom) override {
            std::cerr << "point: " << geom.getPoint() << std::endl;
        }
        void implementLineGeometry(const DecorativeLine& geom) override {
            std::cerr << "line:" << std::endl
                      << "    p1 = " << geom.getPoint1() << std::endl
                      << "    p2 = " << geom.getPoint2() << std::endl;
        }
        void implementBrickGeometry(const DecorativeBrick& geom) override {
            std::cerr << "brick" << std::endl;
        }
        void implementCylinderGeometry(const DecorativeCylinder& geom) override {
            std::cerr << "cylinder:" << std::endl
                      << "    radius = " << geom.getRadius() << std::endl
                      << "    xform = " << to_string(ground_to_decoration_xform(geom)) << std::endl;
        }
        void implementCircleGeometry(const DecorativeCircle& geom) override {
            std::cerr << "circle"  << std::endl;
        }
        void implementSphereGeometry(const DecorativeSphere& geom) override {
            Transform xform = ground_to_decoration_xform(geom);
            std::cerr << "sphere:"  << std::endl
                      << "    radius = " << geom.getRadius() << std::endl
                      << "    xform = " << to_string(xform) << std::endl;
        }
        void implementEllipsoidGeometry(const DecorativeEllipsoid& geom) override {
            std::cerr << "ellipsoid:" << std::endl
                      << "    radii = " << geom.getRadii() << std::endl;
        }
        void implementFrameGeometry(const DecorativeFrame& geom) override {
            std::cerr << "frame"  << std::endl;
        }
        void implementTextGeometry(const DecorativeText& geom) override {
            std::cerr << "text"  << std::endl;
        }
        void implementMeshGeometry(const DecorativeMesh& geom) override {
            std::cerr << "mesh" << std::endl;
        }
        void implementMeshFileGeometry(const DecorativeMeshFile& geom) override {
            std::cerr << "meshfile:" << std::endl
                      << "    filename = " << geom.getMeshFile() << std::endl;
        }
        void implementArrowGeometry(const DecorativeArrow& geom) override {
            std::cerr << "arrow" << std::endl;
        }
        void implementTorusGeometry(const DecorativeTorus& geom) override {
            std::cerr << "torus" << std::endl;
        }
        void implementConeGeometry(const DecorativeCone& geom) override {
            std::cerr << "cone" << std::endl;
        }
    };
    */

    void generateGeometry(Model& model, State const& state, Array_<DecorativeGeometry>& geometry) {
        model.generateDecorations(true, model.getDisplayHints(), state, geometry);
        ComponentList<const Component> allComps = model.getComponentList();
        ComponentList<Component>::const_iterator iter = allComps.begin();
        while (iter != allComps.end()){
            //std::string cn = iter->getConcreteClassName();
            //std::cout << cn << ":" << iter->getName() << std::endl;
            iter->generateDecorations(true, model.getDisplayHints(), state, geometry);
            iter++;
        }


        DefaultGeometry dg{model};
        dg.generateDecorations(state, geometry);
    }


    // A hacky decoration generator that just always generates all geometry,
    // even if it's static.
    struct DynamicDecorationGenerator : public DecorationGenerator {
        Model* _model;
        DynamicDecorationGenerator(Model* model) : _model{model} {
            assert(_model != nullptr);
        }
        void useModel(Model* newModel) {
            assert(newModel != nullptr);
            _model = newModel;
        }

        void generateDecorations(const State& state, Array_<DecorativeGeometry>& geometry) override {
            generateGeometry(*_model, state, geometry);
        }
    };

    struct Geometry_visitor final : public DecorativeGeometryImplementation {
        Model& model;
        State& state;
        std::vector<osim::Geometry>& out;

        Geometry_visitor(Model& _model,
                         State& _state,
                         std::vector<osim::Geometry>& _out) :
            model{_model},
            state{_state},
            out{_out} {
        }

        Transform ground_to_decoration_xform(DecorativeGeometry const& geom) {
            SimbodyMatterSubsystem const& ms = model.getSystem().getMatterSubsystem();
            MobilizedBody const& mobod = ms.getMobilizedBody(MobilizedBodyIndex(geom.getBodyId()));
            Transform const& ground_to_body_xform = mobod.getBodyTransform(state);
            Transform const& body_to_decoration_xform = geom.getTransform();

            return ground_to_body_xform * body_to_decoration_xform;
        }

        glm::mat4 transform(DecorativeGeometry const& geom) {
            Transform t = ground_to_decoration_xform(geom);
            glm::mat4 m = glm::identity<glm::mat4>();
            // x
            m[0][0] = t.R()[0][0];
            m[0][1] = t.R()[0][1];
            m[0][2] = t.R()[0][2];
            // y
            m[1][0] = t.R()[1][0];
            m[1][1] = t.R()[1][1];
            m[1][2] = t.R()[1][2];
            // z
            m[2][0] = t.R()[2][0];
            m[2][1] = t.R()[2][1];
            m[2][2] = t.R()[2][2];
            // w
            m[3][0] = t.p()[0];
            m[3][1] = t.p()[1];
            m[3][2] = t.p()[2];
            return m;
        }

        glm::vec3 scale_factors(DecorativeGeometry const& geom) {
            Vec3 sf = geom.getScaleFactors();
            for (int i = 0; i < 3; ++i) {
                sf[i] = sf[i] <= 0 ? 1.0 : sf[i];
            }
            return {sf[0], sf[1], sf[2]};
        }

        glm::vec4 rgba(DecorativeGeometry const& geom) {
            Vec3 const& rgb = geom.getColor();
            Real a = geom.getOpacity();
            return {rgb[0], rgb[1], rgb[2], a < 0.0f ? 1.0f : a};
        }

        glm::vec4 to_vec4(Vec3 const& v, float w = 1.0f) {
            return glm::vec4{v[0], v[1], v[2], w};
        }

        void implementPointGeometry(const DecorativePoint&) override {
        }
        void implementLineGeometry(const DecorativeLine& geom) override {
            glm::mat4 xform = transform(geom);
            glm::vec4 p1 = xform * to_vec4(geom.getPoint1());
            glm::vec4 p2 = xform * to_vec4(geom.getPoint2());
            out.push_back(osim::Line{
                .p1 = {p1.x, p1.y, p1.z},
                .p2 = {p2.x, p2.y, p2.z},
                .rgba = rgba(geom)
            });
        }
        void implementBrickGeometry(const DecorativeBrick&) override {
        }
        void implementCylinderGeometry(const DecorativeCylinder& geom) override {
            glm::mat4 m = transform(geom);
            glm::vec3 s = scale_factors(geom);
            s.x *= geom.getRadius();
            s.y *= geom.getHalfHeight();
            s.z *= geom.getRadius();

            out.push_back(osim::Cylinder{
                .transform = m,
                .scale = s,
                .rgba = rgba(geom),
            });
        }
        void implementCircleGeometry(const DecorativeCircle&) override {
        }
        void implementSphereGeometry(const DecorativeSphere& geom) override {
            out.push_back(osim::Sphere{
                .transform = transform(geom),
                .rgba = rgba(geom),
                .radius = static_cast<float>(geom.getRadius()),
            });
        }
        void implementEllipsoidGeometry(const DecorativeEllipsoid&) override {
        }
        void implementFrameGeometry(const DecorativeFrame&) override {
        }
        void implementTextGeometry(const DecorativeText& t) override {
        }
        void implementMeshGeometry(const DecorativeMesh&) override {
        }
        void implementMeshFileGeometry(const DecorativeMeshFile&) override {
        }
        void implementArrowGeometry(const DecorativeArrow&) override {
        }
        void implementTorusGeometry(const DecorativeTorus&) override {
        }
        void implementConeGeometry(const DecorativeCone&) override {
        }
    };
}

std::vector<osim::Geometry> osim::geometry_in(std::string_view path) {
    Model model{std::string{path}};
    model.finalizeFromProperties();
    model.finalizeConnections();

    // Configure the model.

    model.buildSystem();
    State& state = model.initSystem();
    model.initializeState();
    model.updMatterSubsystem().setShowDefaultGeometry(false);

    DynamicDecorationGenerator dg{&model};
    Array_<DecorativeGeometry> tmp;
    dg.generateDecorations(state, tmp);

    auto rv = std::vector<osim::Geometry>{};
    auto visitor = Geometry_visitor{model, state, rv};
    for (DecorativeGeometry& dg : tmp) {
        dg.implementGeometry(visitor);
    }

    return rv;
}
