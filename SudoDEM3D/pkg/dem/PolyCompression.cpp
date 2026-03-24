/*************************************************************************
*  Copyright (C) 2016 by Zhswee		        		 	        		 *
*  zhswee@gmail.com      					  							 *
*																		 *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#ifdef SUDODEM_CGAL
#include<sudodem/pkg/dem/PolyCompression.hpp>
#include<sudodem/pkg/common/Sphere.hpp>
#include<sudodem/pkg/common/Box.hpp>
#include<sudodem/pkg/dem/ScGeom.hpp>
#include<sudodem/pkg/dem/Polyhedra.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>
#include<sudodem/core/State.hpp>
#include<assert.h>
#include<sudodem/core/Scene.hpp>
#include<sudodem/pkg/dem/Shop.hpp>

#include<sudodem/core/Omega.hpp>
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/core/Interaction.hpp>
#include<sudodem/pkg/common/ElastMat.hpp>



#include <sys/syscall.h>//get pid of a thread

CREATE_LOGGER(PolyCompressionEngine);

PolyCompressionEngine::~PolyCompressionEngine(){}

void PolyCompressionEngine::updateStiffness ()
{
	stiffness = 0;
	InteractionContainer::iterator ii    = scene->interactions->begin();
	InteractionContainer::iterator iiEnd = scene->interactions->end();
	if (Sphere_on)//spherical particles
	{
		for(  ; ii!=iiEnd ; ++ii ) if ((*ii)->isReal())
		{
			const shared_ptr<Interaction>& contact = *ii;
			Real fn = (static_cast<VolumeFricPhys*>	(contact->phys.get()))->normalForce.norm();
			ScGeom*    geom= static_cast<ScGeom*>(contact->geom.get());
			//Real volume = (static_cast<ScGeom*>(contact->geom.get()))->penetrationVolume;//
			if (fn!=0)
			{
				int id1 = contact->getId1(), id2 = contact->getId2();
				if ( top_wall ==id1 || top_wall==id2 )
				{
					Real ra = geom->radius1;
					Real rb = geom->radius2;
					Real dab = ra + rb - geom->penetrationDepth;
					Real ha = ra - 0.5*dab - (std::pow(ra,2)-std::pow(rb,2))/dab*0.5;
					Real hb = rb - 0.5*dab + (std::pow(ra,2)-std::pow(rb,2))/dab*0.5;
					Real Volume = Mathr::PI/3.*(std::pow(ha,2)*(3.*ra - ha) + std::pow(hb,2)*(3.*rb - hb));

					VolumeFricPhys* currentContactPhysics =
						static_cast<VolumeFricPhys*> ( contact->phys.get() );
					stiffness  += 1.209*pow(Volume,2./3.)*currentContactPhysics->kn;
				}
			}
		}
	}else//polyhedra
	{
		for(  ; ii!=iiEnd ; ++ii ) if ((*ii)->isReal())
		{
			const shared_ptr<Interaction>& contact = *ii;
			Real fn = (static_cast<PolyhedraPhys*>	(contact->phys.get()))->normalForce.norm();
			Real volume = (static_cast<PolyhedraGeom*>(contact->geom.get()))->penetrationVolume;//
			if (fn!=0)
			{
				int id1 = contact->getId1(), id2 = contact->getId2();
				if ( top_wall ==id1 || top_wall==id2 )
				{
					PolyhedraPhys* currentContactPhysics =
					static_cast<PolyhedraPhys*> ( contact->phys.get() );
					stiffness  += 1.209*pow(volume,2./3.)*currentContactPhysics->kn;
				}
			}
		}
	}
}


Real PolyCompressionEngine::ComputeUnbalancedForce( bool maxUnbalanced) {return Shop::unbalancedForce(maxUnbalanced,scene);}


void PolyCompressionEngine::action()
{

	if ( firstRun )
	{
		LOG_INFO ( "First run, will initialize!" );


		getBox();

		Real left = box[left_wall]->state->pos.x();
		Real right = box[right_wall]->state->pos.x();
		Real front = box[front_wall]->state->pos.y();
		Real back = box[back_wall]->state->pos.y();
		Real top = box[top_wall]->state->pos.z();
		Real bottom = box[bottom_wall]->state->pos.z();
		previouswallpos = top;
		width = width0 = right - left;
		depth = depth0 = back - front;
		left_facet_pos = left;
		height = height0 = top - bottom;
		LP_area = width * depth;

		//calculate particlesVolume
		BodyContainer::iterator bi = scene->bodies->begin();
		BodyContainer::iterator biEnd = scene->bodies->end();
		particlesVolume = 0;
		for ( ; bi!=biEnd; ++bi )
		{
			//if((*bi)->isClump()) continue;//no clump for polyhedra
			const shared_ptr<Body>& b = *bi;
			//std::cerr << "getClassName= " << b->shape->getClassName() << "\n";

			if (b->shape->getClassName()=="Polyhedra"){
				const shared_ptr<Polyhedra>& polyhedra = SUDODEM_PTR_CAST<Polyhedra> (b->shape);
				particlesVolume += polyhedra->GetVolume();

			}
			//if ( b->isDynamic() || b->isClumpMember() ) {
			if (b->shape->getClassName()=="Sphere"){
				const shared_ptr<Sphere>& sphere = SUDODEM_PTR_CAST<Sphere> ( b->shape );
				particlesVolume += 1.3333333*Mathr::PI*pow ( sphere->radius, 3 );
			}
		}
		firstRun=false;
	}

	const Real& dt = scene->dt;

	//stress
	if (scene->iter % stiffnessUpdateInterval == 0 || scene->iter<100) updateStiffness();
	// sync thread storage of ForceContainer
	 scene->forces.sync();

	//calculate stress
	//stress[wall_top] = getForce(scene,wall_id[wall_top])/(width*depth);
	if (scene->iter % echo_interval == 0){
		UnbalancedForce = ComputeUnbalancedForce ();
		//getStressStrain();
		std::cerr<<"ubf"<<UnbalancedForce<<", vSn:"<<box[top_wall]->state->pos.z()<<", vSs:"<<getForce(scene,top_wall)[2]/(width0*depth0)
		         <<", allSs:"<<"\n";
	}

	//calculate stress

	//UnbalancedForce = ComputeUnbalancedForce ();
	loadingStress();//add force on the top facets
}



void PolyCompressionEngine::recordData()
{
	if(!out.is_open()){
		assert(!out.is_open());

		std::string fileTemp = file;


		if(fileTemp.empty()) throw ios_base::failure(__FILE__ ": Empty filename.");
		out.open(fileTemp.c_str(), truncate ? fstream::trunc : fstream::app);
		if(!out.good()) throw ios_base::failure(__FILE__ ": I/O error opening file `"+fileTemp+"'.");
	}
	// at the beginning of the file; write column titles
	if(out.tellp()==0)	out<<"iteration sStrain vStrain sStress0 sStress1 unb_force  rotationx rotationy rotationz"<<endl;
	//getStressStrain();
	previousStress[0] = previousStress[1];
	previousStress[1] = shearStress[0]+shearStress[1];

	boxVolume = width0*height*depth0;
	Real overlappingV = GetOverlpV();
	out << std::to_string(scene->iter) << " "
 	<< std::to_string(strain[0]) << " "  //shear strain
 	<< std::to_string(strain[1]) << " "  //volumetric strain
	<< std::to_string(shearStress[0]) << " "
	<< std::to_string(shearStress[1]) << " "
 	<< std::to_string(UnbalancedForce) << " "
 	<< std::to_string(translationAxisz[0]) << " "
	<< std::to_string(translationAxisz[1]) << " "
 	<< std::to_string(translationAxisz[2]) << " "
	//<< std::to_string(syscall(SYS_gettid)) << " "
 	<< endl;
}

Real PolyCompressionEngine::GetOverlpV(){//not used
	Real overlappingV = 0.0;
	InteractionContainer::iterator ii    = scene->interactions->begin();
	InteractionContainer::iterator iiEnd = scene->interactions->end();
	for(  ; ii!=iiEnd ; ++ii ) if ((*ii)->isReal())
	{
		const shared_ptr<Interaction>& contact = *ii;
		//Real fn = (static_cast<PolyhedraPhys*>	(contact->phys.get()))->normalForce.norm();
		Real volume = (static_cast<PolyhedraGeom*>(contact->geom.get()))->penetrationVolume;//
		overlappingV += volume;
	}
	return overlappingV;
}


void PolyCompressionEngine::loadingStress(){
	scene->forces.sync();
	//Real translation=normal[wall].dot(getForce(scene,wall) + getForce(scene,wall+1)-resultantForce);

	Vector3r facet_normal(0.,-1.,0.);
	//Real translation=facet_normal.dot(getForce(scene,top_wall) - Vector3r(0, goal*width0*depth0, 0));
	Real wall_f = getForce(scene,top_wall)[2];
	Real translation= wall_f - goal*LP_area;//zforce
	Real threshold = f_threshold * goal*LP_area;
	Real trans=0.;
	//
	//if (scene->iter % stiffnessUpdateInterval == 0 || scene->iter<100){
	//if translation is very large, then need to reassign the initial values
	//Real wpos = normal[wall].dot(p->pos);
	Real wpos = box[top_wall]->state->pos.z();//z
	//FIXME:if force on the top wall is equal to zero
	//}
	//moving the wall with the maximum velocity if the current fouce on the wall is less than 80% of the target
	//this case including that the wall is not touching with the particles
	//if (std::abs(wall_f) < 0.8*goal*LP_area){//
	if (std::abs(wall_f) < 1e-16){//wall_f is zero or so
		 box[top_wall]->state->vel = Vector3r(0.,0.0,wall_max_vel * Mathr::Sign(translation));
		 //std::cerr<<"1"<<"\n";
		 //Caution:the wall may move too fast to make the most-top particles escape through the wall
	}else{//
		//numerically find the target velocity of the top wall
		if (translation != pos[3] && pos[3]!=0) {
			if (std::abs(translation) > threshold && translation*pos[3] >0 && std::abs(translation) > std::abs(pos[3])){
				trans = 1.1*(pos[1] - wpos);
				//std::cerr<<"2"<<"\n";
			}
			else{
				Real x3 = wpos - translation*(wpos-pos[1])/(translation -pos[3]);
				trans = x3 - wpos;
				//std::cerr<<"3"<<"\n";
			}
		}
		else{
			if (stiffness!=0){
				//std::cerr<<"4"<<"\n";
				trans = translation / stiffness;
				//std::cerr << "translation:" <<translation<<"wall_max_vel:"<<wall_max_vel<< "\n";

				//translation = std::abs(translation) * Mathr::Sign(translation);

			}
			else {
			  trans = wall_max_vel * Mathr::Sign(translation)*scene->dt;
			  //std::cerr<<"5"<<"\n";
			}

		}
		trans = std::min( std::abs(trans), wall_max_vel*scene->dt ) * Mathr::Sign(translation);
		box[top_wall]->state->vel = Vector3r(0.,0.,trans/scene->dt);
		previouswallpos = wpos;
		pos[0] = pos[1];
		pos[1] = wpos;
		pos[2] = pos[3];
		pos[3] = translation ;

		/*
		if(isnan(box[top_wall]->state->vel[0]) || isnan(box[top_wall]->state->vel[1]) || isnan(box[top_wall]->state->vel[2])){//for debug
		  box[top_wall]->state->vel = Vector3r::Zero();

		  char filename[20];
		  sprintf(filename,"%lu",syscall(SYS_gettid));
		  FILE * fin = fopen(filename,"a");
		  fprintf(fin,"translation:\t%e\n",translation);
		  fprintf(fin,"F\t%e\t%e\t%e\t%e\n",pos[0],pos[1],pos[2],pos[3]);
		  fclose(fin);
		  scene->stopAtIter=scene->iter+1;
		}*/

	}
	/*
	if (translation !=0){



	}else{
		 box[top_wall]->state->vel = Vector3r(0.,0.0,wall_max_vel * Mathr::Sign(translation));
		 box[top_wall]->state->pos = Vector3r(0.,0.,previouswallpos);
	}
  	*/
}

void PolyCompressionEngine::getBox(){
	for(int i = 0; i < 6; i++){
		 box[i] = Body::byId(i);
	}
	//find particles at the boundary

}

Real PolyCompressionEngine::getResultantF(){
	scene->forces.sync();
	Real f=0.;
	/*
	for(int i=0;i<downBox.size();i++){
		//f += getForce(scene,downBox[i])[0];//force along x,f=scene->forces.getForce(id);
		f += scene->forces.getForce(downBox[i])[0];
	}
	*/
	return f;
}
/*
void PolyCompressionEngine::getStressStrain(){
	scene->forces.sync();
	//get forces on the left and right facets of the up box
	//Real f = getForce(scene,facet_uleft1_id)[0] + getForce(scene,facet_uleft2_id)[0]+getForce(scene,facet_uright1_id)[0] + getForce(scene,facet_uright2_id)[0] ;
	//Real f = getForce(scene,facet_uright1_id)[0] + getForce(scene,facet_uright2_id)[0] ;

	Real left = box[facet_uleft1_id]->state->pos.x();
	width = width0 - std::abs(left - left_facet_pos);
	strain[0] = std::abs(left - left_facet_pos)/width0;//shear strain
	Real top = box[top_wall]->state->pos.y();
	Real bottom = box[bottom_wall]->state->pos.y();
	height = top - bottom - thickness;
	strain[1] = (height - height0)/height0;//volumetric strain; positive sign for expanding
	//shearStress[0] = (getForce(scene,facet_dleft1_id)[0]) / (depth0*width);
	//shearStress[1] = (getForce(scene,facet_dright1_id)[0]) / (depth0*width);
	Real f=getResultantF()/ (depth0*width);
	shearStress[0] = -f;
	shearStress[1] = (getForce(scene,facet_dright1_id)[0]) / (depth0*width)+(getForce(scene,facet_dleft1_id)[0]) / (depth0*width);
}
*/

void PolyCompressionEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("PolyCompressionEngine");
	pybind11::class_<PolyCompressionEngine, BoundaryController, std::shared_ptr<PolyCompressionEngine>> _classObj(_module, "PolyCompressionEngine", "An engine maintaining constant stresses or constant strain rates on some boundaries of a parallepipedic packing.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("stiffnessUpdateInterval", &PolyCompressionEngine::stiffnessUpdateInterval, "target strain rate (./s)");
	_classObj.def_readwrite("echo_interval", &PolyCompressionEngine::echo_interval, "interval of iteration for printing run info.");
	_classObj.def_readwrite("file", &PolyCompressionEngine::file, "Name of file to save to; must not be empty.");
	_classObj.def_readonly("height", &PolyCompressionEngine::height, "size of the box z(2-axis) |yupdate|");
	_classObj.def_readonly("width", &PolyCompressionEngine::width, "size of the box x(0-axis) |yupdate|");
	_classObj.def_readonly("depth", &PolyCompressionEngine::depth, "size of the box y(1-axis) |yupdate|");
	_classObj.def_readonly("LP_area", &PolyCompressionEngine::LP_area, "area of the loading plates");
	_classObj.def_readwrite("f_threshold", &PolyCompressionEngine::f_threshold, "threshold for approaching the target fast.");
	_classObj.def_readwrite("height0", &PolyCompressionEngine::height0, "Reference size for strain definition.");
	_classObj.def_readwrite("width0", &PolyCompressionEngine::width0, "Reference size for strain definition.");
	_classObj.def_readwrite("depth0", &PolyCompressionEngine::depth0, "Reference size for strain definition.");
	_classObj.def_readwrite("wall_equal_tol", &PolyCompressionEngine::wall_equal_tol, "wall equilibrium tolerance after each increament.");
	_classObj.def_readwrite("majorF_tol", &PolyCompressionEngine::majorF_tol, "stress gradual reduction torlerance of top and bottom walls when compressing");
	_classObj.def_readwrite("unbf_tol", &PolyCompressionEngine::unbf_tol, "tolerance of UnbalancedForce");
	_classObj.def_readwrite("wall_max_vel", &PolyCompressionEngine::wall_max_vel, "max velocity of the top  facet when loading");
	_classObj.def_readwrite("step_interval", &PolyCompressionEngine::step_interval, "step interval during each displacement increament");
	_classObj.def_readwrite("h0", &PolyCompressionEngine::h0, "boundary h0");
	_classObj.def_readwrite("w0", &PolyCompressionEngine::w0, "boundary w0");
	_classObj.def_readwrite("d0", &PolyCompressionEngine::d0, "boundary d0");
	_classObj.def_readwrite("Sphere_on", &PolyCompressionEngine::Sphere_on, "true for spherical particles, false for polyhedral particles");
	_classObj.def_readwrite("controlFlag", &PolyCompressionEngine::controlFlag, "this option is used to control the stress");
	_classObj.def_readwrite("goal", &PolyCompressionEngine::goal, "prescribed stress/strain rate on axis 1");
	_classObj.def_readwrite("total_incr", &PolyCompressionEngine::total_incr, "the total increaments");
	_classObj.def_readwrite("outinfo_interval", &PolyCompressionEngine::outinfo_interval, "the iteration interval at which displaying the info for monitoring");
	_classObj.def_readwrite("savedata_interval", &PolyCompressionEngine::savedata_interval, "the interval steps between two savedata operations");
	_classObj.def_readwrite("max_vel", &PolyCompressionEngine::max_vel, "Maximum allowed walls velocity [m/s].");
	_classObj.def_readonly("externalWork", &PolyCompressionEngine::externalWork, "Energy provided by boundaries.|yupdate|");
	_classObj.def_readwrite("thickness", &PolyCompressionEngine::thickness, "thickness of the board");
	_classObj.def_readwrite("shearRate", &PolyCompressionEngine::shearRate, "target shear rate in direction x (./s)");
	_classObj.def_readwrite("currentStrainRate2", &PolyCompressionEngine::currentStrainRate2, "current strain rate in direction 2");
	_classObj.def_readwrite("UnbalancedForce", &PolyCompressionEngine::UnbalancedForce, "mean resultant forces divided by mean contact force");
	_classObj.def_readwrite("frictionAngleDegree", &PolyCompressionEngine::frictionAngleDegree, "Value of friction used in the simulation if (updateFrictionAngle)");
	_classObj.def_readwrite("updateFrictionAngle", &PolyCompressionEngine::updateFrictionAngle, "Switch to activate the update of the intergranular friction");
	_classObj.def_readwrite("Confining", &PolyCompressionEngine::Confining, "Switch to confine or shear");
	_classObj.def_readwrite("Key", &PolyCompressionEngine::Key, "A string appended at the end of all files");
	_classObj.def_property_readonly("strain", &PolyCompressionEngine::get_strain, "Current strain in a vector (exx,eyy,ezz).");
	_classObj.def_property_readonly("porosity", &PolyCompressionEngine::get_porosity, "Porosity of the packing.");
	_classObj.def_property_readonly("boxVolume", &PolyCompressionEngine::get_boxVolume, "Total packing volume.");
}

#endif
