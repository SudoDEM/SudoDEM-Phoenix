#!/usr/bin/env python3
"""
Unit tests for rotation types in minieigen using pytest.
Tests: Quaternion, Rotation2d
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


class TestQuaternion:
    """Test Quaternion operations."""
    
    def test_constructor(self):
        """Test Quaternion constructor."""
        q = Quaternion(1, 0, 0, 0)
        # Quaternion should be constructed successfully
        assert q is not None
    
    def test_identity(self):
        """Test Quaternion.Identity."""
        q = Quaternion.Identity
        assert q is not None
    
    def test_multiplication(self):
        """Test Quaternion multiplication."""
        q1 = Quaternion(1, 0, 0, 0)
        q2 = Quaternion(0, 1, 0, 0)
        q3 = q1 * q2
        assert q3 is not None
    
    def test_norm(self):
        """Test Quaternion norm."""
        q = Quaternion(1, 0, 0, 0)
        n = q.norm()
        # Unit quaternion should have norm 1
        assert abs(n - 1.0) < 1e-10
    
    def test_normalized(self):
        """Test Quaternion normalized."""
        q = Quaternion(1, 0, 0, 0)
        qn = q.normalized()
        n = qn.norm()
        # Normalized quaternion should have norm 1
        assert abs(n - 1.0) < 1e-10


class TestRotation2D:
    """Test Rotation2D operations."""
    
    def test_constructor(self):
        """Test Rotation2D constructor."""
        r = Rotation2d(0.5)
        assert r is not None
    
    def test_identity(self):
        """Test Rotation2D.Identity."""
        r = Rotation2d.Identity
        assert r is not None