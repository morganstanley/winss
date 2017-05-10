import datetime
import os

import sphinx_rtd_theme


templates_path = ['_templates']

source_suffix = '.rst'
master_doc = 'index'

project = u'winss'
copyright = '{}, Morgan Stanley'.format(datetime.date.today().year)
author = u'Morgan Stanley'

version = os.getenv('APPVEYOR_BUILD_VERSION', 'dev')
release = version

language = 'en'

exclude_patterns = ['_build', 'include']
pygments_style = 'sphinx'

html_theme = 'sphinx_rtd_theme'
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]

html_show_sourcelink = True
html_show_sphinx = True

html_static_path = ['_static']

htmlhelp_basename = 'winss-doc'
