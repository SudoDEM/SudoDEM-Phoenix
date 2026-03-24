#pragma once

#include<sudodem/lib/opengl/OpenGLWrapper.hpp>
#include<sudodem/lib/opengl/GLUtils.hpp>
#include<sudodem/pkg/common/GLDrawFunctors.hpp>
#include<sudodem/pkg/common/NormShearPhys.hpp>
#ifdef __APPLE__
	#include<OpenGL/glu.h>
#else
	#include<GL/glu.h>
#endif



class Gl1_NormPhys: public GlIPhysFunctor{
		static GLUquadric* gluQuadric; // needed for gluCylinder, initialized by ::go if no initialized yet
	public:
		static Real maxFn;
		static Real refRadius;
		static Real maxRadius;
		static int signFilter;
		static int slices;
		static int stacks;
		static Real maxWeakFn;
		static int weakFilter;
		static Real weakScale;
		
		virtual void go(const shared_ptr<IPhys>&,const shared_ptr<Interaction>&,const shared_ptr<Body>&,const shared_ptr<Body>&,bool wireFrame) override;
		virtual string renders() const override { return "NormPhys";}; FUNCTOR1D(NormPhys);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(Gl1_NormPhys, GlIPhysFunctor);

