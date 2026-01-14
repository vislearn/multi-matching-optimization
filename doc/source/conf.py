# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------
import os
import sys

# Add project source to path
sys.path.insert(0, os.path.abspath("../../mgm_python"))
sys.path.insert(0, os.path.abspath("../../mgm_python/stubs"))

# Add extension directory to path
sys.path.insert(0, os.path.abspath("_ext"))

# -- Project information -----------------------------------------------------
project = 'pylibmgm'
copyright = '2025, Sebastian Stricker, Max Kahl'
author = 'Sebastian Stricker, Max Kahl'
release = '1.1.1'

# -- General configuration ---------------------------------------------------
extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.mathjax',
    'sphinx.ext.napoleon',
    'sphinx_rtd_theme',
    'stub_autodoc',  # Custom extension for handling project's documentation from stub files.
]

autosummary_generate = True

# Allow autosummary to regenerate files
autosummary_generate_overwrite = True

# Suppress warnings for orphaned documents (auto-generated nested class docs)
# and autodoc import errors for nested classes
suppress_warnings = ['toc.not_included', 'autodoc.import_object']

# Autodoc settings
autodoc_default_options = {
    'members': True,
    'member-order': 'bysource',
    'special-members': '__init__',
    'undoc-members': True,
    'exclude-members': '__weakref__'
}

autodoc_typehints = 'signature'
autodoc_typehints_format = 'short'

# Napoleon settings - defaults provide compact field list formatting
napoleon_google_docstring = False
napoleon_numpy_docstring = True

# -- Custom autodoc processing -----------------------------------------------
def process_signature(app, what, name, obj, options, signature, return_annotation):
    """Process signatures to enhance documentation display."""
    import enum
    import re
    
    # For Enum classes, remove the (*values) signature
    if isinstance(obj, enum.EnumMeta):
        return '', return_annotation
    
    # For methods and functions, remove 'self' parameter from signature
    if signature and what in ('method', 'function'):
        # Remove self parameter and its type annotation
        # Pattern matches: (self: Type) or (self: Type, ...) or (self, ...)
        signature = re.sub(r'\(self:\s*[^,)]+,?\s*', '(', signature)
        signature = re.sub(r'\(self,\s*', '(', signature)
        signature = re.sub(r'\(self\)', '()', signature)
    
    return signature, return_annotation

def setup(app):
    """Sphinx setup hook."""
    app.connect('autodoc-process-signature', process_signature)

# -- HTML output options -----------------------------------------------------
html_theme = "sphinx_rtd_theme"
html_static_path = ['_static']

templates_path = ['_templates']
exclude_patterns = []