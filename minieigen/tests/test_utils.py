#!/usr/bin/env python3
"""
Unit tests for utility functions and other types in minieigen using pytest.
Tests: AlignedBox3, float2str, vectorize attribute
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


class TestAlignedBox3:
    """Test AlignedBox3 operations."""
    
    def test_default_constructor(self):
        """Test AlignedBox3 default constructor."""
        box = AlignedBox3()
        assert box is not None
    
    def test_constructor_with_min_max(self):
        """Test AlignedBox3 constructor with min and max vectors."""
        box = AlignedBox3(Vector3(0, 0, 0), Vector3(1, 1, 1))
        assert box is not None
    
    def test_min_max(self):
        """Test AlignedBox3 min and max properties."""
        box = AlignedBox3(Vector3(0, 0, 0), Vector3(1, 1, 1))
        # Box should have min and max properties
        assert hasattr(box, 'min')
        assert hasattr(box, 'max')


class TestUtilityFunctions:
    """Test utility functions."""
    
    def test_float2str(self):
        """Test float2str function."""
        s = float2str(3.14159)
        assert isinstance(s, str)
        assert len(s) > 0
    
    def test_float2str_with_padding(self):
        """Test float2str function with padding."""
        s = float2str(3.14159, 10)
        assert isinstance(s, str)
        # With padding, string should be longer
        assert len(s) >= 10


class TestVectorizeAttribute:
    """Test vectorize attribute."""
    
    def test_vectorize_exists(self):
        """Test that vectorize attribute exists."""
        try:
            v = vectorize
            # vectorize should be a boolean value
            assert isinstance(v, bool)
        except NameError:
            pytest.skip("vectorize attribute not available")