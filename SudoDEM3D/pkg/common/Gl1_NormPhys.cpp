#ifdef SUDODEM_OPENGL

#include<sudodem/core/Scene.hpp>
#include<sudodem/pkg/common/Gl1_NormPhys.hpp>
#include<sudodem/pkg/common/OpenGLRenderer.hpp>
#include<sudodem/pkg/common/NormShearPhys.hpp>
#include<sudodem/pkg/dem/DemXDofGeom.hpp>
#include<sudodem/pkg/dem/Shop.hpp>


	SUDODEM_PLUGIN(Gl1_NormPhys);

	void Gl1_NormPhys::pyRegisterClass(pybind11::module_ _module) {
		checkPyClassRegistersItself("Gl1_NormPhys");
		pybind11::class_<Gl1_NormPhys, Functor, std::shared_ptr<Gl1_NormPhys>> _classObj(_module, "Gl1_NormPhys", "Renders :yref:`NormPhys` objects as cylinders of which diameter and color depends on :yref:`NormPhys.normalForce` magnitude.");
		_classObj.def(pybind11::init<>());
		_classObj.def_readwrite_static("maxFn", &Gl1_NormPhys::maxFn, "Value of :yref:`NormPhys.normalForce` corresponding to :yref:`maxRadius<Gl1_NormPhys.maxRadius>`. This value will be increased (but *not decreased* ) automatically.");
		_classObj.def_readwrite_static("signFilter", &Gl1_NormPhys::signFilter, "If non-zero, only display contacts with negative (-1) or positive (+1) normal forces; if zero, all contacts will be displayed.");
		_classObj.def_readwrite_static("refRadius", &Gl1_NormPhys::refRadius, "Reference (minimum) particle radius; used only if :yref:`maxRadius<Gl1_NormPhys.maxRadius>` is negative. This value will be decreased (but *not increased* ) automatically.");
		_classObj.def_readwrite_static("maxRadius", &Gl1_NormPhys::maxRadius, "Cylinder radius corresponding to the maximum normal force. If negative, auto-updated :yref:`refRadius<Gl1_NormPhys.refRadius>` will be used instead.");
		_classObj.def_readwrite_static("slices", &Gl1_NormPhys::slices, "Number of sphere slices;");
		_classObj.def_readwrite_static("stacks", &Gl1_NormPhys::stacks, "Number of sphere stacks;");
		_classObj.def_readwrite_static("maxWeakFn", &Gl1_NormPhys::maxWeakFn, "Value that divides contacts by their normal force into the 'weak fabric' and 'strong fabric'. This value is set as side-effect by :yref:`utils.fabricTensor`.");
		_classObj.def_readwrite_static("weakFilter", &Gl1_NormPhys::weakFilter, "If non-zero, only display contacts belonging to the 'weak' (-1) or 'strong' (+1) fabric.");
		_classObj.def_readwrite_static("weakScale", &Gl1_NormPhys::weakScale, "If :yref:`maxWeakFn<Gl1_NormPhys.maxWeakFn>` is set, scale radius of the weak fabric by this amount (usually smaller than 1). If zero, 1 pixel line is displayed. Colors are not affected by this value.");
	}

	GLUquadric* Gl1_NormPhys::gluQuadric=NULL;
	Real Gl1_NormPhys::maxFn;
	Real Gl1_NormPhys::refRadius;
	Real Gl1_NormPhys::maxRadius;
	int Gl1_NormPhys::signFilter;
	int Gl1_NormPhys::slices;
	int Gl1_NormPhys::stacks;

	Real Gl1_NormPhys::maxWeakFn;
	int Gl1_NormPhys::weakFilter;
	Real Gl1_NormPhys::weakScale;

	void Gl1_NormPhys::go(const shared_ptr<IPhys>& ip, const shared_ptr<Interaction>& i, const shared_ptr<Body>& b1, const shared_ptr<Body>& b2, bool wireFrame){
		if(!gluQuadric){ gluQuadric=gluNewQuadric(); if(!gluQuadric) throw runtime_error("Gl1_NormPhys::go unable to allocate new GLUquadric object (out of memory?)."); }
		NormPhys* np=static_cast<NormPhys*>(ip.get());
		shared_ptr<IGeom> ig(i->geom); if(!ig) return; // changed meanwhile?
		GenericSpheresContact* geom=SUDODEM_CAST<GenericSpheresContact*>(ig.get());
		//if(!geom) cerr<<"Gl1_NormPhys: IGeom is not a GenericSpheresContact, but a "<<ig->getClassName()<<endl;
		Real fnNorm=np->normalForce.dot(geom->normal);
		if((signFilter>0 && fnNorm<0) || (signFilter<0 && fnNorm>0)) return;
		int fnSign=fnNorm>0?1:-1;
		fnNorm=std::abs(fnNorm);
		Real radiusScale=1.;
		// weak/strong fabric, only used if maxWeakFn is set
		if(!isnan(maxWeakFn)){
			if(fnNorm*fnSign<maxWeakFn){ // weak fabric
				if(weakFilter>0) return;
				radiusScale=weakScale;
			} else { // strong fabric
				if(weakFilter<0) return;
			}
		}

		maxFn=max(fnNorm,maxFn);
		Real realMaxRadius;
		if(maxRadius<0){
			if(geom->refR1>0) refRadius=min(geom->refR1,refRadius);
			if(geom->refR2>0) refRadius=min(geom->refR2,refRadius);
			realMaxRadius=refRadius;
		}
		else realMaxRadius=maxRadius;
		Real radius=radiusScale*realMaxRadius*(fnNorm/maxFn); // use logarithmic scale here?
		// Simple color mapping: blue (-maxFn) -> green (0) -> red (maxFn)
		Real t = (fnNorm*fnSign + maxFn) / (2 * maxFn);
		t = std::max(0.0, std::min(1.0, t));
		Vector3r color;
		if (t < 0.5) {
			color = Vector3r(0.0, 2.0 * t, 1.0 - 2.0 * t);
		} else {
			color = Vector3r(2.0 * (t - 0.5), 1.0 - 2.0 * (t - 0.5), 0.0);
		}
		# if 0
			// get endpoints from body positions
			Vector3r p1=b1->state->pos, p2=b2->state->pos;
			Vector3r relPos;
			if(scene->isPeriodic){
				relPos=p2+scene->cell->Hsize*i->cellDist.cast<Real>()-p1;
				p1=scene->cell->wrapShearedPt(p1);
				p2=p1+relPos;
			} else {
				relPos=p2-p1;
			}
			Real dist=relPos.norm();
		#else
			// get endpoints from geom
			// max(r,0) handles r<0 which is the case for "radius" of the facet
			Vector3r cp=scene->isPeriodic? scene->cell->wrapShearedPt(geom->contactPoint) : geom->contactPoint;
			Vector3r p1=cp-max(geom->refR1,(Real)0.)*geom->normal;
			Vector3r p2=cp+max(geom->refR2,(Real)0.)*geom->normal;
			const Vector3r& dispScale=scene->renderer ? scene->renderer->dispScale : Vector3r::Ones();
			if(dispScale!=Vector3r::Ones()){
				// move p1 and p2 by the same amounts as particles themselves would be moved
				p1+=dispScale.cwiseProduct(Vector3r(b1->state->pos-b1->state->refPos));
				p2+=dispScale.cwiseProduct(Vector3r(b2->state->pos-b2->state->refPos));
			}
			Vector3r relPos=p2-p1;
			Real dist=relPos.norm(); //max(geom->refR1,0.)+max(geom->refR2,0.);
	#endif


	glDisable(GL_CULL_FACE);
	glPushMatrix();
		glTranslatef(p1[0],p1[1],p1[2]);
		Quaternionr q(Quaternionr().setFromTwoVectors(Vector3r(0,0,1),relPos/dist /* normalized */));
		// using Transform with OpenGL: http://eigen.tuxfamily.org/dox/TutorialGeometry.html
		//glMultMatrixd(Eigen::Affine3d(q).data());
		glMultMatrix(Eigen::Transform<Real,3,Eigen::Affine>(q).data());
		glColor3v(color);
		gluCylinder(gluQuadric,radius,radius,dist,slices,stacks);
	glPopMatrix();
}

#endif /* SUDODEM_OPENGL */
