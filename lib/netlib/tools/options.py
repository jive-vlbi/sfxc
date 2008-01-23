import SCons

def generate(env, **kw):
	env.Option = SCons.Options.Options('options.conf')
		
def exists(env):
	return 1;
