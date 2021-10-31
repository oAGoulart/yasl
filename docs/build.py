#!/usr/bin/env python
# Build the documentation.

import errno, os, re, sys
from subprocess import check_call, CalledProcessError, Popen, PIPE, STDOUT

class Pip:
  def __init__(self):
    self.path = os.path.join('_env', 'Scripts', 'pip')

  def install(self, package, commit=None):
    "Install package using pip."
    if commit:
      package = 'git+https://github.com/{0}.git@{1}'.format(package, commit)
    print('Installing {0}'.format(package))
    check_call([self.path, 'install', package])

def install_packages():
  pip = Pip()
  pip.install('sphinx')
  pip.install('breathe')
  pip.install('sphinx-bulma')

def build_docs():
  doc_dir = os.path.dirname(os.path.realpath(__file__))
  work_dir = '.'
  include_dir = os.path.join(work_dir, 'include')
  # Build doxygen xml.
  cmd = ['.\doxygen.exe', '-']
  p = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=STDOUT)
  doxyxml_dir = os.path.join(doc_dir, 'doxyxml')
  out, _ = p.communicate(input=r'''
      PROJECT_NAME      = YASL
      GENERATE_LATEX    = NO
      GENERATE_MAN      = NO
      GENERATE_RTF      = NO
      CASE_SENSE_NAMES  = NO
      INPUT             = {0}/base.h {0}/config.h {0}/memory.h {0}/script.h \
                          {0}/status.h {0}/yasl.h {0}/memory/data.h {0}/memory/patch.h \
                          {0}/memory/peformat.h {0}/memory/protection.h {0}/memory/trampoline.h
      QUIET             = YES
      JAVADOC_AUTOBRIEF = YES
      AUTOLINK_SUPPORT  = NO
      GENERATE_HTML     = NO
      GENERATE_XML      = YES
      XML_OUTPUT        = {1}
      ALIASES           = "rst=\verbatim embed:rst"
      ALIASES          += "endrst=\endverbatim"
      MACRO_EXPANSION   = YES
      PREDEFINED        = _WIN32=1 \
                          __cplusplus=1 \
                          _MSVC_LANG=202002L \
                          _DLL=1
    '''.format(include_dir, doxyxml_dir).encode('UTF-8'))
  out = out.decode('utf-8')
  print(out)
  if p.returncode != 0:
    raise CalledProcessError(p.returncode, cmd)
  
  # Build html docs.
  html_dir = os.path.join(work_dir, '_build')
  check_call([os.path.join(work_dir, '_env', 'Scripts', 'sphinx-build'),
              '-Dbreathe_projects.format=' + os.path.abspath(doxyxml_dir),
              '-b', 'html', doc_dir, html_dir])
  return html_dir

if __name__ == '__main__':
  install_packages()
  build_docs()
  