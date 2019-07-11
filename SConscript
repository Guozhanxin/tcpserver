from building import *

src     = ['tcpserver.c']
cwd   = GetCurrentDir()
include_path = [cwd]

if GetDepend(['PKG_TCPSERVER_SAMPLE']):
    src += ['tcpserver_sample.c']

group = DefineGroup('tcpserver', src, depend = ['PKG_USING_TCPSERVER'], CPPPATH = include_path)

Return('group')
