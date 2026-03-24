#!/usr/bin/env python3
"""
Unit tests for Vector types in minieigen using pytest.
Tests: Vector2, Vector3, Vector6, VectorX, Vector2i, Vector3i, Vector6i
"""

import sys
import os

# Add parent directory to path to import minieigen
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import pytest

try:
    from minieigen import *
except ImportError as e:
    pytest.skip(f"Failed to import minieigen: {e}", allow_module_level=True)


class TestVector3:
    """Test Vector3 operations."""
    
    def test_default_constructor(self):
        """Test default Vector3 constructor."""
        v = Vector3()
        assert len(v) == 3
    
    def test_constructor_with_args(self):
        """Test Vector3 constructor with arguments."""
        v = Vector3(1, 2, 3)
        assert v[0] == 1
        assert v[1] == 2
        assert v[2] == 3
    
    def test_addition(self):
        """Test Vector3 addition."""
        v1 = Vector3(1, 2, 3)
        v2 = Vector3(4, 5, 6)
        v3 = v1 + v2
        assert v3[0] == 5
        assert v3[1] == 7
        assert v3[2] == 9
    
    def test_subtraction(self):
        """Test Vector3 subtraction."""
        v1 = Vector3(4, 5, 6)
        v2 = Vector3(1, 2, 3)
        v3 = v1 - v2
        assert v3[0] == 3
        assert v3[1] == 3
        assert v3[2] == 3
    
    def test_scalar_multiplication(self):
        """Test Vector3 scalar multiplication."""
        v = Vector3(1, 2, 3)
        v2 = v * 2.0
        assert v2[0] == 2.0
        assert v2[1] == 4.0
        assert v2[2] == 6.0
    
    def test_scalar_division(self):
        """Test Vector3 scalar division."""
        v = Vector3(2, 4, 6)
        v2 = v / 2.0
        assert v2[0] == 1.0
        assert v2[1] == 2.0
        assert v2[2] == 3.0
    
    def test_negation(self):
        """Test Vector3 negation."""
        v = Vector3(1, 2, 3)
        v2 = -v
        assert v2[0] == -1
        assert v2[1] == -2
        assert v2[2] == -3
    
    def test_dot_product(self):
        """Test Vector3 dot product."""
        v1 = Vector3(1, 2, 3)
        v2 = Vector3(4, 5, 6)
        dp = v1.dot(v2)
        assert dp == 32  # 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    
    def test_cross_product(self):
        """Test Vector3 cross product."""
        v1 = Vector3(1, 0, 0)
        v2 = Vector3(0, 1, 0)
        cp = v1.cross(v2)
        assert cp[0] == 0
        assert cp[1] == 0
        assert cp[2] == 1
    
    def test_norm(self):
        """Test Vector3 norm."""
        v = Vector3(3, 4, 0)
        n = v.norm()
        assert abs(n - 5.0) < 1e-10
    
    def test_normalized(self):
        """Test Vector3 normalized."""
        v = Vector3(3, 4, 0)
        vn = v.normalized()
        n = vn.norm()
        assert abs(n - 1.0) < 1e-10
    
    def test_static_zero(self):
        """Test Vector3.Zero static method."""
        v = Vector3.Zero
        assert v[0] == 0
        assert v[1] == 0
        assert v[2] == 0
    
    def test_static_ones(self):
        """Test Vector3.Ones static method."""
        v = Vector3.Ones
        assert v[0] == 1
        assert v[1] == 1
        assert v[2] == 1
    
    def test_static_unitx(self):
        """Test Vector3.UnitX static method."""
        v = Vector3.UnitX
        assert v[0] == 1
        assert v[1] == 0
        assert v[2] == 0
    
    def test_static_unity(self):
        """Test Vector3.UnitY static method."""
        v = Vector3.UnitY
        assert v[0] == 0
        assert v[1] == 1
        assert v[2] == 0
    
    def test_static_unitz(self):
        """Test Vector3.UnitZ static method."""
        v = Vector3.UnitZ
        assert v[0] == 0
        assert v[1] == 0
        assert v[2] == 1
    
    def test_indexing(self):
        """Test Vector3 indexing."""
        v = Vector3(1, 2, 3)
        assert v[0] == 1
        assert v[1] == 2
        assert v[2] == 3
    
    def test_setitem(self):
        """Test Vector3 setitem."""
        v = Vector3(1, 2, 3)
        v[0] = 10
        assert v[0] == 10
        assert v[1] == 2
        assert v[2] == 3
    
    def test_equality(self):
        """Test Vector3 equality."""
        v1 = Vector3(1, 2, 3)
        v2 = Vector3(1, 2, 3)
        v3 = Vector3(4, 5, 6)
        assert v1 == v2
        assert v1 != v3
    
    def test_length(self):
        """Test Vector3 length."""
        v = Vector3(1, 2, 3)
        assert len(v) == 3
    
    def test_in_place_addition(self):
        """Test Vector3 in-place addition."""
        v = Vector3(1, 2, 3)
        v2 = Vector3(4, 5, 6)
        v += v2
        assert v[0] == 5
        assert v[1] == 7
        assert v[2] == 9
    
    def test_in_place_subtraction(self):
        """Test Vector3 in-place subtraction."""
        v = Vector3(4, 5, 6)
        v -= Vector3(1, 1, 1)
        assert v[0] == 3
        assert v[1] == 4
        assert v[2] == 5
    
    def test_in_place_scalar_multiplication(self):
        """Test Vector3 in-place scalar multiplication."""
        v = Vector3(1, 2, 3)
        v *= 2.0
        assert v[0] == 2.0
        assert v[1] == 4.0
        assert v[2] == 6.0


class TestVector2:
    """Test Vector2 operations."""
    
    def test_constructor_with_args(self):
        """Test Vector2 constructor with arguments."""
        v = Vector2(1.0, 2.0)
        assert v[0] == 1.0
        assert v[1] == 2.0
    
    def test_addition(self):
        """Test Vector2 addition."""
        v1 = Vector2(1, 2)
        v2 = Vector2(3, 4)
        v3 = v1 + v2
        assert v3[0] == 4
        assert v3[1] == 6
    
    def test_scalar_multiplication(self):
        """Test Vector2 scalar multiplication."""
        v = Vector2(1, 2)
        v2 = v * 2.0
        assert v2[0] == 2.0
        assert v2[1] == 4.0
    
    def test_static_zero(self):
        """Test Vector2.Zero static method."""
        v = Vector2.Zero
        assert v[0] == 0
        assert v[1] == 0
    
    def test_static_ones(self):
        """Test Vector2.Ones static method."""
        v = Vector2.Ones
        assert v[0] == 1
        assert v[1] == 1
    
    def test_static_unitx(self):
        """Test Vector2.UnitX static method."""
        v = Vector2.UnitX
        assert v[0] == 1
        assert v[1] == 0
    
    def test_static_unity(self):
        """Test Vector2.UnitY static method."""
        v = Vector2.UnitY
        assert v[0] == 0
        assert v[1] == 1
    
    def test_length(self):
        """Test Vector2 length."""
        v = Vector2(1, 2)
        assert len(v) == 2


class TestVector6:
    """Test Vector6 operations."""
    
    def test_constructor_with_args(self):
        """Test Vector6 constructor with arguments."""
        v = Vector6(1, 2, 3, 4, 5, 6)
        for i in range(6):
            assert v[i] == i + 1
    
    def test_addition(self):
        """Test Vector6 addition."""
        v1 = Vector6(1, 2, 3, 4, 5, 6)
        v2 = Vector6(6, 5, 4, 3, 2, 1)
        v3 = v1 + v2
        assert v3[0] == 7
        assert v3[5] == 7
    
    def test_static_zero(self):
        """Test Vector6.Zero static method."""
        v = Vector6.Zero
        for i in range(6):
            assert v[i] == 0
    
    def test_static_ones(self):
        """Test Vector6.Ones static method."""
        v = Vector6.Ones
        for i in range(6):
            assert v[i] == 1
    
    def test_length(self):
        """Test Vector6 length."""
        v = Vector6(1, 2, 3, 4, 5, 6)
        assert len(v) == 6


class TestVector6i:
    """Test Vector6i (integer) operations."""
    
    def test_constructor_with_args(self):
        """Test Vector6i constructor with arguments."""
        v = Vector6i(1, 2, 3, 4, 5, 6)
        for i in range(6):
            assert v[i] == i + 1
    
    def test_addition(self):
        """Test Vector6i addition."""
        v1 = Vector6i(1, 2, 3, 4, 5, 6)
        v2 = Vector6i(6, 5, 4, 3, 2, 1)
        v3 = v1 + v2
        assert v3[0] == 7
        assert v3[5] == 7
    
    def test_static_zero(self):
        """Test Vector6i.Zero static method."""
        v = Vector6i.Zero
        for i in range(6):
            assert v[i] == 0
    
    def test_static_ones(self):
        """Test Vector6i.Ones static method."""
        v = Vector6i.Ones
        for i in range(6):
            assert v[i] == 1


class TestVector3i:
    """Test Vector3i (integer) operations."""
    
    def test_constructor_with_args(self):
        """Test Vector3i constructor with arguments."""
        v = Vector3i(1, 2, 3)
        assert v[0] == 1
        assert v[1] == 2
        assert v[2] == 3
    
    def test_addition(self):
        """Test Vector3i addition."""
        v1 = Vector3i(1, 2, 3)
        v2 = Vector3i(4, 5, 6)
        v3 = v1 + v2
        assert v3[0] == 5
        assert v3[1] == 7
        assert v3[2] == 9
    
    def test_static_zero(self):
        """Test Vector3i.Zero static method."""
        v = Vector3i.Zero
        assert v[0] == 0
        assert v[1] == 0
        assert v[2] == 0


class TestVector2i:
    """Test Vector2i (integer) operations."""
    
    def test_constructor_with_args(self):
        """Test Vector2i constructor with arguments."""
        v = Vector2i(1, 2)
        assert v[0] == 1
        assert v[1] == 2
    
    def test_addition(self):
        """Test Vector2i addition."""
        v1 = Vector2i(1, 2)
        v2 = Vector2i(3, 4)
        v3 = v1 + v2
        assert v3[0] == 4
        assert v3[1] == 6


class TestVectorX:
    """Test VectorX (dynamic-sized) operations."""
    
    def test_constructor_with_size(self):
        """Test VectorX constructor with size."""
        v = VectorX(5)
        assert len(v) == 5
    
    def test_constructor_with_list(self):
        """Test VectorX constructor with list."""
        v = VectorX([1, 2, 3, 4, 5])
        for i in range(5):
            assert v[i] == i + 1
    
    def test_addition(self):
        """Test VectorX addition."""
        v1 = VectorX([1, 2, 3])
        v2 = VectorX([4, 5, 6])
        v3 = v1 + v2
        assert v3[0] == 5
        assert v3[1] == 7
        assert v3[2] == 9
    
    def test_static_zero(self):
        """Test VectorX.Zero static method."""
        v = VectorX.Zero(5)
        assert len(v) == 5
        for i in range(5):
            assert v[i] == 0
    
    def test_static_ones(self):
        """Test VectorX.Ones static method."""
        v = VectorX.Ones(5)
        assert len(v) == 5
        for i in range(5):
            assert v[i] == 1
    
    def test_static_random(self):
        """Test VectorX.Random static method."""
        v = VectorX.Random(5)
        assert len(v) == 5
    
    def test_static_unit(self):
        """Test VectorX.Unit static method."""
        v = VectorX.Unit(5, 2)
        assert len(v) == 5
        assert v[2] == 1
        for i in range(5):
            if i != 2:
                assert v[i] == 0
    
    def test_resize(self):
        """Test VectorX resize."""
        v = VectorX(3)
        v.resize(5)
        assert len(v) == 5