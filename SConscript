from building import *

src   = ['tcpserver']
cwd   = GetCurrentDir()
include_path = [cwd]

group = DefineGroup('tcpserver', src, depend = ['PKG_USING_TCPSERVER'], CPPPATH = include_path)

Return('group')
