// © 2004 Olivier Galizzi <olivier.galizzi@imag.fr>
// © 2004 Janek Kozicki <cosurgi@berlios.de>
// © 2008 Vaclav Smilauer <eudoxos@arcig.cz>
// © 2006 Bruno Chareyre <bruno.chareyre@hmg.inpg.fr>

#include<sudodem/pkg/dem/ScGeom.hpp>
#include<sudodem/core/Omega.hpp>
#include<sudodem/core/Scene.hpp>

ScGeom::~ScGeom(){};
ScGeom6D::~ScGeom6D(){};
ChCylGeom6D::~ChCylGeom6D(){};

REGISTER_CLASS_INDEX_CPP(ScGeom,GenericSpheresContact)
REGISTER_CLASS_INDEX_CPP(ScGeom6D,ScGeom)
REGISTER_CLASS_INDEX_CPP(ChCylGeom6D,ScGeom6D)

Vector3r& ScGeom::rotate(Vector3r& shearForce) const {
	// approximated rotations
	shearForce -= shearForce.cross(orthonormal_axis);
	shearForce -= shearForce.cross(twist_axis);
	//NOTE : make sure it is in the tangent plane? It's never been done before. Is it not adding rounding errors at the same time in fact?...
	//shearForce -= normal.dot(shearForce)*normal;
	return shearForce;
}

//!Precompute data needed for rotating tangent vectors attached to the interaction
void ScGeom::precompute(const State& rbp1, const State& rbp2, const Scene* scene, const shared_ptr<Interaction>& c, const Vector3r& currentNormal, bool isNew, const Vector3r& shift2, bool avoidGranularRatcheting){
	if(!isNew) {
		orthonormal_axis = normal.cross(currentNormal);
		Real angle = scene->dt*0.5*normal.dot(rbp1.angVel + rbp2.angVel);
		twist_axis = angle*normal;}
	else twist_axis=orthonormal_axis=Vector3r::Zero();
	//Update contact normal
	normal=currentNormal;
	//Precompute shear increment
	Vector3r relativeVelocity=getIncidentVel(&rbp1,&rbp2,scene->dt,shift2,scene->isPeriodic ? scene->cell->intrShiftVel(c->cellDist) : Vector3r::Zero(),avoidGranularRatcheting);
	//keep the shear part only
	relativeVelocity = relativeVelocity-normal.dot(relativeVelocity)*normal;
	shearInc = relativeVelocity*scene->dt;
}

Vector3r ScGeom::getIncidentVel(const State* rbp1, const State* rbp2, Real dt, const Vector3r& shift2, const Vector3r& shiftVel, bool avoidGranularRatcheting){
	if(avoidGranularRatcheting){
		/* B.C. Comment :
		Short explanation of what we want to avoid :
		Numerical ratcheting is best understood considering a small elastic cycle at a contact between two grains : assuming b1 is fixed, impose this displacement to b2 :
		1. translation "dx" in the normal direction
		2. rotation "a"
		3. translation "-dx" (back to initial position)
		4. rotation "-a" (back to initial orientation)
		If the branch vector used to define the relative shear in rotation×branch is not constant (typically if it is defined from the vector center→contactPoint), then the shear displacement at the end of this cycle is not zero: rotations *a* and *-a* are multiplied by branches of different lengths.
		It results in a finite contact force at the end of the cycle even though the positions and orientations are unchanged, in total contradiction with the elastic nature of the problem. It could also be seen as an *inconsistent energy creation or loss*. Given that DEM simulations tend to generate oscillations around equilibrium (damped mass-spring), it can have a significant impact on the evolution of the packings, resulting for instance in slow creep in iterations under constant load.
		The solution adopted here to avoid ratcheting is as proposed by McNamara and co-workers.
		They analyzed the ratcheting problem in detail - even though they comment on the basis of a cycle that differs from the one shown above. One will find interesting discussions in e.g. DOI 10.1103/PhysRevE.77.031304, even though solution it suggests is not fully applied here (equations of motion are not incorporating alpha, in contradiction with what is suggested by McNamara et al.).
		 */
		// For sphere-facet contact this will give an erroneous value of relative velocity...
		Real alpha = (radius1+radius2)/(radius1+radius2-penetrationDepth);
		Vector3r relativeVelocity = (rbp2->vel-rbp1->vel)*alpha + rbp2->angVel.cross(-radius2*normal) - rbp1->angVel.cross(radius1*normal);
		relativeVelocity+=alpha*shiftVel;
		return relativeVelocity;
	} else {
		// It is correct for sphere-sphere and sphere-facet contact
		Vector3r c1x = (contactPoint - rbp1->pos);
		Vector3r c2x = (contactPoint - rbp2->pos - shift2);
		Vector3r relativeVelocity = (rbp2->vel+rbp2->angVel.cross(c2x)) - (rbp1->vel+rbp1->angVel.cross(c1x));
		relativeVelocity+=shiftVel;
		return relativeVelocity;}
}

Vector3r ScGeom::getIncidentVel(const State* rbp1, const State* rbp2, Real dt, bool avoidGranularRatcheting){
	//Just pass null shift to the periodic version
	return getIncidentVel(rbp1,rbp2,dt,Vector3r::Zero(),Vector3r::Zero(),avoidGranularRatcheting);
}

Vector3r ScGeom::getIncidentVel_py(shared_ptr<Interaction> i, bool avoidGranularRatcheting){
	if(i->geom.get()!=this) throw invalid_argument("ScGeom object is not the same as Interaction.geom.");
	Scene* scene=Omega::instance().getScene().get();
	return getIncidentVel(Body::byId(i->getId1(),scene)->state.get(),Body::byId(i->getId2(),scene)->state.get(),
		scene->dt,
		scene->isPeriodic ? scene->cell->intrShiftPos(i->cellDist): Vector3r::Zero(), // shift2
		scene->isPeriodic ? scene->cell->intrShiftVel(i->cellDist): Vector3r::Zero(), // shiftVel
		avoidGranularRatcheting);
}

Vector3r ScGeom::getRelAngVel(const State* rbp1, const State* rbp2, Real dt){
  	Vector3r relAngVel = (rbp2->angVel-rbp1->angVel);
	return relAngVel;
}

Vector3r ScGeom::getRelAngVel_py(shared_ptr<Interaction> i){
	if(i->geom.get()!=this) throw invalid_argument("ScGeom object is not the same as Interaction.geom.");
	Scene* scene=Omega::instance().getScene().get();
	return getRelAngVel(Body::byId(i->getId1(),scene)->state.get(),Body::byId(i->getId2(),scene)->state.get(),scene->dt);
}

//!Precompute relative rotations (and precompute ScGeom3D)
void ScGeom6D::precomputeRotations(const State& rbp1, const State& rbp2, bool isNew, bool creep){
	if (isNew) {
		initRotations(rbp1,rbp2);
	} else {
		Quaternionr delta((rbp1.ori * (initialOrientation1.conjugate())) * (initialOrientation2 * (rbp2.ori.conjugate())));
		delta.normalize();
		if (creep) delta = delta * twistCreep;
		AngleAxisr aa(delta); // axis of rotation - this is the Moment direction UNIT vector; // angle represents the power of resistant ELASTIC moment
		//Eigen::AngleAxisr(q) returns nan's when q close to identity, next tline fixes the pb.
// add -DSUDODEM_SCGEOM_DEBUG to CXXFLAGS to enable this piece or just do
// #define SUDODEM_SCGEOM_DEBUG //(but do not commit with that enabled in the code)
#ifdef SUDODEM_SCGEOM_DEBUG
		if (isnan(aa.angle())) {
			cerr<<"NaN angle found in angleAxisr(q), for quaternion "<<delta<<", after quaternion product"<<endl;
			cerr<<"rbp1.ori * (initialOrientation1.conjugate())) * (initialOrientation2 * (rbp2.ori.conjugate()) with quaternions :"<<endl;
			cerr<<rbp1.ori<<" * "<<initialOrientation1<<" * "<<initialOrientation2<<" * "<<rbp2.ori<<endl<<" and sub-products :"<<endl<<rbp1.ori * (initialOrientation1.conjugate())<<" * "<<initialOrientation2 * (rbp2.ori.conjugate())<<endl;
			cerr <<"q.w (before normalization) "<<delta.w();
			delta.normalize();
			cerr <<"q.w (after) "<<delta.w()<<endl;
			AngleAxisr bb(delta);
			cerr<<delta<<" "<<bb.angle()<<endl;
		}
#else
		if (isnan(aa.angle())) aa.angle()=0;
#endif
		if (aa.angle() > Mathr::PI) aa.angle() -= Mathr::TWO_PI;   // angle is between 0 and 2*pi, but should be between -pi and pi
		twist = (aa.angle() * aa.axis().dot(normal));
		bending = Vector3r(aa.angle()*aa.axis() - twist*normal);
	}
}

void ScGeom6D::initRotations(const State& state1, const State& state2)
{
	initialOrientation1	= state1.ori;
	initialOrientation2	= state2.ori;
	twist=0;
	bending=Vector3r::Zero();
	twistCreep=Quaternionr(1.0,0.0,0.0,0.0);
}

// pyRegisterClass implementations moved from ScGeom.hpp
void ScGeom::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("ScGeom");
	pybind11::class_<ScGeom, GenericSpheresContact, std::shared_ptr<ScGeom>> _classObj(_module, "ScGeom", "Class representing :yref:`geometry<IGeom>` of a contact point between two :yref:`bodies<Body>`. It is more general than sphere-sphere contact even though it is primarily focused on spheres interactions (reason for the 'Sc' namming); it is also used for representing contacts of a :yref:`Sphere` with non-spherical bodies (:yref:`Facet`, :yref:`Plane`,  :yref:`Box`, :yref:`ChainedCylinder`), or between two non-spherical bodies (:yref:`ChainedCylinder`). The contact has 3 DOFs (normal and 2×shear) and uses incremental algorithm for updating shear.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("penetrationDepth", &ScGeom::penetrationDepth, "Penetration distance of spheres (positive if overlapping)");
	_classObj.def_readwrite("shearInc", &ScGeom::shearInc, "Shear displacement increment in the last step");
	_classObj.def("incidentVel", &ScGeom::getIncidentVel_py, pybind11::arg("i"), pybind11::arg("avoidGranularRatcheting") = true, "Return incident velocity of the interaction.");
	_classObj.def("relAngVel", &ScGeom::getRelAngVel_py, pybind11::arg("i"), "Return relative angular velocity of the interaction.");
}

void ScGeom6D::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("ScGeom6D");
	pybind11::class_<ScGeom6D, ScGeom, std::shared_ptr<ScGeom6D>> _classObj(_module, "ScGeom6D", "Class representing :yref:`geometry<IGeom>` of two :yref:`bodies<Body>` in contact. The contact has 6 DOFs (normal, 2×shear, twist, 2xbending) and uses :yref:`ScGeom` incremental algorithm for updating shear.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("initialOrientation1", &ScGeom6D::initialOrientation1, "Orientation of body 1 one at initialisation time");
	_classObj.def_readwrite("initialOrientation2", &ScGeom6D::initialOrientation2, "Orientation of body 2 one at initialisation time");
	_classObj.def_readwrite("twistCreep", &ScGeom6D::twistCreep, "Stored creep, substracted from total relative rotation for computation of elastic moment");
	_classObj.def_readwrite("twist", &ScGeom6D::twist, "Elastic twist angle of the contact.");
	_classObj.def_readwrite("bending", &ScGeom6D::bending, "Bending at contact as a vector defining axis of rotation and angle (angle=norm).");
}

void ChCylGeom6D::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("ChCylGeom6D");
	pybind11::class_<ChCylGeom6D, ScGeom6D, std::shared_ptr<ChCylGeom6D>> _classObj(_module, "ChCylGeom6D", "Test");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("relPos1", &ChCylGeom6D::relPos1, "");
	_classObj.def_readwrite("relPos2", &ChCylGeom6D::relPos2, "");
}
