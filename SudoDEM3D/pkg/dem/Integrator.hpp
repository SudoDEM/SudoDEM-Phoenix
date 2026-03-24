#pragma once

#include<sudodem/core/TimeStepper.hpp>

class Integrator;


typedef std::vector<Real> stateVector;// Currently, we are unable to use Eigen library within odeint
typedef std::vector<std::vector<std::shared_ptr<Engine>>> slaveContainer;

/*Observer used to update the state of the scene*/
class observer
{
	Integrator* integrator;
public:
	observer(Integrator* _in):integrator(_in){}
        void operator()( const stateVector& /* x */ , Real /* t */ ) const;
};

//[ ode_wrapper
template< class Obj , class Mem >
class ode_wrapper
{
    Obj m_obj;
    Mem m_mem;

public:

    ode_wrapper( Obj obj , Mem mem ) : m_obj( obj ) , m_mem( mem ) { }

    template< class State , class Deriv , class Time >
    void operator()( const State &x , Deriv &dxdt , Time t )
    {
        (m_obj.*m_mem)( x , dxdt , t );
    }
};

template< class Obj , class Mem >
ode_wrapper< Obj , Mem > make_ode_wrapper( Obj obj , Mem mem )
{
    return ode_wrapper< Obj , Mem >( obj , mem );
}
//]



class Integrator: public TimeStepper {

		public:

		stateVector accumstateofthescene;
		stateVector accumstatedotofthescene;
		stateVector resetstate;
		Real timeresetvalue;
		slaveContainer slaves;
		Real integrationsteps;
		Real maxVelocitySq;
		Real updatingDispFactor;
		
		#ifdef SUDODEM_OPENMP
			vector<Real> threadMaxVelocitySq;
			void ensureSync(); 
			bool syncEnsured;
		#endif
		
		Integrator() : timeresetvalue(0), integrationsteps(0), maxVelocitySq(NaN), updatingDispFactor(0) 
		{
			#ifdef SUDODEM_OPENMP
				threadMaxVelocitySq.resize(omp_get_max_threads());
				syncEnsured = false;
			#endif
		}
		
		inline void evaluateQuaternions(const stateVector &);

		typedef vector<vector<shared_ptr<Engine> > > slaveContainer;

		virtual void action();

		virtual void system(const stateVector&, stateVector&, Real);

		virtual bool isActivated(){return true;}
		pybind11::list slaves_get();

		stateVector& getSceneStateDot();

		bool saveCurrentState(Scene const* ourscene);

		bool resetLastState(void);

		void slaves_set(const pybind11::list& slaves);

		stateVector& getCurrentStates(void);

		bool setCurrentStates(stateVector);

		void saveMaximaDisplacement(const shared_ptr<Body>& b);

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(TimeStepper, slaves, integrationsteps, maxVelocitySq);
		
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Integrator, TimeStepper);