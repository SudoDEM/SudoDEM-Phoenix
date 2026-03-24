#!/usr/bin/env python3
"""Test __setitem__ and __getitem__ for Vector types"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../build/src/lib'))

from minieigen import Vector2, Vector3, VectorX
import time

def test_vector2_setitem():
    """Test Vector2 __setitem__ with index assignment"""
    print("Testing Vector2 __setitem__...")
    
    # Test basic setitem
    v = Vector2(0, 0)
    start = time.time()
    v[0] = 1.0
    elapsed = time.time() - start
    print(f"  Vector2[0] = 1.0 took {elapsed:.4f}s")
    assert v[0] == 1.0, f"Expected v[0] == 1.0, got {v[0]}"
    
    start = time.time()
    v[1] = 2.0
    elapsed = time.time() - start
    print(f"  Vector2[1] = 2.0 took {elapsed:.4f}s")
    assert v[1] == 2.0, f"Expected v[1] == 2.0, got {v[1]}"
    
    # Test float assignment
    v[0] = 3.5
    assert v[0] == 3.5, f"Expected v[0] == 3.5, got {v[0]}"
    
    # Test int assignment
    v[1] = 4
    assert v[1] == 4.0, f"Expected v[1] == 4.0, got {v[1]}"
    
    print("  Vector2 __setitem__ tests passed!")

def test_vector2_getitem():
    """Test Vector2 __getitem__"""
    print("Testing Vector2 __getitem__...")
    
    v = Vector2(1.0, 2.0)
    
    start = time.time()
    x = v[0]
    elapsed = time.time() - start
    print(f"  Vector2[0] access took {elapsed:.4f}s")
    assert x == 1.0, f"Expected v[0] == 1.0, got {x}"
    
    start = time.time()
    y = v[1]
    elapsed = time.time() - start
    print(f"  Vector2[1] access took {elapsed:.4f}s")
    assert y == 2.0, f"Expected v[1] == 2.0, got {y}"
    
    print("  Vector2 __getitem__ tests passed!")

def test_vector3_setitem():
    """Test Vector3 __setitem__"""
    print("Testing Vector3 __setitem__...")
    
    v = Vector3(0, 0, 0)
    start = time.time()
    v[0] = 1.0
    elapsed = time.time() - start
    print(f"  Vector3[0] = 1.0 took {elapsed:.4f}s")
    assert v[0] == 1.0
    
    start = time.time()
    v[1] = 2.0
    elapsed = time.time() - start
    print(f"  Vector3[1] = 2.0 took {elapsed:.4f}s")
    assert v[1] == 2.0
    
    start = time.time()
    v[2] = 3.0
    elapsed = time.time() - start
    print(f"  Vector3[2] = 3.0 took {elapsed:.4f}s")
    assert v[2] == 3.0
    
    print("  Vector3 __setitem__ tests passed!")

def test_vectorX_setitem():
    """Test VectorX __setitem__"""
    print("Testing VectorX __setitem__...")
    
    v = VectorX(5)  # Already initialized to zeros
    
    for i in range(5):
        start = time.time()
        v[i] = float(i + 1)
        elapsed = time.time() - start
        print(f"  VectorX[{i}] = {i+1} took {elapsed:.4f}s")
        assert v[i] == float(i + 1)
    
    print("  VectorX __setitem__ tests passed!")

def test_wall_scenario():
    """Test the exact scenario from wall() function"""
    print("Testing wall() scenario...")
    
    axis = 1
    position = -5.0
    
    start = time.time()
    pos2 = Vector2(0, 0)
    elapsed = time.time() - start
    print(f"  Vector2(0, 0) creation took {elapsed:.4f}s")
    
    start = time.time()
    pos2[axis] = position
    elapsed = time.time() - start
    print(f"  Vector2[{axis}] = {position} took {elapsed:.4f}s")
    
    assert pos2[0] == 0.0
    assert pos2[1] == -5.0
    
    print(f"  pos2 = {pos2}")
    print("  wall() scenario test passed!")

if __name__ == '__main__':
    print("=" * 60)
    print("Testing minieigen __setitem__ functionality")
    print("=" * 60)
    
    test_vector2_setitem()
    print()
    test_vector2_getitem()
    print()
    test_vector3_setitem()
    print()
    test_vectorX_setitem()
    print()
    test_wall_scenario()
    
    print()
    print("=" * 60)
    print("All __setitem__ tests passed!")
    print("=" * 60)