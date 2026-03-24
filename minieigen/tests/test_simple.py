"""Simple test to check what constructors are available"""
import sys
import os

# Add the path to minieigen module
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build', 'lib'))

try:
    import minieigen
except ImportError as e:
    print(f"Error importing minieigen: {e}")
    sys.exit(1)

print("minieigen imported successfully")
print(f"Vector2 class: {minieigen.Vector2}")
print(f"Vector3 class: {minieigen.Vector3}")

# Try to create with different arguments
try:
    v = minieigen.Vector2()
    print(f"Vector2() works: {v}")
except Exception as e:
    print(f"Vector2() failed: {e}")

try:
    v = minieigen.Vector2(1.0, 2.0)
    print(f"Vector2(1.0, 2.0) works: {v}")
except Exception as e:
    print(f"Vector2(1.0, 2.0) failed: {e}")

try:
    v = minieigen.Vector2([1.0, 2.0])
    print(f"Vector2([1.0, 2.0]) works: {v}")
except Exception as e:
    print(f"Vector2([1.0, 2.0]) failed: {e}")

try:
    v = minieigen.Vector2((1.0, 2.0))
    print(f"Vector2((1.0, 2.0)) works: {v}")
except Exception as e:
    print(f"Vector2((1.0, 2.0)) failed: {e}")

# Check the type of the argument
print(f"\nType of [1.0, 2.0]: {type([1.0, 2.0])}")
print(f"Type of (1.0, 2.0): {type((1.0, 2.0))}")