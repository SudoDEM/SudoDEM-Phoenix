#!/usr/bin/env python3
"""
Unit tests for complex types in minieigen using pytest.
Tests: Vector3c, Matrix3c, etc.
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


class TestVector3c:
    """Test Vector3c (complex vector) operations."""
    
    def test_constructor(self):
        """Test Vector3c constructor with complex numbers."""
        try:
            v = Vector3c(1+2j, 3+4j, 5+6j)
            assert v is not None
        except NameError:
            pytest.skip("Vector3c not available (complex support disabled)")
    
    def test_length(self):
        """Test Vector3c length."""
        try:
            v = Vector3c(1+2j, 3+4j, 5+6j)
            assert len(v) == 3
        except NameError:
            pytest.skip("Vector3c not available (complex support disabled)")


class TestMatrix3c:
    """Test Matrix3c (complex matrix) operations."""
    
    def test_identity(self):
        """Test Matrix3c.Identity."""
        try:
            m = Matrix3c.Identity
            assert m is not None
        except NameError:
            pytest.skip("Matrix3c not available (complex support disabled)")
    
    def test_dimensions(self):
        """Test Matrix3c dimensions."""
        try:
            m = Matrix3c.Identity
            assert m.rows() == 3
            assert m.cols() == 3
        except NameError:
            pytest.skip("Matrix3c not available (complex support disabled)")


class TestVector2c:
    """Test Vector2c (complex vector) operations."""
    
    def test_constructor(self):
        """Test Vector2c constructor with complex numbers."""
        try:
            v = Vector2c(1+2j, 3+4j)
            assert v is not None
        except NameError:
            pytest.skip("Vector2c not available (complex support disabled)")
    
    def test_length(self):
        """Test Vector2c length."""
        try:
            v = Vector2c(1+2j, 3+4j)
            assert len(v) == 2
        except NameError:
            pytest.skip("Vector2c not available (complex support disabled)")


class TestMatrix3cr:
    """Test Matrix3cr (complex matrix) operations - alias."""
    
    def test_identity(self):
        """Test Matrix3cr.Identity (should be same as Matrix3c)."""
        try:
            m = Matrix3cr.Identity
            assert m is not None
        except NameError:
            pytest.skip("Matrix3cr not available (complex support disabled)")


class TestVector3cr:
    """Test Vector3cr (complex vector) operations - alias."""
    
    def test_constructor(self):
        """Test Vector3cr constructor with complex numbers."""
        try:
            v = Vector3cr(1+2j, 3+4j, 5+6j)
            assert v is not None
        except NameError:
            pytest.skip("Vector3cr not available (complex support disabled)")