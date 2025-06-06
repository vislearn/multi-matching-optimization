# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'pylibmgm'
copyright = '2025, Sebastian Stricker, Max Kahl'
author = 'Sebastian Stricker, Max Kahl'
release = '0.0.0.a6'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.mathjax',
    'sphinx_rtd_theme',
    'sphinx.ext.napoleon',
]
autosummary_generate = True

templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "sphinx_rtd_theme"
html_static_path = ['_static']

# -- Project specific configuration -----------------------------------------------
import os 
import sys
sys.path.insert(0, os.path.abspath("../../mgm_python"))
sys.path.insert(0, os.path.abspath("../../mgm_python/stubs"))