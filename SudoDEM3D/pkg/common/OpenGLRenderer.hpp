// © 2004 Olivier Galizzi <olivier.galizzi@imag.fr>
// © 2008 Vaclav Smilauer <eudoxos@arcig.cz>
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
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
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
		Vector3r viewDirection; // updated from GLViewer regularly
		GLViewInfo viewInfo; // update from GLView regularly
		Vector3r highlightEmission0;
		Vector3r highlightEmission1;

		// normalized saw signal with given periodicity, with values ∈ 〈0,1〉 */
		Real normSaw(Real t, Real period){ Real xi=(t-period*((int)(t/period)))/period; /* normalized value, (0-1〉 */ return (xi<.5?2*xi:2-2*xi); }
		Real normSquare(Real t, Real period){ Real xi=(t-period*((int)(t/period)))/period; /* normalized value, (0-1〉 */ return (xi<.5?0:1); }

		void drawPeriodicCell();

		void setBodiesRefPos();

		struct BodyDisp{
			Vector3r pos;
			Quaternionr ori;
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
		Vector3r dispScale;
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

		OpenGLRenderer() : dispScale(Vector3r::Ones()), rotScale(1), lightPos(Vector3r(75,130,0)), light2Pos(Vector3r(-130,75,30)), lightColor(Vector3r(0.6,0.6,0.6)), light2Color(Vector3r(0.5,0.5,0.1)), cellColor(Vector3r(1,1,0)), bgColor(Vector3r(.32,.34,.43)), wire(false), light1(true), light2(true), dof(false), id(false), bound(false), shape(true), intrWire(false), intrGeom(false), intrPhys(false), ghosts(true), mask(~0), selId(Body::ID_NONE), intrAllWire(false), scene(nullptr), viewDirection(Vector3r::Zero()), viewInfo(GLViewInfo()), highlightEmission0(Vector3r::Zero()), highlightEmission1(Vector3r::Zero()) {}

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE(OpenGLRenderer);