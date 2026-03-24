// © 2009 Vaclav Smilauer <eudoxos@arcig.cz>
#include<sudodem/pkg/common/Wall.hpp>
#include<sudodem/pkg/common/Aabb.hpp>
#include<pybind11/pybind11.h>
#include<pybind11/eigen.h>


Wall::~Wall(){} // vtable
Fwall::~Fwall(){}


void Bo1_Wall_Aabb::go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r& se3, const Body* b){
	Wall* wall=static_cast<Wall*>(cm.get());
	if(!bv){ bv=shared_ptr<Bound>(new Aabb); }
	Aabb* aabb=static_cast<Aabb*>(bv.get());
	if(scene->isPeriodic && scene->cell->hasShear()) throw logic_error(__FILE__ "Walls not supported in sheared cell.");
	const Real& inf=std::numeric_limits<Real>::infinity();
	aabb->min=Vector3r(-inf,-inf,-inf); aabb->min[wall->axis]=se3.position[wall->axis];
	aabb->max=Vector3r( inf, inf, inf); aabb->max[wall->axis]=se3.position[wall->axis];
}


CREATE_LOGGER(Fwall);



void Fwall::postLoad(Fwall&)
{
	// if this fails, it means someone did vertices push_back, but they are resized to 3 at Fwall initialization already
	// in the future, a fixed-size array should be used instead of vector<Vector3r> for vertices
	// this is prevented by sudodem::serialization now IIRC
	//if(vertices.size()!=2){ throw runtime_error(("Fwall must have exactly 2 vertices (not "+std::to_string(vertices.size())+")").c_str()); }
	if(isnan(vertex1[0]) || isnan(vertex2[0])) return;  // not initialized, nothing to do
	vu = vertex2-vertex1;
	vl = vu.norm();
	normal = Vector3r(-vu[1],vu[0],0);
	normal.normalize();
}


void Bo1_Fwall_Aabb::go(	  const shared_ptr<Shape>& cm
				, shared_ptr<Bound>& bv
				, const Se3r& se3
				, const Body* b)
{
	if(!bv){ bv=shared_ptr<Bound>(new Aabb); }
	Aabb* aabb=static_cast<Aabb*>(bv.get());
	Fwall* fwall = static_cast<Fwall*>(cm.get());
	const Vector3r& O = se3.position;
	Matrix3r rot =se3.orientation.toRotationMatrix();
	Vector3r vu_new = rot*fwall->vu;
	Vector3r halfsize = 0.5*Vector3r(abs(vu_new[0]),abs(vu_new[1]),abs(vu_new[2]));
	if(scene->isPeriodic){
			vu_new =scene->cell->unshearPt(O);
			aabb->min = vu_new - halfsize;
			aabb->max = vu_new + halfsize;
	}else{
		aabb->min = O - halfsize;
		aabb->max = O + halfsize;
	}

}

REGISTER_CLASS_INDEX_CPP(Wall,Shape)

REGISTER_CLASS_INDEX_CPP(Fwall,Shape)


#ifdef SUDODEM_OPENGL
	#include<sudodem/lib/opengl/OpenGLWrapper.hpp>

	void Gl1_Wall::go(const shared_ptr<Shape>& cm, const shared_ptr<State>& pp, bool, const GLViewInfo& glinfo){
		
		static int callCount = 0;
		Wall* wall=static_cast<Wall*>(cm.get());
		int ax0=wall->axis,ax1=(wall->axis+1)%3; if(ax1==ax0) ax1=(ax1+1)%3;
		int ax2=(ax1+1)%3; if(ax2==ax0) ax2=(ax2+1)%3;
		
		// Calculate line coordinates
		Real mn1=glinfo.sceneCenter[ax1]-glinfo.sceneRadius-pp->se3.position[ax1];
		Real step=2*glinfo.sceneRadius/div;
		
		Vector3r a1,b1;
		a1[ax0]=b1[ax0]=0;
		a1[ax1]=mn1-step;
		b1[ax1]=mn1+step*(div+2);
		a1[ax2]=b1[ax2]=pp->se3.position[ax2];
		
		// if (callCount++ < 10) {
		// 	std::cerr << "DEBUG Gl1_Wall::go: axis=" << wall->axis 
		// 	          << " pos=" << pp->se3.position.transpose()
		// 	          << " sceneCenter=" << glinfo.sceneCenter.transpose()
		// 	          << " sceneRadius=" << glinfo.sceneRadius
		// 	          << " mn1=" << mn1 << " step=" << step
		// 	          << " a1=" << a1.transpose() << " b1=" << b1.transpose() << std::endl;
		// }
		
		// Ensure color values are valid (between 0 and 1)
		Vector3r validColor = cm->color;
		for (int i = 0; i < 3; i++) {
			if (validColor[i] < 0) validColor[i] = 0;
			if (validColor[i] > 1) validColor[i] = 1;
		}
		glColor3v(validColor);
		
		// Draw a grid to represent the infinite wall plane
		Real start1 = mn1 - step;
		Real end1 = mn1 + step * (div + 2);
		Real start2 = glinfo.sceneCenter[ax2] - glinfo.sceneRadius - pp->se3.position[ax2];
		Real end2 = glinfo.sceneCenter[ax2] + glinfo.sceneRadius + pp->se3.position[ax2];
		Real step2 = (end2 - start2) / div;
		
		glBegin(GL_LINES);
			// Draw lines along axis 1
			for (int i = 0; i <= div + 2; i++) {
				Vector3r p1, p2;
				p1[ax0] = p2[ax0] = 0;
				p1[ax1] = p2[ax1] = start1 + i * step;
				p1[ax2] = start2;
				p2[ax2] = end2;
				glVertex3v(p1); glVertex3v(p2);
			}
			// Draw lines along axis 2
			for (int i = 0; i <= div; i++) {
				Vector3r p1, p2;
				p1[ax0] = p2[ax0] = 0;
				p1[ax1] = start1;
				p2[ax1] = end1;
				p1[ax2] = p2[ax2] = start2 + i * step2;
				glVertex3v(p1); glVertex3v(p2);
			}
		glEnd();
	}

	void Gl1_Fwall::go(const shared_ptr<Shape>& cm, const shared_ptr<State>& ,bool wire,const GLViewInfo&)
	{

		Fwall* fwall = static_cast<Fwall*>(cm.get());
		//const vector<Vector3r>& vertices = fwall->vertices;
		//if(cm->wire || wire){
			// Fwall
			glBegin(GL_LINES);
				glColor3v(cm->color);
				 glVertex3v(fwall->vertex1);
				 glVertex3v(fwall->vertex2);
				glEnd();
		//}
	}
#endif

// pyRegisterClass implementations moved from Wall.hpp
void Wall::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<Wall, Shape, std::shared_ptr<Wall>> _classObj(_module, "Wall", "Object representing infinite plane aligned with the coordinate system (axis-aligned wall).");
	_classObj.def(pybind11::init<>());
	// Constructor with keyword arguments for utils.wall()
	_classObj.def(pybind11::init([](int sense, int axis, pybind11::object colorObj) {
		auto w = std::make_shared<Wall>();
		w->sense = sense;
		w->axis = axis;
		if (!colorObj.is_none()) {
			// Convert color to Vector3r - handle both Vector3r and tuple/list
			try {
				Vector3r color = pybind11::cast<Vector3r>(colorObj);
				w->color = color;
			} catch (const pybind11::cast_error&) {
				// Try to cast as tuple/list of 3 floats
				try {
					pybind11::tuple colorTuple = pybind11::cast<pybind11::tuple>(colorObj);
					if (colorTuple.size() >= 3) {
						w->color = Vector3r(
							pybind11::cast<double>(colorTuple[0]),
							pybind11::cast<double>(colorTuple[1]),
							pybind11::cast<double>(colorTuple[2])
						);
					}
				} catch (...) {
					// If all conversions fail, leave default color
				}
			} catch (...) {
				// Other exceptions, leave default color
			}
		}
		return w;
	}), pybind11::arg("sense") = 0, pybind11::arg("axis") = 0, pybind11::arg("color") = pybind11::none());
	_classObj.def_readwrite("sense", &Wall::sense, "Which side of the wall interacts: -1 for negative only, 0 for both, +1 for positive only");
	_classObj.def_readwrite("axis", &Wall::axis, "Axis of the normal; can be 0,1 for +x, +y respectively (Body's orientation is disregarded for walls)");
}

void Bo1_Wall_Aabb::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<Bo1_Wall_Aabb, BoundFunctor, std::shared_ptr<Bo1_Wall_Aabb>> _classObj(_module, "Bo1_Wall_Aabb", "Creates/updates an :yref:`Aabb` of a :yref:`Wall`");
	_classObj.def(pybind11::init<>());
}

void Fwall::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<Fwall, Shape, std::shared_ptr<Fwall>> _classObj(_module, "Fwall", "Fwall (triangular particle) geometry.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("vertex1", &Fwall::vertex1, "Vertex positions in local coordinates.");
	_classObj.def_readwrite("vertex2", &Fwall::vertex2, "Vertex positions in local coordinates.");
	_classObj.def_readwrite("normal", &Fwall::normal, "Fwall's normal (in local coordinate system)");
}

void Bo1_Fwall_Aabb::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<Bo1_Fwall_Aabb, BoundFunctor, std::shared_ptr<Bo1_Fwall_Aabb>> _classObj(_module, "Bo1_Fwall_Aabb", "Creates/updates an :yref:`Aabb` of a :yref:`Fwall`.");
	_classObj.def(pybind11::init<>());
}

#ifdef SUDODEM_OPENGL
void Gl1_Wall::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<Gl1_Wall, Functor, std::shared_ptr<Gl1_Wall>> _classObj(_module, "Gl1_Wall", "Renders :yref:`Wall` object");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("div", &Gl1_Wall::div, "Number of divisions of the wall inside visible scene part.");
}

void Gl1_Fwall::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<Gl1_Fwall, Functor, std::shared_ptr<Gl1_Fwall>> _classObj(_module, "Gl1_Fwall", "Renders :yref:`Fwall` object");
	_classObj.def(pybind11::init<>());
}
#endif
