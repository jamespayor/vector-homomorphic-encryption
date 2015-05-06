from flask import Flask, request
app = Flask(__name__)


from model import features, probabilities

@app.route('/')
def route_index():
    return '''
    	<html>
    		<body>
    			<h1> Homomorphic Email Classification Service! </h1>
    			<p> This server gives likelihoods that an email is spam or ham given an encrypted form of the original email,
    			without gaining access to any information about the message. </p>
    			<p> Get the features description from '/features' - each feature being a linear combination of word counts
    			in the document. </p>
    			<p> Submit requests by POST with the relevant transformation matrix (a tuple of tuples of integers) to '/transform'. </p>
    		</body>
    	</html>
    	'''

@app.route('/features', methods=['GET'])
def route_features():
    return repr(features)

@app.route('/transform', methods=['GET', 'POST'])
def route_search():
    print 'Getting key switch matrix...'
    keySwitchMatrix = tuple(y for y in (tuple(int(x.strip().strip('L')) for x in things if x.strip()) for things in (thing.replace('(','').split(',') for thing in request.form['keySwitch'].split(')'))) if y)
    print 'Done.  First entries of matrix:', keySwitchMatrix[0][:10]
    print 'Getting linear transformations...'
    results = get_linear_transformation(testX, keySwitchMatrix)
    print 'Done.  First results:', results[:10]
    return '\n'.join(map(repr, results))

def inf(x):
    while True:
        yield x

def flatten(x):
    for y in x:
        for z in y:
            yield z

def flatzip(*args):
    return list(flatten(zip(*args)))

def loadFeatures():
    import cPickle as pk
    return map(pk.load, (open('testX.pk'), open('testY.pk'), open('testFileNames.pk')))


def get_linear_transformation(features,keySwitchMatrix):
    from hevector import evaluate
    return evaluate([keySwitchMatrix] + flatzip(inf('duplicate-matrix'), features, inf('linear-transform')))[:-1]

testX, testY, testFileNames = loadFeatures()

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6856, debug=False)

