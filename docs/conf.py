# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import sys
import os
from datetime import datetime

sys.path.append(os.path.abspath('..'))
sys.path.append(os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'YASL'
year = datetime.now().year
copyright = u'%d Augusto Goulart' % year
author = 'Augusto Goulart'

# The full version, including alpha/beta/rc tags
release = '0.8.0'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ['breathe']

breathe_projects = {
  'YASL': 'doxyxml',
}
breathe_default_project = 'YASL'

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build']

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#

html_favicon = 'static/favicon.ico'
html_logo = 'static/logo.png'
html_theme = 'sphinx-bulma'
html_theme_options = {
  'display_git': True,
  'git_host': 'github.com',
  'git_user': 'oAGoulart',
  'git_repo': 'yasl',
  'git_version': 'master/docs/',
  'git_icon': 'github-circled',
  'git_desc': 'Edit on GitHub',
  'primary': '6ca0ff'
}

html_theme_path = ["../_env/Lib/site-packages"]

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['static']

pygments_style = 'sphinx'
highlight_language = 'c++'
primary_domain = 'cpp'
