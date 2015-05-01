
def tupleToVec(t):
	if type(t) is int: return str(t).strip('L')
	return 'vector [%s]' % ' '.join(map(tupleToVec,t))

def vecToTuple(v):
	v = v[1::-1]
	if '[' in v:
		return tuple(map(vecToTuple,(x.strip().strip('[') for x in v.split(']') if x.strip())))
	return tuple(map(int, v.split()))

def send(ops):
	return '\n'.join(v if type(v) is str else tupleToVec(v) for v in ops)

def recv(output):
	out = map(str.strip, output.splitlines())[::-1]
	res = []
	while out:
		x = out.pop()
		if x[:2] == '[[':
			x = x[1:]
			m = []
			while x != ']':
				m.append(vecToTuple(x))
				x = out.pop()
			res.append(tuple(m))
		else:
			res.append(tupleToVec(x))
	return tuple(res)

def evaluate(operations):
	from subprocess import Popen, PIPE

	output, error = Popen(['vhe.exe'], stdin=PIPE, stdout=PIPE, stderr=PIPE, shell=True).communicate(send(operations))
        
	if error:
		from sys import stderr
		stderr.write(error + '\n')
		stderr.flush()

	return recv(output)





