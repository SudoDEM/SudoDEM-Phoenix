// © 2004 Olivier Galizzi <olivier.galizzi@imag.fr>
// © 2008 Václav Šmilauer <eudoxos@arcig.cz>
#pragma once

#include<sudodem/lib/multimethods/DynLibDispatcher.hpp>
#include<sudodem/core/Dispatcher.hpp>
#include<sudodem/core/Body.hpp>
#include<sudodem/lib/opengl/OpenGLWrapper.hpp>

#include<sudodem/pkg/common/GLDrawFunctors.hpp>

struct GlExtraDrawer: public Serializable{
	Scene* scene;
	bool dead;
	
	virtual void render();
	
	GlExtraDrawer() : scene(nullptr), dead(false) {}
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
		pybind11::class_<GlExtraDrawer, Serializable, std::shared_ptr<GlExtraDrawer>> _classObj(_module, "GlExtraDrawer", "Performing arbitrary OpenGL drawing commands; called from :yref:`OpenGLRenderer` (see :yref:`OpenGLRenderer.extraDrawers`) once regular rendering routines will have finished.\n\nThis class itself does not render anything, derived classes should override the *render* method.");
		_classObj.def(pybind11::init<>());
		_classObj.def_readwrite("dead", &GlExtraDrawer::dead, "Deactivate the object (on error/exception).");
	}
};
REGISTER_SERIALIZABLE_BASE(GlExtraDrawer, Serializable);

class OpenGLRenderer : public Serializable
{
	public:
		//static const int numClipPlanes=3;

		//bool pointClipped(const Vector3r& p);
		//vector<Vector3r> clipPlaneNormals;
		void setBodiesDispInfo();
		static bool initDone;
		Vector2r viewDirection; // updated from GLViewer regularly
		GLViewInfo viewInfo; // update from GLView regularly
		Vector3r highlightEmission0;
		Vector3r highlightEmission1;

		// normalized saw signal with given periodicity, with values ∈ 〈0,1〉 */
		Real normSaw(Real t, Real period){ Real xi=(t-period*((int)(t/period)))/period; /* normalized value, (0-1〉 */ return (xi<.5?2*xi:2-2*xi); }
		Real normSquare(Real t, Real period){ Real xi=(t-period*((int)(t/period)))/period; /* normalized value, (0-1〉 */ return (xi<.5?0:1); }

		void drawPeriodicCell();

		void setBodiesRefPos();

		struct BodyDisp{
			Vector2r pos;
			Rotationr ori;
			bool isDisplayed;
			bool hidden;
		};
		//! display data for individual bodies
		vector<BodyDisp> bodyDisp;
		void hide(Body::id_t id) {if ((unsigned int) id<bodyDisp.size()) bodyDisp[id].hidden=true; }
		void show(Body::id_t id) {if ((unsigned int) id<bodyDisp.size()) bodyDisp[id].hidden=false; }

		virtual ~OpenGLRenderer();

	private:
		void resetSpecularEmission();

		GlBoundDispatcher boundDispatcher;
		GlIGeomDispatcher geomDispatcher;
		GlIPhysDispatcher physDispatcher;
		GlShapeDispatcher shapeDispatcher;
		// GlStateDispatcher stateDispatcher;


		vector<string>
			// stateFunctorNames,
			boundFunctorNames,
			shapeFunctorNames,
			geomFunctorNames,
			physFunctorNames;

		DECLARE_LOGGER;

	public :
		// updated after every call to render
		shared_ptr<Scene> scene;
		
		// member variables
		Vector2r dispScale;
		Real rotScale;
		Vector3r lightPos;
		Vector3r light2Pos;
		Vector3r lightColor;
		Vector3r light2Color;
		Vector3r cellColor;
		Vector3r bgColor;
		bool wire;
		bool light1;
		bool light2;
		bool dof;
		bool id;
		bool bound;
		bool shape;
		bool intrWire;
		bool intrGeom;
		bool intrPhys;
		bool ghosts;
		int mask;
		Body::id_t selId;
		//vector<Se3r> clipPlaneSe3;
		//vector<bool> clipPlaneActive;
		vector<shared_ptr<GlExtraDrawer>> extraDrawers;
		bool intrAllWire;

		void init();
		void initgl();
		void render(const shared_ptr<Scene>& scene, Body::id_t selection=Body::id_t(-1));
		void pyRender(){render(Omega::instance().getScene());}

		void renderDOF_ID();
		void renderIPhys();
		void renderIGeom();
		void renderBound();
		// called also to render selectable entitites;
		void renderShape();
		void renderAllInteractionsWire();

		OpenGLRenderer() : dispScale(Vector2r::Ones()), rotScale(1), lightPos(Vector3r(75,130,0)), light2Pos(Vector3r(-130,75,30)), lightColor(Vector3r(0.6,0.6,0.6)), light2Color(Vector3r(0.5,0.5,0.1)), cellColor(Vector3r(1,1,0)), bgColor(Vector3r(.32,.34,.43)), wire(false), light1(true), light2(true), dof(false), id(false), bound(false), shape(true), intrWire(false), intrGeom(false), intrPhys(false), ghosts(true), mask(~0), selId(Body::ID_NONE), intrAllWire(false), scene(nullptr), viewDirection(Vector2r::Zero()), viewInfo(GLViewInfo()), highlightEmission0(Vector3r::Zero()), highlightEmission1(Vector3r::Zero()) {}

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<OpenGLRenderer, Serializable, std::shared_ptr<OpenGLRenderer>> _classObj(_module, "OpenGLRenderer", "Class responsible for rendering scene on OpenGL devices.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("dispScale", &OpenGLRenderer::dispScale, "Artificially enlarge (scale) dispalcements from bodies' :yref:`reference positions<State.refPos>` by this relative amount, so that they become better visible (independently in 3 dimensions). Disbled if (1,1,1).");
			_classObj.def_readwrite("rotScale", &OpenGLRenderer::rotScale, "Artificially enlarge (scale) rotations of bodies relative to their :yref:`reference orientation<State.refOri>`, so the they are better visible.");
			_classObj.def_readwrite("lightPos", &OpenGLRenderer::lightPos, "Position of OpenGL light source in the scene.");
			_classObj.def_readwrite("light2Pos", &OpenGLRenderer::light2Pos, "Position of secondary OpenGL light source in the scene.");
			_classObj.def_readwrite("lightColor", &OpenGLRenderer::lightColor, "Per-color intensity of primary light (RGB).");
			_classObj.def_readwrite("light2Color", &OpenGLRenderer::light2Color, "Per-color intensity of secondary light (RGB).");
			_classObj.def_readwrite("cellColor", &OpenGLRenderer::cellColor, "Color of the periodic cell (RGB).");
			_classObj.def_readwrite("bgColor", &OpenGLRenderer::bgColor, "Color of the background canvas (RGB)");
			_classObj.def_readwrite("wire", &OpenGLRenderer::wire, "Render all bodies with wire only (faster)");
			_classObj.def_readwrite("light1", &OpenGLRenderer::light1, "Turn light 1 on.");
			_classObj.def_readwrite("light2", &OpenGLRenderer::light2, "Turn light 2 on.");
			_classObj.def_readwrite("dof", &OpenGLRenderer::dof, "Show which degrees of freedom are blocked for each body");
			_classObj.def_readwrite("id", &OpenGLRenderer::id, "Show body id's");
			_classObj.def_readwrite("bound", &OpenGLRenderer::bound, "Render body :yref:`Bound`");
			_classObj.def_readwrite("shape", &OpenGLRenderer::shape, "Render body :yref:`Shape`");
			_classObj.def_readwrite("intrWire", &OpenGLRenderer::intrWire, "If rendering interactions, use only wires to represent them.");
			_classObj.def_readwrite("intrGeom", &OpenGLRenderer::intrGeom, "Render :yref:`Interaction::geom` objects.");
			_classObj.def_readwrite("intrPhys", &OpenGLRenderer::intrPhys, "Render :yref:`Interaction::phys` objects");
			_classObj.def_readwrite("ghosts", &OpenGLRenderer::ghosts, "Render objects crossing periodic cell edges by cloning them in multiple places (periodic simulations only).");
			_classObj.def_readwrite("mask", &OpenGLRenderer::mask, "Bitmask for showing only bodies where ((mask & :yref:`Body::mask`)!=0)");
			_classObj.def_readwrite("selId", &OpenGLRenderer::selId, "Id of particle that was selected by the user.");
			_classObj.def_readwrite("extraDrawers", &OpenGLRenderer::extraDrawers, "Additional rendering components (:yref:`GlExtraDrawer`).");
			_classObj.def_readwrite("intrAllWire", &OpenGLRenderer::intrAllWire, "Draw wire for all interactions, blue for potential and green for real ones (mostly for debugging)");
			_classObj.def("setRefPos", &OpenGLRenderer::setBodiesRefPos, "Make current positions and orientation reference for scaleDisplacements and scaleRotations.");
			_classObj.def("render", &OpenGLRenderer::pyRender, "Render the scene in the current OpenGL context.");
			_classObj.def("hideBody", &OpenGLRenderer::hide, pybind11::arg("id"), "Hide body from id (see :yref:`OpenGLRenderer::showBody`)");
			_classObj.def("showBody", &OpenGLRenderer::show, pybind11::arg("id"), "Make body visible (see :yref:`OpenGLRenderer::hideBody`)");
		}
};
REGISTER_SERIALIZABLE(OpenGLRenderer);