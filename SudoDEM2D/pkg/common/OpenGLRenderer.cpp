// © 2004 Olivier Galizzi <olivier.galizzi@imag.fr>
// © 2008 Václav Šmilauer <eudoxos@arcig.cz>

#ifdef SUDODEM_OPENGL

#include"OpenGLRenderer.hpp"
#include<sudodem/lib/opengl/OpenGLWrapper.hpp>
#include<sudodem/lib/opengl/GLUtils.hpp>
#include<sudodem/core/Timing.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/pkg/common/Aabb.hpp>
#include<sudodem/lib/pyutil/gil.hpp>

#ifdef __APPLE__
#  include <OpenGL/glu.h>
#  include <OpenGL/gl.h>
#  include <GLUT/glut.h>
#else
#  include <GL/glu.h>
#  include <GL/gl.h>
#  include <GL/glut.h>
#endif

CREATE_LOGGER(OpenGLRenderer);

void GlExtraDrawer::render(){ throw runtime_error("GlExtraDrawer::render called from class "+getClassName()+". (did you forget to override it in the derived class?)"); }

bool OpenGLRenderer::initDone=false;
//const int OpenGLRenderer::numClipPlanes;
OpenGLRenderer::~OpenGLRenderer(){}


void OpenGLRenderer::init(){
	std::cerr << "DEBUG OpenGLRenderer::init: scanning ClassRegistry for GL functors" << std::endl;
	
	// Use ClassRegistry instead of dynlibs to find GL functors
	// Get all registered classes from ClassRegistry
	// std::cerr << "DEBUG: About to get registered classes..." << std::endl;
	std::vector<std::string> allClasses = ClassRegistry::instance().getRegisteredClasses();
	// std::cerr << "DEBUG: ClassRegistry has " << allClasses.size() << " classes" << std::endl;
	
	for(const std::string& className : allClasses){
		std::string baseClassName = ClassRegistry::instance().getBaseClassName(className);
		
		// Check if this class inherits from GL functor base classes
		// We need to check the full inheritance chain
		// Note: class names may be mangled, so we check if the name contains the target substring
		std::set<std::string> visited;
		std::function<bool(const std::string&, const std::string&)> inheritsFrom = 
			[&](const std::string& cls, const std::string& base) -> bool {
				// Check if cls contains base as substring (handles mangled names)
				if (cls.find(base) != std::string::npos) return true;
				if (visited.count(cls)) return false;
				visited.insert(cls);
				std::string parent = ClassRegistry::instance().getBaseClassName(cls);
				if (parent.empty()) return false;
				return inheritsFrom(parent, base);
			};
		
		// Skip the base classes themselves (they have Functor as direct base)
		bool isBaseClass = (baseClassName.find("Functor") != std::string::npos && 
		                    baseClassName.find("Gl") == std::string::npos);
		
		if (!isBaseClass && inheritsFrom(baseClassName, "GlBoundFunctor")) {
			std::cerr << "DEBUG: Adding " << className << " to boundFunctorNames (base=" << baseClassName << ")" << std::endl;
			boundFunctorNames.push_back(className);
		}
		if (!isBaseClass && inheritsFrom(baseClassName, "GlShapeFunctor")) {
			std::cerr << "DEBUG: Adding " << className << " to shapeFunctorNames (base=" << baseClassName << ")" << std::endl;
			shapeFunctorNames.push_back(className);
		}
		if (!isBaseClass && inheritsFrom(baseClassName, "GlIGeomFunctor")) {
			geomFunctorNames.push_back(className);
		}
		if (!isBaseClass && inheritsFrom(baseClassName, "GlIPhysFunctor")) {
			physFunctorNames.push_back(className);
		}
	}
	
	// std::cerr << "DEBUG: shapeFunctorNames has " << shapeFunctorNames.size() << " functors" << std::endl;
	// std::cerr << "DEBUG: About to call initgl()..." << std::endl;
	initgl(); // creates functor objects in the proper sense
	// std::cerr << "DEBUG: initgl() completed" << std::endl;

	//clipPlaneNormals.resize(numClipPlanes);

	static bool glutInitDone=false;
	if(!glutInitDone){
#if QT_VERSION_MAJOR >= 6
		// When using Qt6, GLUT initialization is not needed and causes conflicts
		// Qt6's QOpenGLWidget provides all necessary OpenGL functionality
		// Skip GLUT initialization to avoid "glutInit being called a second time" error
		std::cerr << "DEBUG: Skipping GLUT initialization (Qt6 provides OpenGL context)" << std::endl;
#else
		glutInit(&Omega::instance().origArgc,Omega::instance().origArgv);
		/* transparent disks (still not working): glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_ALPHA); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE); */
#endif
		glutInitDone=true;
	}

	// std::cerr << "DEBUG: Setting initDone=true" << std::endl;
	initDone=true;
	// std::cerr << "DEBUG: OpenGLRenderer::init() completed successfully" << std::endl;
	// glGetError crashes at some machines?! Was never really useful, anyway.
	// reported http://www.mail-archive.com/sudodem-users@lists.launchpad.net/msg01482.html
	#if 0
		int e=glGetError();
		if(e!=GL_NO_ERROR) throw runtime_error((string("OpenGLRenderer::init returned GL error ")+std::to_string(e)).c_str());
	#endif
}

void OpenGLRenderer::setBodiesRefPos(){
	LOG_DEBUG("(re)initializing reference positions.");
	for(const shared_ptr<Body>& b : *scene->bodies) if(b && b->state) { b->state->refPos=b->state->pos;}
	scene->cell->refHSize=scene->cell->hSize;
}


void OpenGLRenderer::initgl(){
	LOG_DEBUG("(re)initializing GL for gldraw methods.\n");
	
	// Helper function to create functors from ClassRegistry
	struct FunctorCreator {
		static shared_ptr<Functor> create(const std::string& className) {
			shared_ptr<Factorable> obj = ClassRegistry::instance().create(className);
			if (!obj) return shared_ptr<Functor>();
			return dynamic_pointer_cast<Functor>(obj);
		}
	};
	
	// Setup shape dispatcher
	shapeDispatcher.clearMatrix();
	for(string& s : shapeFunctorNames) {
		shared_ptr<Functor> f = FunctorCreator::create(s);
		if (f) {
			shared_ptr<GlShapeFunctor> typedF = dynamic_pointer_cast<GlShapeFunctor>(f);
			if (typedF) { typedF->initgl(); shapeDispatcher.add(typedF); }
		}
	}
	
	// Setup other dispatchers
	boundDispatcher.clearMatrix();
	for(string& s : boundFunctorNames) {
		shared_ptr<Functor> f = FunctorCreator::create(s);
		if (f) {
			shared_ptr<GlBoundFunctor> typedF = dynamic_pointer_cast<GlBoundFunctor>(f);
			if (typedF) { typedF->initgl(); boundDispatcher.add(typedF); }
		}
	}
	
	geomDispatcher.clearMatrix();
	for(string& s : geomFunctorNames) {
		shared_ptr<Functor> f = FunctorCreator::create(s);
		if (f) {
			shared_ptr<GlIGeomFunctor> typedF = dynamic_pointer_cast<GlIGeomFunctor>(f);
			if (typedF) { typedF->initgl(); geomDispatcher.add(typedF); }
		}
	}
	
	physDispatcher.clearMatrix();
	for(string& s : physFunctorNames) {
		shared_ptr<Functor> f = FunctorCreator::create(s);
		if (f) {
			shared_ptr<GlIPhysFunctor> typedF = dynamic_pointer_cast<GlIPhysFunctor>(f);
			if (typedF) { typedF->initgl(); physDispatcher.add(typedF); }
		}
	}
}

/*bool OpenGLRenderer::pointClipped(const Vector3r& p){
	if(numClipPlanes<1) return false;
	for(int i=0;i<numClipPlanes;i++) if(clipPlaneActive[i]&&(p-clipPlaneSe3[i].position).dot(clipPlaneNormals[i])<0) return true;
	return false;
}*/


void OpenGLRenderer::setBodiesDispInfo(){
	if(scene->bodies->size()!=bodyDisp.size()) {
		bodyDisp.resize(scene->bodies->size());
		for (unsigned k=0; k<scene->bodies->size(); k++){ bodyDisp[k].hidden=0;bodyDisp[k].isDisplayed=1;}}
	bool scaleRotations=(rotScale!=1.0);
	bool scaleDisplacements=(dispScale!=Vector2r::Ones());
	for(const shared_ptr<Body>& b : *scene->bodies){
		if(!b || !b->state) continue;
		
		// Lock State during access to prevent race conditions
		std::lock_guard<std::mutex> stateLock(b->state->updateMutex);
		
		size_t id=b->getId();
		const Vector2r& pos=b->state->pos; const Vector2r& refPos=b->state->refPos;
		const Rotationr& ori=b->state->ori; const Rotationr& refOri=b->state->refOri;
		Vector2r cellPos=(!scene->isPeriodic ? pos : scene->cell->wrapShearedPt(pos)); // inside the cell if periodic, same as pos otherwise
		//bodyDisp[id].isDisplayed=!pointClipped(cellPos);
		// if no scaling and no periodic, return quickly
		if(!(scaleDisplacements||scaleRotations||scene->isPeriodic)){ bodyDisp[id].pos=pos; bodyDisp[id].ori=ori; continue; }
		// apply scaling
		bodyDisp[id].pos=cellPos; // point of reference (inside the cell for periodic)
		if(scaleDisplacements) bodyDisp[id].pos+=dispScale.cwiseProduct(Vector2r(pos-refPos)); // add scaled translation to the point of reference
		if(!scaleRotations) bodyDisp[id].ori=ori;
		else{
			Rotationr relRot=refOri.inverse()*ori;
			//AngleAxisr aa(relRot);
			relRot.angle()*=rotScale;
			bodyDisp[id].ori=refOri*relRot;
		}
	}
}

// draw periodic cell, if active
void OpenGLRenderer::drawPeriodicCell(){
	if(!scene->isPeriodic) return;
	glColor3v(cellColor);
	glPushMatrix();
		// Vector2r size=scene->cell->getSize();
		const Matrix2r& hSize=scene->cell->hSize;
		if(dispScale!=Vector2r::Ones()){
			const Matrix2r& refHSize(scene->cell->refHSize);
			Matrix2r scaledHSize;
			for(int i=0; i<2; i++) scaledHSize.col(i)=refHSize.col(i)+dispScale.cwiseProduct(Vector2r(hSize.col(i)-refHSize.col(i)));
			GLUtils::Parallelogram(scaledHSize.col(0),scaledHSize.col(1));
		} else {
			GLUtils::Parallelogram(hSize.col(0),hSize.col(1));
		}
	glPopMatrix();
}

void OpenGLRenderer::resetSpecularEmission(){
	glMateriali(GL_FRONT, GL_SHININESS, 80);
	const GLfloat glutMatSpecular[4]={0.3,0.3,0.3,0.5};
	const GLfloat glutMatEmit[4]={0.2,0.2,0.2,1.0};
	glMaterialfv(GL_FRONT,GL_SPECULAR,glutMatSpecular);
	glMaterialfv(GL_FRONT,GL_EMISSION,glutMatEmit);
}

void OpenGLRenderer::render(const shared_ptr<Scene>& _scene,Body::id_t selection){

	gilLock lockgil;
	if(!initDone) init();
	assert(initDone);
	selId = selection;

	scene=_scene;

	// assign scene inside functors
	boundDispatcher.updateScenePtr();
	geomDispatcher.updateScenePtr();
	physDispatcher.updateScenePtr();
	shapeDispatcher.updateScenePtr();
	// stateDispatcher.updateScenePtr();

	// just to make sure, since it is not initialized by default
	if(!scene->bound) scene->bound=shared_ptr<Aabb>(new Aabb);

	// recompute emissive light colors for highlighted bodies
	Real now=TimingInfo::getNow(/*even if timing is disabled*/true)*1e-9;
	highlightEmission0[0]=highlightEmission0[1]=highlightEmission0[2]=.8*normSquare(now,1);
	highlightEmission1[0]=highlightEmission1[1]=highlightEmission0[2]=.5*normSaw(now,2);

	// clipping
	/*
	assert(clipPlaneNormals.size()==(size_t)numClipPlanes);
	for(size_t i=0;i<(size_t)numClipPlanes; i++){
		// someone could have modified those from python and truncate the vectors; fill those here in that case
		if(i==clipPlaneSe3.size()) clipPlaneSe3.push_back(Se3r(Vector3r::Zero(),Quaternionr::Identity()));
		if(i==clipPlaneActive.size()) clipPlaneActive.push_back(false);
		if(i==clipPlaneNormals.size()) clipPlaneNormals.push_back(Vector3r::UnitX());
		// end filling stuff modified from python
		if(clipPlaneActive[i]) clipPlaneNormals[i]=clipPlaneSe3[i].orientation*Vector3r(0,0,1);
		// glBegin(GL_LINES);glVertex3v(clipPlaneSe3[i].position);glVertex3v(clipPlaneSe3[i].position+clipPlaneNormals[i]);glEnd();
	}*/
	// set displayed Se3 of body (scaling) and isDisplayed (clipping)
	setBodiesDispInfo();

	glClearColor(bgColor[0],bgColor[1],bgColor[2],1.0);

	// set light sources
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1); // important: do lighting calculations on both sides of polygons

	const GLfloat pos[4]	= {(float) lightPos[0], (float) lightPos[1], (float) lightPos[2],1.0};
	const GLfloat ambientColor[4]={0.2,0.2,0.2,1.0};
	const GLfloat specularColor[4]={1,1,1,1.f};
	const GLfloat diffuseLight[4] = { (float) lightColor[0], (float) lightColor[1], (float) lightColor[2], 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION,pos);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	if (light1) glEnable(GL_LIGHT0); else glDisable(GL_LIGHT0);

	const GLfloat pos2[4]	= {(float) light2Pos[0], (float) light2Pos[1], (float) light2Pos[2],1.0};
	const GLfloat ambientColor2[4]={0.0,0.0,0.0,1.0};
	const GLfloat specularColor2[4]={1,1,0.6,1.f};
	const GLfloat diffuseLight2[4] = { (float) light2Color[0], (float) light2Color[1], (float) light2Color[2], 1.0f };
	glLightfv(GL_LIGHT1, GL_POSITION,pos2);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularColor2);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientColor2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight2);
	if (light2) glEnable(GL_LIGHT1); else glDisable(GL_LIGHT1);

	glEnable(GL_LIGHTING);

	//glEnable(GL_CULL_FACE);//no need for 2D
	// http://www.sjbaker.org/steve/omniv/opengl_lighting.html
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	//Shared material settings
	resetSpecularEmission();

	drawPeriodicCell();

	if (dof || id) renderDOF_ID();
	//if (bound) renderBound();

	if (shape) renderShape();
	if (bound) renderBound();
	if (intrAllWire) renderAllInteractionsWire();
	if (intrGeom) renderIGeom();
	if (intrPhys) renderIPhys();

	for(const shared_ptr<GlExtraDrawer> d : extraDrawers){
		if(d->dead) continue;
		glPushMatrix();
			d->scene=scene.get();
			d->render();
		glPopMatrix();
	}


}

void OpenGLRenderer::renderAllInteractionsWire(){
	std::lock_guard<std::mutex> lock(scene->interactions->drawloopmutex);
	for(const shared_ptr<Interaction>& i : *scene->interactions){
		if(!i->functorCache.geomExists) continue;
		glColor3v(i->isReal()? Vector3r(0,1,0) : Vector3r(.5,0,1));
		Vector2r p1=Body::byId(i->getId1(),scene)->state->pos;
		const Vector2r& size=scene->cell->getSize();
		Vector2r shift2(i->cellDist[0]*size[0],i->cellDist[1]*size[1]);
		// in sheared cell, apply shear on the mutual position as well
		shift2=scene->cell->shearPt(shift2);
		Vector2r rel=Body::byId(i->getId2(),scene)->state->pos+shift2-p1;
		if(scene->isPeriodic) p1=scene->cell->wrapShearedPt(p1);
		glBegin(GL_LINES); glVertex2v(p1);glVertex2v(Vector2r(p1+rel));glEnd();
	}
}

void OpenGLRenderer::renderDOF_ID(){
	const GLfloat ambientColorSelected[4]={10.0,0.0,0.0,1.0};
	const GLfloat ambientColorUnselected[4]={0.5,0.5,0.5,1.0};
	for(const shared_ptr<Body> b : *scene->bodies){
		if(!b) continue;
		if(b->shape && ((b->getGroupMask() & mask) || b->getGroupMask()==0)){
			if(!b->state) continue;
			std::lock_guard<std::mutex> stateLock(b->state->updateMutex);
			if(!id && b->state->blockedDOFs==0) continue;
			if(selId==b->getId()){glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambientColorSelected);}
			{ // write text
				glColor3f(1.0-bgColor[0],1.0-bgColor[1],1.0-bgColor[2]);
				unsigned d = b->state->blockedDOFs;
				std::string sDof = std::string()+(((d&State::DOF_X )!=0)?"x":"")+(((d&State::DOF_Y )!=0)?"y":" ")+(((d&State::DOF_RZ)!=0)?"Z":"");
				std::string sId = std::to_string(b->getId());
				std::string str;
				if(dof && id) sId += " ";
				if(id) str += sId;
				if(dof) str += sDof;
				const Vector3r& h(selId==b->getId() ? highlightEmission0 : Vector3r(1,1,1));
				glColor3v(h);
				GLUtils::GLDrawText(str,bodyDisp[b->id].pos,h);
			}
			if(selId == b->getId()){glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambientColorUnselected);}
		}
	}
}

void OpenGLRenderer::renderIGeom(){
	geomDispatcher.scene=scene.get(); geomDispatcher.updateScenePtr();

	{
		std::lock_guard<std::mutex> lock(scene->interactions->drawloopmutex);
		for(const shared_ptr<Interaction>& I : *scene->interactions){
			if(!I->geom) continue; // avoid refcount manipulations if the interaction is not real anyway
			shared_ptr<IGeom> ig(I->geom); // keep reference so that ig does not disappear suddenly while being rendered
			if(!ig) continue;
			const shared_ptr<Body>& b1=Body::byId(I->getId1(),scene), b2=Body::byId(I->getId2(),scene);
			if(!(bodyDisp[I->getId1()].isDisplayed||bodyDisp[I->getId2()].isDisplayed)) continue;
			glPushMatrix(); geomDispatcher(ig,I,b1,b2,intrWire); glPopMatrix();
		}
	}
}


void OpenGLRenderer::renderIPhys(){
	physDispatcher.scene=scene.get(); physDispatcher.updateScenePtr();
	{
		std::lock_guard<std::mutex> lock(scene->interactions->drawloopmutex);
		for(const shared_ptr<Interaction>& I : *scene->interactions){
			shared_ptr<IPhys> ip(I->phys);
			if(!ip) continue;
			const shared_ptr<Body>& b1=Body::byId(I->getId1(),scene), b2=Body::byId(I->getId2(),scene);
			Body::id_t id1=I->getId1(), id2=I->getId2();
			if(!(bodyDisp[id1].isDisplayed||bodyDisp[id2].isDisplayed)) continue;
			glPushMatrix(); physDispatcher(ip,I,b1,b2,intrWire); glPopMatrix();
		}
	}
}

void OpenGLRenderer::renderBound(){
	boundDispatcher.scene=scene.get(); boundDispatcher.updateScenePtr();

	for(const shared_ptr<Body>& b : *scene->bodies){
		if(!b || !b->bound) continue;
		if(!bodyDisp[b->getId()].isDisplayed or bodyDisp[b->getId()].hidden) continue;
		if(b->bound && ((b->getGroupMask()&mask) || b->getGroupMask()==0)){
			glPushMatrix(); boundDispatcher(b->bound,scene.get()); glPopMatrix();
		}
	}
	// since we remove the functor as Scene doesn't inherit from Body anymore, hardcore the rendering routine here
	// for periodic scene, renderPeriodicCell is called separately
	/*if(!scene->isPeriodic){
		if(!scene->bound) scene->updateBound();
		glColor3v(Vector3r(0,1,0));
		Vector2r size=scene->bound->max-scene->bound->min;
		Vector2r center=.5*(scene->bound->min+scene->bound->max);
		glPushMatrix();
			glTranslatev(Vector3r(center[0],center[1],0));
			glScalev(Vector3r(size[0],size[1],0));
			glutWireCube(1);
		glPopMatrix();
	}*/
}

// this function is called for both rendering as well as
// in the selection mode

// nice reading on OpenGL selection
// http://glprogramming.com/red/chapter13.html

void OpenGLRenderer::renderShape(){
	shapeDispatcher.scene=scene.get(); shapeDispatcher.updateScenePtr();

	// instead of const shared_ptr&, get proper shared_ptr;
	// Less efficient in terms of performance, since memory has to be written (not measured, though),
	// but it is still better than crashes if the body gets deleted meanwile.
	//cout<<"here"<<endl;
	for(shared_ptr<Body> b : *scene->bodies){
		//cout<<"here1"<<endl;
		if(!b || !b->shape) continue;
		if(!(bodyDisp[b->getId()].isDisplayed and !bodyDisp[b->getId()].hidden)) continue;
		
		// Lock State during rendering to prevent race conditions with simulation thread
		if(b->state) {
			std::lock_guard<std::mutex> stateLock(b->state->updateMutex);
			Vector2r pos=bodyDisp[b->getId()].pos;
			//Quaternionr ori=bodyDisp[b->getId()].ori;
			//cout<<"here3"<<endl;
			Rotationr ori=bodyDisp[b->getId()].ori;
			if(!b->shape || !((b->getGroupMask()&mask) || b->getGroupMask()==0)) continue;

			// ignored in non-selection mode, use it always
			glPushName(b->id);
			bool highlight=(b->id==selId || (b->clumpId>=0 && b->clumpId==selId) || b->shape->highlight);
			//cout<<"here4"<<highlight<<endl;
			glPushMatrix();
				//AngleAxisr aa(ori);
				glTranslate2v(Vector2r(pos));
				glRotatef(ori.angle()*Mathr::RAD_TO_DEG,0,0,1);
				if(highlight){
					// set hightlight
					// different color for body highlighted by selection and by the shape attribute
					const Vector3r& h((selId==b->id||(b->clumpId>=0 && selId==b->clumpId)) ? highlightEmission0 : highlightEmission1);
					glMaterialv(GL_FRONT_AND_BACK,GL_EMISSION,h);
					glMaterialv(GL_FRONT_AND_BACK,GL_SPECULAR,h);
					shapeDispatcher(b->shape,b->state,wire || b->shape->wire,viewInfo);
					// reset highlight
					resetSpecularEmission();
				} else {
					// no highlight; in case previous functor fiddled with glMaterial
					resetSpecularEmission();
					//cout<<"here2"<<endl;
					shapeDispatcher(b->shape,b->state,wire || b->shape->wire,viewInfo);
				}
			glPopMatrix();
			if(highlight){
				if(!b->bound || wire || b->shape->wire) GLUtils::GLDrawInt(b->getId(),pos);
				else {
					// move the label towards the camera by the bounding box so that it is not hidden inside the body
					const Vector2r& mn=b->bound->min; const Vector2r& mx=b->bound->max; const Vector2r& p=pos;
					Vector2r ext(viewDirection[0]>0?p[0]-mn[0]:p[0]-mx[0],viewDirection[1]>0?p[1]-mn[1]:p[1]-mx[1]); // signed extents towards the camera
					Vector2r dr=-1.01*(viewDirection.dot(ext)*viewDirection);
					dr += pos;
					GLUtils::GLDrawInt(b->getId(),dr,Vector3r::Ones());
				}
			}
			// if the body goes over the cell margin, draw it in positions where the bbox overlaps with the cell in wire
			// precondition: pos is inside the cell.
			if(b->bound && scene->isPeriodic && ghosts){
				const Vector2r& cellSize(scene->cell->getSize());
				pos=scene->cell->unshearPt(pos); // remove the shear component
				// traverse all periodic cells around the body, to see if any of them touches
				Vector2r halfSize=b->bound->max-b->bound->min; halfSize*=.5;
				Vector2r pmin,pmax;
				Vector2i i;
				for(i[0]=-1; i[0]<=1; i[0]++) for(i[1]=-1;i[1]<=1; i[1]++){
					if(i[0]==0 && i[1]==0) continue; // middle; already rendered above
					Vector2r pos2=pos+Vector2r(cellSize[0]*i[0],cellSize[1]*i[1]); // shift, but without shear!
					pmin=pos2-halfSize; pmax=pos2+halfSize;
					if(pmin[0]<=cellSize[0] && pmax[0]>=0 &&
						pmin[1]<=cellSize[1] && pmax[1]>=0) {
						Vector2r pt=scene->cell->shearPt(pos2);
						//if(pointClipped(pt)) continue;
						glLoadName(b->id);
						glPushMatrix();
							glTranslate2v(pt);
							glRotatef(ori.angle()*Mathr::RAD_TO_DEG,0,0,1);
							shapeDispatcher(b->shape,b->state,/*wire*/ true, viewInfo);
						glPopMatrix();
					}
				}
			}
			glPopName();
		}
	}
}


#endif /* SUDODEM_OPENGL */
