#include <sudodem/pkg/common/Sphere.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>

namespace py = pybind11;


void Bo1_Sphere_Aabb::go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r& se3, const Body* b){
	Sphere* sphere = static_cast<Sphere*>(cm.get());
	if(!bv){ bv=shared_ptr<Bound>(new Aabb); }
	Aabb* aabb=static_cast<Aabb*>(bv.get());
	Vector3r halfSize = (aabbEnlargeFactor>0?aabbEnlargeFactor:1.)*Vector3r(sphere->radius,sphere->radius,sphere->radius);
	if(!scene->isPeriodic){
		aabb->min=se3.position-halfSize; aabb->max=se3.position+halfSize;
		return;
	}
	// adjust box size along axes so that sphere doesn't stick out of the box even if sheared (i.e. parallelepiped)
	// Note: This code assumes getCos() returns a vector, but in 3D it returns a scalar.
	// Commented out as it appears to be 2D-specific or incorrect for 3D.
	// if(scene->cell->hasShear()) {
	// 	Vector3r refHalfSize(halfSize);
	// 	const Vector3r& cos=scene->cell->getCos();
	// 	for(int i=0; i<3; i++){
	// 		//cerr<<"cos["<<i<<"]"<<cos[i]<<" ";
	// 		int i1=(i+1)%3,i2=(i+2)%3;
	// 		halfSize[i1]+=.5*refHalfSize[i1]*(1/cos[i]-1);
	// 		halfSize[i2]+=.5*refHalfSize[i2]*(1/cos[i]-1);
	// 	}
	// }
	//cerr<<" || "<<halfSize<<endl;
	aabb->min = scene->cell->unshearPt(se3.position)-halfSize;
	aabb->max = scene->cell->unshearPt(se3.position)+halfSize;
}

REGISTER_CLASS_INDEX_CPP(Sphere,Shape)

#ifdef SUDODEM_OPENGL
#include <sudodem/lib/opengl/OpenGLWrapper.hpp>
#include <sudodem/core/Scene.hpp>

bool Gl1_Sphere::wire=false;
bool Gl1_Sphere::stripes=false;
int  Gl1_Sphere::Slices=16;
int  Gl1_Sphere::glutSlices=16;
int  Gl1_Sphere::glutStacks=16;
Real  Gl1_Sphere::quality=1.0;
bool  Gl1_Sphere::localSpecView=true;
vector<Vector3r> Gl1_Sphere::vertices, Gl1_Sphere::faces;
int Gl1_Sphere::glStripedSphereList=-1;
int Gl1_Sphere::glGlutSphereList=-1;
Real  Gl1_Sphere::prevQuality=0;

void Gl1_Sphere::go(const shared_ptr<Shape>& cm, const shared_ptr<State>& ,bool wire2, const GLViewInfo&)
{
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);

	Real r=(static_cast<Sphere*>(cm.get()))->radius;


	glColor3v(cm->color);
	if (wire || wire2) glutWireSphere(r,quality*glutSlices,quality*glutStacks);
	else {
		//Check if quality has been modified or if previous lists are invalidated (e.g. by creating a new qt view), then regenerate lists
		bool somethingChanged = (std::abs(quality-prevQuality)>0.001 || glIsList(glStripedSphereList)!=GL_TRUE);
		if (somethingChanged) {initStripedGlList(); initGlutGlList(); prevQuality=quality;}
		glScalef(r,r,r);
		if(stripes) glCallList(glStripedSphereList);
		else glCallList(glGlutSphereList);
	}
	return;
}


void Gl1_Sphere::subdivideTriangle(const Vector3r& v1,const Vector3r& v2,const Vector3r& v3, int depth){
	Vector3r v;
	//Change color only at the appropriate level, i.e. 8 times in total, since we draw 8 mono-color sectors one after another
	if (depth==int(quality) || quality<=0){
		v = (v1+v2+v3)/3.0;
		GLfloat matEmit[4];
		if (v[1]*v[0]*v[2]>0){
			matEmit[0] = 0.3;
			matEmit[1] = 0.3;
			matEmit[2] = 0.3;
			matEmit[3] = 1.f;
		}else{
			matEmit[0] = 0.15;
			matEmit[1] = 0.15;
			matEmit[2] = 0.15;
			matEmit[3] = 0.2;
		}
 		glMaterialfv(GL_FRONT, GL_EMISSION, matEmit);
	}
	if (depth==1){//Then display 4 triangles
		Vector3r v12 = v1+v2;
		Vector3r v23 = v2+v3;
		Vector3r v31 = v3+v1;
		v12.normalize();
		v23.normalize();
		v31.normalize();
		//Use TRIANGLE_STRIP for faster display of adjacent facets
		glBegin(GL_TRIANGLE_STRIP);
			glNormal3v(v1); glVertex3v(v1);
			glNormal3v(v31); glVertex3v(v31);
			glNormal3v(v12); glVertex3v(v12);
			glNormal3v(v23); glVertex3v(v23);
			glNormal3v(v2); glVertex3v(v2);
		glEnd();
		//terminate with this triangle left behind
		glBegin(GL_TRIANGLES);
			glNormal3v(v3); glVertex3v(v3);
			glNormal3v(v23); glVertex3v(v23);
			glNormal3v(v31); glVertex3v(v31);
		glEnd();
		return;
	}
	Vector3r v12 = v1+v2;
	Vector3r v23 = v2+v3;
	Vector3r v31 = v3+v1;
	v12.normalize();
	v23.normalize();
	v31.normalize();
	subdivideTriangle(v1,v12,v31,depth-1);
	subdivideTriangle(v2,v23,v12,depth-1);
	subdivideTriangle(v3,v31,v23,depth-1);
	subdivideTriangle(v12,v23,v31,depth-1);
}

void Gl1_Sphere::initStripedGlList() {
	if (!vertices.size()){//Fill vectors with vertices and facets
		//Define 6 points for +/- coordinates
		vertices.push_back(Vector3r(-1,0,0));//0
		vertices.push_back(Vector3r(1,0,0));//1
		vertices.push_back(Vector3r(0,-1,0));//2
		vertices.push_back(Vector3r(0,1,0));//3
		vertices.push_back(Vector3r(0,0,-1));//4
		vertices.push_back(Vector3r(0,0,1));//5
		//Define 8 sectors of the sphere
		faces.push_back(Vector3r(3,4,1));
		faces.push_back(Vector3r(3,0,4));
		faces.push_back(Vector3r(3,5,0));
		faces.push_back(Vector3r(3,1,5));
		faces.push_back(Vector3r(2,1,4));
		faces.push_back(Vector3r(2,4,0));
		faces.push_back(Vector3r(2,0,5));
		faces.push_back(Vector3r(2,5,1));
	}
	//Generate the list. Only once for each qtView, or more if quality is modified.
	glDeleteLists(glStripedSphereList,1);
	glStripedSphereList = glGenLists(1);
	glNewList(glStripedSphereList,GL_COMPILE);
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	// render the sphere now
	for (int i=0;i<8;i++)
		subdivideTriangle(vertices[(unsigned int)faces[i][0]],vertices[(unsigned int)faces[i][1]],vertices[(unsigned int)faces[i][2]],1+ (int) quality);
	glEndList();

}

void Gl1_Sphere::initgl(){
	// Initialize display lists for sphere rendering
	// This is called once when the functor is first created
	if(glIsList(glGlutSphereList)!=GL_TRUE) {
		initGlutGlList();
	}
	if(glIsList(glStripedSphereList)!=GL_TRUE) {
		initStripedGlList();
	}
}

void Gl1_Sphere::initGlutGlList(){
	//Generate the "no-stripes" display list, each time quality is modified
	glDeleteLists(glGlutSphereList,1);
	glGlutSphereList = glGenLists(1);
	glNewList(glGlutSphereList,GL_COMPILE);
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
		glutSolidSphere(1.0,max(quality*glutSlices,(Real)2.),max(quality*glutStacks,(Real)3.));
	glEndList();
}
#endif

// pyRegisterClass implementation moved from Sphere.hpp
void Sphere::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Sphere");
	pybind11::class_<Sphere, Shape, std::shared_ptr<Sphere>> _classObj(_module, "Sphere", "Geometry of spherical particle.");
	_classObj.def(pybind11::init<Real>());
	// Constructor with radius, color, wire, highlight
	_classObj.def(pybind11::init([](Real r, pybind11::object color_py, bool w, bool h) {
		auto s = std::make_shared<Sphere>(r);
		// Handle None color - use default
		if (!color_py.is_none()) {
			s->color = color_py.cast<Vector3r>();
		}
		s->wire = w;
		s->highlight = h;
		return s;
	}), py::arg("radius") = 0.5, py::arg("color") = pybind11::none(), 
	   py::arg("wire") = false, py::arg("highlight") = false);
	_classObj.def_readwrite("radius", &Sphere::radius, "Radius [m]");
	_classObj.def_readwrite("ref_radius", &Sphere::ref_radius, "reference radius [m]");
}

void Bo1_Sphere_Aabb::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Bo1_Sphere_Aabb");
	pybind11::class_<Bo1_Sphere_Aabb, BoundFunctor, std::shared_ptr<Bo1_Sphere_Aabb>> _classObj(_module, "Bo1_Sphere_Aabb", "Functor creating :yref:`Aabb` from :yref:`Sphere`.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("aabbEnlargeFactor", &Bo1_Sphere_Aabb::aabbEnlargeFactor, "Relative enlargement of the bounding box; deactivated if negative.\n\n.. note::\n\tThis attribute is used to create distant interaction, but is only meaningful with an :yref:`IGeomFunctor` which will not simply discard such interactions: :yref:`Ig2_Sphere_Sphere_ScGeom::interactionDetectionFactor` should have the same value as :yref:`aabbEnlargeFactor<Bo1_Sphere_Aabb::aabbEnlargeFactor>`.");
}

#ifdef SUDODEM_OPENGL
void Gl1_Sphere::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Gl1_Sphere");
	pybind11::class_<Gl1_Sphere, Functor, std::shared_ptr<Gl1_Sphere>> _classObj(_module, "Gl1_Sphere", "Renders :yref:`Sphere` object");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite_static("wire", &Gl1_Sphere::wire, "Only show wireframe (controlled by ``glutSlices`` and ``glutStacks``.");
	_classObj.def_readonly_static("Slices", &Gl1_Sphere::Slices, "Base number of sphere slices, multiplied by :yref:`Gl1_Sphere::quality` before use); not used with ``stripes`` (see `glut{Solid,Wire}Sphere reference <http://www.opengl.org/documentation/specs/glut/spec3/node81.html>`_)");
}
#endif
