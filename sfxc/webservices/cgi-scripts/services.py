import sys
sys.path = ([sys.path[0]] + ['/home/kruithof/share/lib64/python2.4/site-packages/simplejson-1.7.3-py2.4-linux-x86_64.egg',
                             '/home/small/code/share/lib/python2.4/site-packages/antlr_python_runtime-3.0.1-py2.4.egg',
                             '/home/small/lib64/python2.4/site-packages', 
                             '/home/small/code/share/lib/python2.4/site-packages',
                             '/home/small/code/webservices/vex2ccf_web_service',
                             '/home/small/code/webservices/translationnode'] +
            sys.path[1:])

from ZSI import dispatch

from vex2ccf import convertRequest
from translation_node_cgi import forkTranslationJob as startTranslationJob
from correlated_data_cgi import forkDataSetIsReady as dataSetIsReady


if __name__=="__main__":
    dispatch.AsCGI(rpc=False)
