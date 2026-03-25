// © 2009 Václav Šmilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/core/Shape.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>


/*! Object representing infinite plane aligned with the coordinate system (axis-aligned wall). */
class Wall: public Shape{
	public:
		virtual ~Wall(); // vtable
		int sense;
		int axis;
		
		Wall() : sense(0), axis(0) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Shape, sense, axis);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
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
	REGISTER_CLASS_NAME_DERIVED(Wall);
	REGISTER_CLASS_INDEX_H(Wall,Shape)
};
REGISTER_SERIALIZABLE_BASE(Wall, Shape);

/*! Functor for computing axis-aligned bounding box
    from axis-aligned wall. Has no parameters. */
class Bo1_Wall_Aabb: public BoundFunctor{
	public:
		virtual void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se2r& se2, const Body*) override;
		FUNCTOR1D(Wall);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Bo1_Wall_Aabb, BoundFunctor, std::shared_ptr<Bo1_Wall_Aabb>> _classObj(_module, "Bo1_Wall_Aabb", "Creates/updates an :yref:`Aabb` of a :yref:`Wall`");
			_classObj.def(pybind11::init<>());
		}
};
REGISTER_SERIALIZABLE_BASE(Bo1_Wall_Aabb, BoundFunctor);

class Fwall : public Shape {
		public:
	Fwall() { createIndex(); }
	virtual ~Fwall();

	// Postprocessed attributes

	/// Fwall's normal
	//Vector3r nf;
	/// Normals of edge
	//Vector2r ne;
	/// Inscribing cirle radius
	//Real icr;
	/// Length of the vertice vector
	Real vl;
	/// vertice vector
	Vector2r vu;//vertice 1 to vertice 2
	Vector2r vertex1;
	Vector2r vertex2;
	Vector2r normal;

	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(Shape, vl, vu, vertex1, vertex2, normal);

	void postLoad(Fwall&);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
		pybind11::class_<Fwall, Shape, std::shared_ptr<Fwall>> _classObj(_module, "Fwall", "Fwall (triangular particle) geometry.");
		_classObj.def(pybind11::init<>());
		_classObj.def_readwrite("vertex1", &Fwall::vertex1, "Vertex positions in local coordinates.");
		_classObj.def_readwrite("vertex2", &Fwall::vertex2, "Vertex positions in local coordinates.");
		_classObj.def_readwrite("normal", &Fwall::normal, "Fwall's normal (in local coordinate system)");
	}
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX_H(Fwall,Shape)
};
REGISTER_SERIALIZABLE_BASE(Fwall, Shape);

class Bo1_Fwall_Aabb : public BoundFunctor{
	public:
		void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se2r& se2, const Body*) override;
		FUNCTOR1D(Fwall);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Bo1_Fwall_Aabb, BoundFunctor, std::shared_ptr<Bo1_Fwall_Aabb>> _classObj(_module, "Bo1_Fwall_Aabb", "Creates/updates an :yref:`Aabb` of a :yref:`Fwall`.");
			_classObj.def(pybind11::init<>());
		}
};
REGISTER_SERIALIZABLE_BASE(Bo1_Fwall_Aabb, BoundFunctor);

#ifdef SUDODEM_OPENGL
	#include<sudodem/pkg/common/GLDrawFunctors.hpp>
	class Gl1_Wall: public GlShapeFunctor{
		public:
			int div = 20;

			virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&) override;

			SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {

						pybind11::class_<Gl1_Wall, Functor, std::shared_ptr<Gl1_Wall>> _classObj(_module, "Gl1_Wall", "Renders :yref:`Wall` object");

						_classObj.def(pybind11::init<>());

						_classObj.def_readwrite("div", &Gl1_Wall::div, "Number of divisions of the wall inside visible scene part.");

					}
		RENDERS(Wall);
	};
	REGISTER_SERIALIZABLE_BASE(Gl1_Wall, GlShapeFunctor);
	class Gl1_Fwall : public GlShapeFunctor
	{
		public:
			virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&) override;

			SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
				pybind11::class_<Gl1_Fwall, Functor, std::shared_ptr<Gl1_Fwall>> _classObj(_module, "Gl1_Fwall", "Renders :yref:`Fwall` object");
				_classObj.def(pybind11::init<>());
			}
			RENDERS(Fwall);
	};

	REGISTER_SERIALIZABLE_BASE(Gl1_Fwall, GlShapeFunctor);

#endif
