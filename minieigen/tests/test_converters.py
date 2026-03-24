"""
Unit tests for minieigen type converters
Tests conversion from Python list/tuple to Eigen vectors
"""

import sys
import os

# Add the path to minieigen module
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build', 'src', 'lib'))

try:
    import minieigen
except ImportError:
    print("Error: minieigen module not found. Please build minieigen first.")
    sys.exit(1)


def test_vector2_from_list():
    """Test Vector2 conversion from Python list using constructor"""
    # Test creating Vector2 from list
    v = minieigen.Vector2([1.0, 2.0])
    assert v[0] == 1.0, f"Expected 1.0, got {v[0]}"
    assert v[1] == 2.0, f"Expected 2.0, got {v[1]}"
    print("✓ Vector2 from list test passed")


def test_vector2_from_tuple():
    """Test Vector2 conversion from Python tuple using constructor"""
    v = minieigen.Vector2((3.0, 4.0))
    assert v[0] == 3.0, f"Expected 3.0, got {v[0]}"
    assert v[1] == 4.0, f"Expected 4.0, got {v[1]}"
    print("✓ Vector2 from tuple test passed")


def test_vector3_from_list():
    """Test Vector3 conversion from Python list using constructor"""
    v = minieigen.Vector3([1.0, 2.0, 3.0])
    assert v[0] == 1.0, f"Expected 1.0, got {v[0]}"
    assert v[1] == 2.0, f"Expected 2.0, got {v[1]}"
    assert v[2] == 3.0, f"Expected 3.0, got {v[2]}"
    print("✓ Vector3 from list test passed")


def test_vector3_from_tuple():
    """Test Vector3 conversion from Python tuple using constructor"""
    v = minieigen.Vector3((4.0, 5.0, 6.0))
    assert v[0] == 4.0, f"Expected 4.0, got {v[0]}"
    assert v[1] == 5.0, f"Expected 5.0, got {v[1]}"
    assert v[2] == 6.0, f"Expected 6.0, got {v[2]}"
    print("✓ Vector3 from tuple test passed")


def test_vector2_operations():
    """Test Vector2 operations after conversion"""
    v1 = minieigen.Vector2([1.0, 2.0])
    v2 = minieigen.Vector2([3.0, 4.0])
    
    # Test addition
    v3 = v1 + v2
    assert v3[0] == 4.0, f"Expected 4.0, got {v3[0]}"
    assert v3[1] == 6.0, f"Expected 6.0, got {v3[1]}"
    
    # Test dot product
    dot = v1.dot(v2)
    expected = 1.0*3.0 + 2.0*4.0  # 11.0
    assert dot == expected, f"Expected {expected}, got {dot}"
    
    print("✓ Vector2 operations test passed")


def test_vector3_operations():
    """Test Vector3 operations after conversion"""
    v1 = minieigen.Vector3([1.0, 0.0, 0.0])
    v2 = minieigen.Vector3([0.0, 1.0, 0.0])
    
    # Test cross product
    v3 = v1.cross(v2)
    assert abs(v3[0]) < 1e-10, f"Expected ~0, got {v3[0]}"
    assert abs(v3[1]) < 1e-10, f"Expected ~0, got {v3[1]}"
    assert abs(v3[2] - 1.0) < 1e-10, f"Expected ~1.0, got {v3[2]}"
    
    print("✓ Vector3 operations test passed")


def test_invalid_conversion():
    """Test that invalid conversions raise appropriate errors"""
    try:
        # Wrong size list for Vector2
        v = minieigen.Vector2([1.0, 2.0, 3.0])
        print("✗ Should have raised error for wrong size list")
        assert False, "Should have raised error for wrong size list"
    except (TypeError, ValueError, RuntimeError) as e:
        print(f"✓ Invalid conversion correctly raises error: {e}")


def run_all_tests():
    """Run all tests"""
    print("Running minieigen converter tests...")
    print("=" * 50)
    
    tests = [
        test_vector2_from_list,
        test_vector2_from_tuple,
        test_vector3_from_list,
        test_vector3_from_tuple,
        test_vector2_operations,
        test_vector3_operations,
        test_invalid_conversion,
    ]
    
    passed = 0
    failed = 0
    
    for test in tests:
        try:
            result = test()
            if result is not False:
                passed += 1
            else:
                failed += 1
        except AssertionError as e:
            print(f"✗ {test.__name__} failed: {e}")
            failed += 1
        except Exception as e:
            print(f"✗ {test.__name__} failed with exception: {e}")
            failed += 1
    
    print("=" * 50)
    print(f"Results: {passed} passed, {failed} failed")
    
    return failed == 0


if __name__ == "__main__":
    success = run_all_tests()
    sys.exit(0 if success else 1)