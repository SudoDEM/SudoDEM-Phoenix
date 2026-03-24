#include <sudodem/pkg/common/Aabb.hpp>


#ifdef SUDODEM_OPENGL
#include <sudodem/lib/opengl/OpenGLWrapper.hpp>
#include<sudodem/lib/opengl/GLUtils.hpp>
#include <sudodem/core/Scene.hpp>

		void Gl1_Aabb::go(const shared_ptr<Bound>& bv, Scene* scene){
			Aabb* aabb = static_cast<Aabb*>(bv.get());
			glColor3v(bv->color);

			if(!scene->isPeriodic){
				glTranslatev(Vector3r(.5*(aabb->min+aabb->max)));
				glScalev(Vector3r(aabb->max-aabb->min));
				//glBegin(GL_LINE_STRIP);
		 	 	//glVertex3v(aabb->min); glVertex3(aabb->max[0],aabb->min[1],aabb->min[2]); glVertex3v(aabb->max);
		    //glVertex3(aabb->min[0],aabb->max[1],aabb->min[2]); glVertex3v(aabb->min);
		 	  //glEnd();
			} else {
				glTranslatev(Vector3r(scene->cell->shearPt(scene->cell->wrapPt(.5*(aabb->min+aabb->max)))));
				glMultMatrixd(scene->cell->getGlShearTrsfMatrix());
				glScalev(Vector3r(aabb->max-aabb->min));//FIXME:not working without reconstrution of Vector3r

				//glutWireCube(1);
				/*Vector3r min,max;
				Vector3r trans = Vector3r(scene->cell->shearPt(scene->cell->wrapPt(.5*(aabb->min+aabb->max))));
				min = aabb->min + trans; max = aabb->max +trans;
				glBegin(GL_LINE_STRIP);
		 	 	glVertex3v(min); glVertex3(max[0],min[1],min[2]); glVertex3v(max);
		     glVertex3(min[0],max[1],min[2]); glVertex3v(min);
		 	  glEnd();*/
			}
			GLUtils::Square(1);

		}

void Gl1_Aabb::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<Gl1_Aabb, Functor, std::shared_ptr<Gl1_Aabb>> _classObj(_module, "Gl1_Aabb", "Render Axis-aligned bounding box (:yref:`Aabb`).");
	_classObj.def(pybind11::init<>());
}

#endif

// pyRegisterClass implementation moved from Aabb.hpp
void Aabb::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<Aabb, Bound, std::shared_ptr<Aabb>> _classObj(_module, "Aabb", "Axis-aligned bounding box, for use with :yref:`InsertionSortCollider`. (This class is quasi-redundant since min,max are already contained in :yref:`Bound` itself. That might change at some point, though.)");
	_classObj.def(pybind11::init<>());
}

REGISTER_CLASS_INDEX_CPP(Aabb,Bound)
