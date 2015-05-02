
def tupleToVec(t):
    if type(t) is int: return str(t).strip('L')
    if t and type(t[0]) is tuple:
        return 'matrix [%s]' % ' '.join('[%s]' % ' '.join(map(str, x)) for x in t)
    return 'vector [%s]' % ' '.join(map(tupleToVec,t))

def vecToTuple(v):
    v = v[1:-1]
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
            res.append(vecToTuple(x))
    return tuple(res)

def evaluate(operations, DEBUG=False):
    from subprocess import Popen, PIPE

    inp = send(operations)
    output, error = Popen(['vhe.exe'], stdin=PIPE, stdout=PIPE, stderr=PIPE, shell=True).communicate(inp)
    
    if error:
        from sys import stderr
        stderr.write(error + '\n')
        stderr.flush()

    return recv(output)





