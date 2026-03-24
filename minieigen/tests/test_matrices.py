#!/usr/bin/env python3
"""
Unit tests for Matrix types in minieigen using pytest.
Tests: Matrix2, Matrix3, Matrix6, MatrixX
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


class TestMatrix2:
    """Test Matrix2 operations."""
    
    def test_identity(self):
        """Test Matrix2.Identity."""
        m = Matrix2.Identity
        assert m.rows() == 2
        assert m.cols() == 2
    
    def test_zero(self):
        """Test Matrix2.Zero."""
        m = Matrix2.Zero
        assert m.rows() == 2
        assert m.cols() == 2
    
    def test_addition(self):
        """Test Matrix2 addition."""
        m1 = Matrix2.Identity
        m2 = Matrix2.Identity
        m3 = m1 + m2
        assert m3.rows() == 2
        assert m3.cols() == 2
    
    def test_multiplication(self):
        """Test Matrix2 multiplication."""
        m1 = Matrix2.Identity
        m2 = Matrix2.Identity
        m3 = m1 * m2
        assert m3.rows() == 2
        assert m3.cols() == 2
    
    def test_dimensions(self):
        """Test Matrix2 dimensions."""
        m = Matrix2.Identity
        assert m.rows() == 2
        assert m.cols() == 2
    
    def test_sum(self):
        """Test Matrix2.sum()."""
        m = Matrix2.Identity
        s = m.sum()
        # Identity matrix sum = 2 (diagonal elements)
        assert s == 2.0


class TestMatrix3:
    """Test Matrix3 operations."""
    
    def test_identity(self):
        """Test Matrix3.Identity."""
        m = Matrix3.Identity
        assert m.rows() == 3
        assert m.cols() == 3
    
    def test_zero(self):
        """Test Matrix3.Zero."""
        m = Matrix3.Zero
        assert m.rows() == 3
        assert m.cols() == 3
    
    def test_from_quaternion(self):
        """Test Matrix3 from Quaternion."""
        q = Quaternion(1, 0, 0, 0)
        m = Matrix3(q)
        assert m.rows() == 3
        assert m.cols() == 3
    
    def test_addition(self):
        """Test Matrix3 addition."""
        m1 = Matrix3.Identity
        m2 = Matrix3.Identity
        m3 = m1 + m2
        assert m3.rows() == 3
        assert m3.cols() == 3
    
    def test_multiplication(self):
        """Test Matrix3 multiplication."""
        m1 = Matrix3.Identity
        m2 = Matrix3.Identity
        m3 = m1 * m2
        assert m3.rows() == 3
        assert m3.cols() == 3
    
    def test_determinant(self):
        """Test Matrix3.determinant()."""
        m = Matrix3.Identity
        det = m.determinant()
        # Identity matrix determinant = 1
        assert abs(det - 1.0) < 1e-10
    
    def test_dimensions(self):
        """Test Matrix3 dimensions."""
        m = Matrix3.Identity
        assert m.rows() == 3
        assert m.cols() == 3
    
    def test_sum(self):
        """Test Matrix3.sum()."""
        m = Matrix3.Identity
        s = m.sum()
        # Identity matrix sum = 3 (diagonal elements)
        assert s == 3.0
    
    def test_mean(self):
        """Test Matrix3.mean()."""
        m = Matrix3.Identity
        mn = m.mean()
        # Identity matrix mean = 3/9 = 1/3
        assert abs(mn - 1.0/3.0) < 1e-10


class TestMatrix6:
    """Test Matrix6 operations."""
    
    def test_identity(self):
        """Test Matrix6.Identity."""
        m = Matrix6.Identity
        assert m.rows() == 6
        assert m.cols() == 6
    
    def test_zero(self):
        """Test Matrix6.Zero."""
        m = Matrix6.Zero
        assert m.rows() == 6
        assert m.cols() == 6
    
    def test_addition(self):
        """Test Matrix6 addition."""
        m1 = Matrix6.Identity
        m2 = Matrix6.Identity
        m3 = m1 + m2
        assert m3.rows() == 6
        assert m3.cols() == 6
    
    def test_multiplication(self):
        """Test Matrix6 multiplication."""
        m1 = Matrix6.Identity
        m2 = Matrix6.Identity
        m3 = m1 * m2
        assert m3.rows() == 6
        assert m3.cols() == 6
    
    def test_dimensions(self):
        """Test Matrix6 dimensions."""
        m = Matrix6.Identity
        assert m.rows() == 6
        assert m.cols() == 6
    
    def test_sum(self):
        """Test Matrix6.sum()."""
        m = Matrix6.Identity
        s = m.sum()
        # Identity matrix sum = 6 (diagonal elements)
        assert s == 6.0


class TestMatrixX:
    """Test MatrixX (dynamic-sized) operations."""
    
    def test_identity(self):
        """Test MatrixX.Identity."""
        m = MatrixX.Identity(3, 3)
        assert m.rows() == 3
        assert m.cols() == 3
    
    def test_zero(self):
        """Test MatrixX.Zero."""
        m = MatrixX.Zero(2, 3)
        assert m.rows() == 2
        assert m.cols() == 3
    
    def test_addition(self):
        """Test MatrixX addition."""
        m1 = MatrixX.Identity(2, 2)
        m2 = MatrixX.Identity(2, 2)
        m3 = m1 + m2
        assert m3.rows() == 2
        assert m3.cols() == 2
    
    def test_multiplication(self):
        """Test MatrixX multiplication."""
        m1 = MatrixX.Identity(2, 2)
        m2 = MatrixX.Identity(2, 2)
        m3 = m1 * m2
        assert m3.rows() == 2
        assert m3.cols() == 2
    
    def test_sum(self):
        """Test MatrixX.sum()."""
        m = MatrixX.Identity(2, 2)
        s = m.sum()
        # Identity matrix sum = 2 (diagonal elements)
        assert s == 2.0
    
    def test_mean(self):
        """Test MatrixX.mean()."""
        m = MatrixX.Identity(2, 2)
        mn = m.mean()
        # Identity matrix mean = 2/4 = 0.5
        assert abs(mn - 0.5) < 1e-10


class TestMatrixScalarOperations:
    """Test matrix-scalar operations."""
    
    def test_scalar_multiplication(self):
        """Test matrix scalar multiplication."""
        m = Matrix3.Identity
        m2 = m * 2.0
        assert m2.rows() == 3
        assert m2.cols() == 3
    
    def test_scalar_division(self):
        """Test matrix scalar division."""
        m = Matrix3.Identity
        m2 = m / 2.0
        assert m2.rows() == 3
        assert m2.cols() == 3


class TestMatrixVectorOperations:
    """Test matrix-vector operations."""
    
    def test_matrix_vector_multiplication(self):
        """Test Matrix3 * Vector3."""
        m = Matrix3.Identity
        v = Vector3(1, 2, 3)
        result = m * v
        assert result[0] == 1
        assert result[1] == 2
        assert result[2] == 3


class TestMatrixMatrixOperations:
    """Test matrix-matrix operations."""
    
    def test_matrix_matrix_multiplication(self):
        """Test Matrix3 * Matrix3."""
        m1 = Matrix3.Identity
        m2 = Matrix3.Identity
        result = m1 * m2
        assert result.rows() == 3
        assert result.cols() == 3