#include <sudodem/pkg/common/Box.hpp>



void Bo1_Box_Aabb::go(	const shared_ptr<Shape>& cm,
				shared_ptr<Bound>& bv,
				const Se3r& se3,
				const Body*	b)
{
	Box* box = static_cast<Box*>(cm.get());
	if(!bv){ bv=shared_ptr<Bound>(new Aabb); }
	Aabb* aabb=static_cast<Aabb*>(bv.get());

	if(scene->isPeriodic && scene->cell->hasShear()) throw logic_error(__FILE__ "Boxes not (yet?) supported in sheared cell.");

	Matrix3r r=se3.orientation.toRotationMatrix();
	Vector3r halfSize(Vector3r::Zero());
	for( int i=0; i<3; ++i )
		for( int j=0; j<3; ++j )
			halfSize[i] += std::abs( r(i,j) * box->extents[j] );

	aabb->min = se3.position-halfSize;
	aabb->max = se3.position+halfSize;
}

#ifdef SUDODEM_OPENGL

#include <sudodem/lib/opengl/OpenGLWrapper.hpp>
#include <sudodem/core/Scene.hpp>

void Gl1_Box::go(const shared_ptr<Shape>& cg, const shared_ptr<State>&,bool wire,const GLViewInfo&)
{
	glColor3v(cg->color);
	Vector3r &extents = (static_cast<Box*>(cg.get()))->extents;
	glScalef(2*extents[0],2*extents[1],2*extents[2]);
	if (wire) glutWireCube(1);
	else glutSolidCube(1);
}

void Gl1_Box::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Gl1_Box");
	pybind11::class_<Gl1_Box, Functor, std::shared_ptr<Gl1_Box>> _classObj(_module, "Gl1_Box", "Renders :yref:`Box` object");
	_classObj.def(pybind11::init<>());
}

#endif

// pyRegisterClass implementations moved from Box.hpp
void Box::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Box");
	pybind11::class_<Box, Shape, std::shared_ptr<Box>> _classObj(_module, "Box", "Box (cuboid) particle geometry. (Avoid using in new code, prefer :yref:`Facet` instead.");
	_classObj.def(pybind11::init<>());
	_classObj.def(pybind11::init<Vector3r>());
	_classObj.def_readwrite("extents", &Box::extents, "Half-size of the cuboid");
}

void Bo1_Box_Aabb::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Bo1_Box_Aabb");
	pybind11::class_<Bo1_Box_Aabb, BoundFunctor, std::shared_ptr<Bo1_Box_Aabb>> _classObj(_module, "Bo1_Box_Aabb", "Create/update an :yref:`Aabb` of a :yref:`Box`.");
	_classObj.def(pybind11::init<>());
}

REGISTER_CLASS_INDEX_CPP(Box,Shape)
