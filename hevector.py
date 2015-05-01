
def tupleToVec(t):
	if type(t) is int: return str(t).strip('L')
	return '[%s]' % ' '.join(map(tupleToVec,t))

def vecToTuple(v):
	return tuple(map(int, v.strip('[]').split()))

def send(ops):
	return '\n'.join(v if type(v) is str else tupleToVec(v) for v in ops)

def recv(output):
	return tuple(vecToTuple(l) for l in map(str.strip, output.splitlines()))

def evaluate(operations):
	import subprocess
	output, error = subprocess.Popen(['vhe'], shell=True).communicate(send(operations))

	if error:
		from sys import stderr
		stderr.write(error + '\n')
		stderr.flush()

	return recv(output)





