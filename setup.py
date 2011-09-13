try:
    from setuptools import setup, find_packages, Extension
except ImportError:
    from ez_setup import use_setuptools
    use_setuptools()
    from setuptools import setup, find_packages, Extension

from distutils.command.install import INSTALL_SCHEMES
for scheme in INSTALL_SCHEMES.values():
    scheme['data'] = scheme['purelib']

setup(
    name='pfp',
    version='0.0.2',
    description='pretty fast statistical parser for natural languages',
    author='Wavii',
    author_email='info@wavii.com',
    url='https://github.com/wavii/pfp',
    packages=find_packages(exclude=['ez_setup', 'tests*', 'integration*']),
    include_package_data=True,
    zip_safe=False,
    
    ext_modules=[
        Extension('pfp',
                  ['src/pfp/config.cpp',
                   'src/pfp/tokenizer.yy.cpp',
                   'src/pypfp/pypfp.cpp'],
                  include_dirs=['include'],
                  libraries=['boost_python', 'boost_filesystem', 'boost_thread', 'boost_regex', 'boost_system', 'icuio'],
                  extra_compile_args=['-g']
                  ),
        ],
    data_files=[('share', ['share/pfp/americanizations', 'share/pfp/binary_rules', 'share/pfp/sigs', 'share/pfp/sig_state',
                           'share/pfp/states', 'share/pfp/unary_rules', 'share/pfp/words', 'share/pfp/word_state'])]
)
