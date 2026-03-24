# encoding: utf-8
"""
Qt6 GUI module for SudoDEM.

This module provides access to the Qt6-based GUI components.
The GUI is implemented in C++ and exposed to Python via pybind11.
"""

import sys
import os
import importlib.util
import importlib.machinery

# Check if we're in batch mode
if 'SUDODEM_BATCH' in os.environ:
    raise ImportError("Do not import qt when running in batch mode (SUDODEM_BATCH is set).")

# Find and load the _GLViewer dynamic library
_qt_dir = os.path.dirname(os.path.abspath(__file__))

# Try different possible names for the dynamic library
_possible_names = ['_GLViewer.dylib', '_GLViewer.so', '_GLViewer.pyd']
_glv_path = None

for name in _possible_names:
    path = os.path.join(_qt_dir, name)
    if os.path.exists(path):
        _glv_path = path
        break

if _glv_path:
    # Load the extension module using ExtensionFileLoader
    # Note: _GLViewer may fail to initialize if OpenGLManager already exists
    # (which happens when GUI is started from C++), so we catch that error
    try:
        _loader = importlib.machinery.ExtensionFileLoader("sudodem.qt._GLViewer", _glv_path)
        _spec = importlib.util.spec_from_loader("sudodem.qt._GLViewer", _loader)
        _GLViewer = importlib.util.module_from_spec(_spec)
        sys.modules['sudodem.qt._GLViewer'] = _GLViewer
        _loader.exec_module(_GLViewer)
    except ImportError as e:
        if "OpenGLManager instance already exists" in str(e):
            # OpenGLManager was already created by C++ GUI
            # The module might still be partially loaded, try to get it from sys.modules
            if 'sudodem.qt._GLViewer' in sys.modules:
                _GLViewer = sys.modules['sudodem.qt._GLViewer']
            else:
                raise ImportError("GUI module partially loaded. Please ensure GUI is not already running.")
        else:
            raise
    
    # Export symbols
    View = _GLViewer.View
    views = _GLViewer.views
    center = _GLViewer.center
    Renderer = _GLViewer.Renderer
    GLViewer = _GLViewer.GLViewer
else:
    raise ImportError(f"_GLViewer dynamic library not found in {_qt_dir}")

# Controller is created by C++ at startup, expose it to Python
# Users can access it via sudodem.qt.Controller()
_controller = None

def Controller():
    """
    Show/raise the Controller dialog.
    
    In Qt6 mode, the Controller is created automatically at startup.
    This function brings it to the front.
    """
    global _controller
    
    # Try to access the Controller through the OpenGLManager
    # The Controller instance is created and managed by C++ in sudodem.cpp
    # We need to ensure it's visible by calling show(), raise(), and activateWindow()
    try:
        if _GLViewer is not None and hasattr(_GLViewer, 'showController'):
            _GLViewer.showController()
            _controller = True  # Mark as available
        else:
            print("Controller is managed by C++ Qt6 GUI.")
            print("If the window is not visible, it may be minimized or behind other windows.")
            print("Look for 'SudoDEM Controller' in your taskbar or Dock.")
    except Exception as e:
        print(f"Note: {e}")
        print("Controller is running in C++ layer. Check your taskbar/Dock for 'SudoDEM Controller'")
    
    return _controller

def Inspector():
    """Show the Inspector dialog (not yet implemented in Qt6 mode)."""
    print("Inspector not yet implemented in Qt6 mode")
    return None

def Generator():
    """Show the Generator tab in Controller (not yet implemented in Qt6 mode)."""
    print("Generator not yet implemented in Qt6 mode")
    return None

# Documentation paths
sphinxOnlineDocPath = 'https://www.sudodem.org/doc/'
sphinxLocalDocPath = os.environ.get('SUDODEM_PREFIX', '') + '/share/doc/sudodem-doc/html/'

if os.path.exists(sphinxLocalDocPath + '/index.html'):
    sphinxPrefix = 'file://' + sphinxLocalDocPath
else:
    sphinxPrefix = sphinxOnlineDocPath

sphinxDocWrapperPage = sphinxPrefix + '/sudodem.wrapper.html'


def openUrl(url):
    """Open a URL in the default browser."""
    import webbrowser
    webbrowser.open(url)


def save(filename, quiet=False):
    """Save current simulation to file.
    This is a convenience wrapper around O.save().
    """
    import sudodem
    sudodem.O.save(filename, quiet)


def load(filename, quiet=False):
    """Load simulation from file.
    This is a convenience wrapper around O.load().
    """
    import sudodem
    sudodem.O.load(filename, quiet)


# Export all public symbols
__all__ = [
    'View',
    'views',
    'center',
    'Renderer',
    'GLViewer',
    'Controller',
    'Inspector',
    'Generator',
    'openUrl',
    'save',
    'load',
]